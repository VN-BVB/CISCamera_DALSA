#include "jointView.h"
#include "ui_jointView.h"
#include "src/ui/utils/imageWidget/interactiveScene.h"
#include "src/ui/utils/imageWidget/interactiveDisplayManager.h"

#include <QFileDialog>
#include <QDebug>
#include <QGraphicsPathItem>
#include <QPainterPath>

JointView::JointView(QWidget *parent)
    : QWidget(parent), 
    ui(new Ui::JointView),
    readWorker(new ImageReadWorker),
    processWorker(new ImageProcessWorker)
{
    ui->setupUi(this);
    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<std::shared_ptr<cv::Mat>>("std::shared_ptr<cv::Mat>");
    qRegisterMetaType<std::vector<cv::Point2f>>("std::vector<cv::Point2f>");
    qRegisterMetaType<std::vector<std::vector<cv::Point>>>("std::vector<std::vector<cv::Point>>");
    qRegisterMetaType<std::vector<std::vector<cv::Point2f>>>("std::vector<std::vector<cv::Point2f>>");
    qRegisterMetaType<std::vector<cv::Vec4f>>("std::vector<cv::Vec4f>");

    // 读取线程
    readWorker->moveToThread(&readThread);
    connect(&readThread, &QThread::finished, readWorker, &QObject::deleteLater);
    connect(this, &JointView::startImageRead, readWorker, &ImageReadWorker::readImage);
    connect(readWorker, &ImageReadWorker::imageRead, this, &JointView::handleImageRead);
    connect(readWorker, &ImageReadWorker::errorOccurred, this, &JointView::handleError);

    // 处理线程
    processWorker->moveToThread(&processThread);
    connect(&processThread, &QThread::finished, processWorker, &QObject::deleteLater);
    connect(this, &JointView::startImageProcess, processWorker, &ImageProcessWorker::processImage);
    connect(processWorker, &ImageProcessWorker::imageProcessed, this, &JointView::handleImageProcessed);
    connect(processWorker, &ImageProcessWorker::imageProcessedCannyDevenay, this, &JointView::handleImageProcessedCannyDevenay);
    connect(processWorker, &ImageProcessWorker::errorOccurred, this, &JointView::handleError);

    // 启动线程
    readThread.start();
    processThread.start();
}

JointView::~JointView()
{
    readThread.quit();
    readThread.wait();
    processThread.quit();
    processThread.wait();
    delete ui;
}

void JointView::on_pb_open_clicked()
{
    startTime = std::chrono::high_resolution_clock::now();

    // QString path = QFileDialog::getOpenFileName(this, "Select Image", "", "(*.png *.jpg *.bmp)");
    QString path = "E:/work/车门门环拼接/image/test/cropped_img_mirrored_stitched.bmp";
    if(path.isEmpty())
        return;

    emit startImageRead(path);
}

void JointView::handleImageRead(std::shared_ptr<cv::Mat> image)
{
    emit startImageProcess(image);
}

// Zernike矩对应槽函数
void JointView::handleImageProcessed(std::shared_ptr<cv::Mat> processedImage,
                                           std::vector<std::vector<cv::Point2f>> subpixelContours,
                                           std::vector<std::vector<cv::Point>> pixelContours,
                                           std::vector<cv::Vec4f> lines)
{
    // 在主线程中显示图像
    ui->gv_image->displayImage(*processedImage, true);
    InteractiveDisplayManager* displayMgr = ui->gv_image->getDisplayManager();
    if (displayMgr)
    {
        InteractiveScene* scene = displayMgr->displayScene();
        // scene->whenDrawSubpixelContours(subpixelContours);
        scene->whenDrawPixelContours(pixelContours);
        scene->whenDrawLines(lines);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    // 计算并输出时间差
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    qDebug() << "Total processing time: " << duration.count() << " ms";
}

// CannyDevenay算法对应槽函数
void JointView::handleImageProcessedCannyDevenay(std::shared_ptr<cv::Mat> processedImage, std::vector<Point2fCurve> edgeCurves)
{

}

void JointView::handleError(const QString &error)
{
    qDebug() << "错误:" << error;
}
