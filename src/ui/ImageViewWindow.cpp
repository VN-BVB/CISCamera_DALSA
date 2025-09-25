#include "ImageViewWindow.h"
#include "ui_ImageViewWindow.h"

#include <QFileDialog>
#include <QDebug>
#include <QGraphicsPathItem>
#include <QPainterPath>

ImageViewWindow::ImageViewWindow(QWidget *parent)
    : QWidget(parent), 
    ui(new Ui::ImageViewWindow),
    readWorker(new ImageReadWorker),
    processWorker(new ImageProcessWorker)
{
    ui->setupUi(this);
    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<std::vector<cv::Point2f>>("std::vector<cv::Point2f>");
    qRegisterMetaType<std::vector<std::vector<cv::Point>>>("std::vector<std::vector<cv::Point>>");

    // 读取线程
    readWorker->moveToThread(&readThread);
    connect(&readThread, &QThread::finished, readWorker, &QObject::deleteLater);
    connect(this, &ImageViewWindow::startImageRead, readWorker, &ImageReadWorker::readImage);
    connect(readWorker, &ImageReadWorker::imageRead, this, &ImageViewWindow::handleImageRead);
    connect(readWorker, &ImageReadWorker::errorOccurred, this, &ImageViewWindow::handleError);

    // 处理线程
    processWorker->moveToThread(&processThread);
    connect(&processThread, &QThread::finished, processWorker, &QObject::deleteLater);
    connect(this, &ImageViewWindow::startImageProcess, processWorker, &ImageProcessWorker::processImage);
    connect(processWorker, &ImageProcessWorker::imageProcessed, this, &ImageViewWindow::handleImageProcessed);
    connect(processWorker, &ImageProcessWorker::imageProcessedCannyDevenay, this, &ImageViewWindow::handleImageProcessedCannyDevenay);
    connect(processWorker, &ImageProcessWorker::errorOccurred, this, &ImageViewWindow::handleError);

    // 启动线程
    readThread.start();
    processThread.start();
}

ImageViewWindow::~ImageViewWindow()
{
    readThread.quit();
    readThread.wait();
    processThread.quit();
    processThread.wait();
    delete ui;
}

// 统一的轮廓绘制接口 - 亚像素
void ImageViewWindow::drawContour(QGraphicsScene *scene, const std::vector<cv::Point2f> &contour, bool isSubpixel) {
    if (contour.empty())
        return;

    QPainterPath path;
    path.moveTo(contour[0].x + 0.5, contour[0].y + 0.5);    // 坐标增加(0.5,0.5)，以像素块中心为像素整数坐标，而不是左上角

    for (size_t i = 1; i < contour.size(); ++i) {
        path.lineTo(contour[i].x + 0.5, contour[i].y + 0.5);
    }

    QGraphicsPathItem *pathItem = new QGraphicsPathItem(path);
    QPen pen(isSubpixel ? Qt::red : Qt::green); // 亚像素用红色，像素级用绿色
    pen.setWidthF(0.1);
    pen.setStyle(isSubpixel ? Qt::SolidLine : Qt::DashLine);
    pathItem->setPen(pen);
    pathItem->setZValue(10);

    scene->addItem(pathItem);
}

// 统一的轮廓绘制接口 - 像素级
void ImageViewWindow::drawContour(QGraphicsScene *scene, const std::vector<cv::Point> &contour, bool isSubpixel) {
    if (contour.empty())
        return;

    QPainterPath path;
    path.moveTo(contour[0].x + 0.5, contour[0].y + 0.5);    // 坐标增加(0.5,0.5)，以像素块中心为像素整数坐标，而不是左上角

    for (size_t i = 1; i < contour.size(); ++i) {
        path.lineTo(contour[i].x + 0.5, contour[i].y + 0.5);
    }

    QGraphicsPathItem *pathItem = new QGraphicsPathItem(path);
    QPen pen(isSubpixel ? Qt::red : Qt::green); // 亚像素用红色，像素级用绿色
    pen.setWidthF(0.1);
    pen.setStyle(isSubpixel ? Qt::SolidLine : Qt::DashLine);
    pathItem->setPen(pen);
    pathItem->setZValue(10);

    scene->addItem(pathItem);
}


// 绘制亚像素轮廓线
void ImageViewWindow::drawSubpixelContour(QGraphicsScene *scene, const std::vector<cv::Point2f> &subpixelContour) {
    drawContour(scene, subpixelContour, true);
}

// 像素级轮廓绘制方法
void ImageViewWindow::drawPixelContour(QGraphicsScene *scene, const std::vector<cv::Point> &pixelContour) {
    drawContour(scene, pixelContour, false);
}
void ImageViewWindow::on_pb_open_clicked()
{
    // QString path = QFileDialog::getOpenFileName(this, "Select Image", "", "(*.png *.jpg *.bmp)");
    QString path = "E:/work/车门门环焊接/背光20250529/背光20250529/822-1200-50us-2(背光).bmp";
    if(path.isEmpty())
        return;

    emit startImageRead(path);
}

void ImageViewWindow::handleImageRead(cv::Mat image)
{
    emit startImageProcess(image);
}

// Zernike矩对应槽函数
void ImageViewWindow::handleImageProcessed(cv::Mat processedImage, std::vector<cv::Point2f> subpixelContour,
                                           std::vector<std::vector<cv::Point>> pixelContour)
{
    // 在主线程中显示图像
    QImage qimg;
    if (processedImage.type() == CV_8UC1)
    {
        qimg = QImage(processedImage.data, processedImage.cols, processedImage.rows,
                      processedImage.step, QImage::Format_Grayscale8);
    }
    else
    {
        cv::Mat img_rgb;
        cv::cvtColor(processedImage, img_rgb, cv::COLOR_BGR2RGB);
        qimg = QImage(img_rgb.data, img_rgb.cols, img_rgb.rows, img_rgb.step, QImage::Format_RGB888);
    }

    QGraphicsScene *scene = new QGraphicsScene(this);
    // TODO:将绘制亚像素边缘的线的点的x，y都加0.5
    drawSubpixelContour(scene, subpixelContour);
    drawPixelContour(scene, pixelContour[0]);
    QPixmap pixmap = QPixmap::fromImage(qimg);
    scene->addPixmap(pixmap);
    ui->gv_image->setScene(scene);
    ui->gv_image->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

// CannyDevenay算法对应槽函数
void ImageViewWindow::handleImageProcessedCannyDevenay(cv::Mat processedImage, std::vector<Point2fCurve> edgeCurves)
{
    // 在主线程中显示图像
    QImage qimg;
    if (processedImage.type() == CV_8UC1)
    {
        qimg = QImage(processedImage.data, processedImage.cols, processedImage.rows,
                      processedImage.step, QImage::Format_Grayscale8);
    }
    else
    {
        cv::Mat img_rgb;
        cv::cvtColor(processedImage, img_rgb, cv::COLOR_BGR2RGB);
        qimg = QImage(img_rgb.data, img_rgb.cols, img_rgb.rows, img_rgb.step, QImage::Format_RGB888);
    }

    QGraphicsScene *scene = new QGraphicsScene(this);
    std::vector<cv::Point2f> subpixelContour = edgeCurves[0].points;
    drawSubpixelContour(scene, subpixelContour);
    // drawPixelContour(scene, pixelContour[0]);
    QPixmap pixmap = QPixmap::fromImage(qimg);
    scene->addPixmap(pixmap);
    ui->gv_image->setScene(scene);

    ui->gv_image->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void ImageViewWindow::handleError(const QString &error)
{
    qDebug() << "错误:" << error;
}



















