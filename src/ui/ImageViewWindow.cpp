#include "ImageViewWindow.h"
#include "ui_ImageViewWindow.h"
#include "src/ui/utils/imageWidget/interactiveScene.h"
#include "src/ui/utils/imageWidget/interactiveDisplayManager.h"

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
    qRegisterMetaType<std::shared_ptr<cv::Mat>>("std::shared_ptr<cv::Mat>");
    qRegisterMetaType<std::vector<cv::Point2f>>("std::vector<cv::Point2f>");
    qRegisterMetaType<std::vector<std::vector<cv::Point>>>("std::vector<std::vector<cv::Point>>");
    qRegisterMetaType<std::vector<std::vector<cv::Point>>>("std::vector<std::vector<cv::Point2f>>");

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

void ImageViewWindow::on_pb_open_clicked()
{
    startTime = std::chrono::high_resolution_clock::now();

    // QString path = QFileDialog::getOpenFileName(this, "Select Image", "", "(*.png *.jpg *.bmp)");
    QString path = "E:/work/车门门环拼接/image/test/cropped_img_mirrored_stitched.bmp";
    if(path.isEmpty())
        return;

    emit startImageRead(path);
}

void ImageViewWindow::handleImageRead(std::shared_ptr<cv::Mat> image)
{
    emit startImageProcess(image);
}

// Zernike矩对应槽函数
void ImageViewWindow::handleImageProcessed(std::shared_ptr<cv::Mat> processedImage, std::vector<std::vector<cv::Point2f>> subpixelContours,
                                           std::vector<std::vector<cv::Point>> pixelContours)
{
    // 在主线程中显示图像
    ui->gv_image->displayImage(*processedImage, true);
    InteractiveDisplayManager* displayMgr = ui->gv_image->getDisplayManager();
    if (displayMgr)
    {
        InteractiveScene* scene = displayMgr->displayScene();
        scene->whenDrawSubpixelContours(subpixelContours);
        scene->whenDrawPixelContours(pixelContours);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    // 计算并输出时间差
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    qDebug() << "Total processing time: " << duration.count() << " ms";
}

// CannyDevenay算法对应槽函数
void ImageViewWindow::handleImageProcessedCannyDevenay(std::shared_ptr<cv::Mat> processedImage, std::vector<Point2fCurve> edgeCurves)
{

}

void ImageViewWindow::handleError(const QString &error)
{
    qDebug() << "错误:" << error;
}
