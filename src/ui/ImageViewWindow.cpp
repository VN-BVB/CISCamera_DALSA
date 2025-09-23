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

// 绘制亚像素轮廓线
void ImageViewWindow::drawSubpixelContour(QGraphicsScene *scene, const std::vector<cv::Point2f> &subpixelContour) {
    if (subpixelContour.empty())
        return;

    // 创建路径并移动到第一个点
    QPainterPath path;
    path.moveTo(subpixelContour[0].x, subpixelContour[0].y);

    for (size_t i = 1; i < subpixelContour.size(); ++i)
    {
        path.lineTo(subpixelContour[i].x, subpixelContour[i].y);
    }

    // 闭合路径（如果是闭合轮廓）
    if (subpixelContour.size() > 2)
    {
        path.lineTo(subpixelContour[0].x, subpixelContour[0].y);
    }

    // 创建路径项并设置样式
    QGraphicsPathItem *pathItem = new QGraphicsPathItem(path);
    QPen pen(Qt::red);
    pen.setWidth(0.5);
    pathItem->setPen(pen);
    pathItem->setZValue(10); // 确保在最上层显示

    scene->addItem(pathItem);
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

void ImageViewWindow::handleImageProcessed(cv::Mat processedImage, std::vector<cv::Point2f> subpixelContour)
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
    drawSubpixelContour(scene, subpixelContour);
    QPixmap pixmap = QPixmap::fromImage(qimg);
    scene->addPixmap(pixmap);
    ui->gv_image->setScene(scene);
    ui->gv_image->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void ImageViewWindow::handleError(const QString &error)
{
    qDebug() << "错误:" << error;
}



















