#include "ImageViewWindow.h"
#include "ui_ImageViewWindow.h"

#include <QFileDialog>
#include <QDebug>

ImageViewWindow::ImageViewWindow(QWidget *parent)
    : QWidget(parent), 
    ui(new Ui::ImageViewWindow),
    readWorker(new ImageReadWorker),
    processWorker(new ImageProcessWorker)
{
    ui->setupUi(this);
    qRegisterMetaType<cv::Mat>("cv::Mat");

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

void ImageViewWindow::on_pb_open_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Select Image", "", "(*.png *.jpg *.bmp)");
    if(path.isEmpty())
        return;

    emit startImageRead(path);
}

void ImageViewWindow::handleImageRead(cv::Mat image)
{
    emit startImageProcess(image);
}

void ImageViewWindow::handleImageProcessed(cv::Mat processedImage)
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
    QPixmap pixmap = QPixmap::fromImage(qimg);
    scene->addPixmap(pixmap);
    ui->gv_image->setScene(scene);
    ui->gv_image->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void ImageViewWindow::handleError(const QString &error)
{
    qDebug() << "错误:" << error;
}



















