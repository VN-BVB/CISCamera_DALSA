#include "joint_view.h"
#include "ui_joint_view.h"
#include "src/ui/utils/display/display_scene.h"
#include "src/ui/utils/display/display_manager.h"

#include <QFileDialog>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <plog/Log.h>

JointView::JointView(QWidget *parent)
    : QWidget(parent), 
    ui(new Ui::JointView),
    readWorker(new ImageReadWorker),
    processWorker(new ImageProcessWorker),
    m_showPixelContoursSquare(false)
{
    ui->setupUi(this);
    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<std::shared_ptr<cv::Mat>>("std::shared_ptr<cv::Mat>");
    qRegisterMetaType<std::vector<cv::Point2f>>("std::vector<cv::Point2f>");
    qRegisterMetaType<std::vector<std::vector<cv::Point>>>("std::vector<std::vector<cv::Point>>");
    qRegisterMetaType<std::vector<std::vector<cv::Point2f>>>("std::vector<std::vector<cv::Point2f>>");
    qRegisterMetaType<std::vector<cv::Vec4f>>("std::vector<cv::Vec4f>");
    qRegisterMetaType<std::vector<CurveSeg>>("std::vector<CurveSeg>");
    qRegisterMetaType<std::shared_ptr<JointSeam>>("std::shared_ptr<JointSeam>");

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
    // 连接第一个信号重载到第一个槽函数重载（5个参数版本）
    connect(processWorker,
            QOverload<std::shared_ptr<cv::Mat>,
                      std::vector<std::vector<cv::Point2f>>,
                      std::vector<std::vector<cv::Point>>,
                      std::vector<cv::Vec4f>,
                      std::vector<CurveSeg>>::of(&ImageProcessWorker::imageProcessed),
            this,
            QOverload<std::shared_ptr<cv::Mat>,
                      std::vector<std::vector<cv::Point2f>>,
                      std::vector<std::vector<cv::Point>>,
                      std::vector<cv::Vec4f>,
                      std::vector<CurveSeg>>::of(&JointView::handleImageProcessed));
    // 连接第二个信号重载到第二个槽函数重载（2个参数版本）
    connect(processWorker,
            QOverload<std::shared_ptr<cv::Mat>, std::shared_ptr<JointSeam>>::of(&ImageProcessWorker::imageProcessed),
            this,
            QOverload<std::shared_ptr<cv::Mat>, std::shared_ptr<JointSeam>>::of(&JointView::handleImageProcessed));
    connect(processWorker, &ImageProcessWorker::imageProcessedCannyDevenay, this, &JointView::handleImageProcessedCannyDevenay);
    connect(processWorker, &ImageProcessWorker::errorOccurred, this, &JointView::handleError);

    // 连接checkbox信号
    connect(ui->ckb_pixelContoursSquare, &QCheckBox::toggled, this, &JointView::on_ckb_pixelContoursSquare_toggled);
    connect(ui->ckb_pixelContoursLine, &QCheckBox::toggled, this, &JointView::on_ckb_pixelContoursLine_toggled);
    connect(ui->ckb_subpixelContours, &QCheckBox::toggled, this, &JointView::on_ckb_subpixelContours_toggled);
    connect(ui->ckb_fitlines, &QCheckBox::toggled, this, &JointView::on_ckb_fitlines_toggled);
    connect(ui->ckb_endPoints, &QCheckBox::toggled, this, &JointView::on_ckb_endPoints_toggled);
    // @TODO:整理这里的connect，在需要的地方才连接


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
    // QString path = "E:/work/车门门环拼接/image/背面打光/Splice_20251027_092312509.bmp";
    QString path = "E:/work/车门门环拼接/image/背面打光/9/1/6984_5772.bmp";
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
                                     std::vector<cv::Vec4f> lines,
                                     std::vector<CurveSeg> curves)
{
    // 保存当前数据
    m_currentImage = processedImage;
    m_subpixelContours = subpixelContours;
    m_pixelContours = pixelContours;
    m_fitTangentLines = lines;
    m_fitCurves = curves;
    // 计算角点（拼缝端点）
    m_endPointsByTangentLines.clear();
    if (lines.size() >= 2) {
        // 计算前两条直线的交点作为角点
        cv::Point2f corner = cv::Point2f(4, 5);
        if (corner.x >= 0 && corner.y >= 0) {
            m_endPointsByTangentLines.push_back(corner);
        }
    }

    // 更新显示
    updateDisplay();


    auto endTime = std::chrono::high_resolution_clock::now();
    // 计算并输出时间差
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    PLOG_INFO << "Total processing time: " << duration.count() << " ms";
}

void JointView::handleImageProcessed(std::shared_ptr<cv::Mat> processedImage,
                                     std::shared_ptr<JointSeam> jointSeam)
{
    m_currentImage = processedImage;

    // contour_processor的结果获取方式
    for (auto& cd : jointSeam->getContourDatas()) {
        m_subpixelContours.push_back(cd.getSortedContour());

        for (auto& [index, curveSeg] : cd.getCurveSegments())
            m_fitCurves.push_back(curveSeg);

        for (auto& line : cd.getTangentLines())
            m_fitTangentLines.push_back(line);

        for (auto& point : cd.getEndPoints())
            m_endPointsByTangentLines.push_back(point);

        for (auto& point : cd.getEndPoints())
            m_endPointsByFittedLines.push_back(point);
    }

    // 更新显示
    updateDisplay();

    auto endTime = std::chrono::high_resolution_clock::now();
    // 计算并输出时间差
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    PLOG_INFO << "Total processing time: " << duration.count() << " ms";
}

// CannyDevenay算法对应槽函数
void JointView::handleImageProcessedCannyDevenay(std::shared_ptr<cv::Mat> processedImage, std::vector<Point2fCurve> edgeCurves)
{

}

void JointView::handleError(const QString &error)
{
    PLOG_INFO << "错误:" << error;
}

// 更新显示函数
void JointView::updateDisplay() {
    if (!m_currentImage) return;

    // 在主线程中显示图像

    DisplayManager* displayMgr = ui->gv_image->getDisplayManager();
    if (!displayMgr) return;

    DisplayScene* scene = displayMgr->displayScene();
    if (!scene) return;
    ui->gv_image->displayImage(*m_currentImage, true);

    // 根据checkbox状态绘制不同的内容
    if (m_showPixelContoursSquare && !m_pixelContours.empty()) {
        scene->whenDrawPixelContours(m_pixelContours);
    }

    if (m_showPixelContoursLine && !m_pixelContours.empty()) {
        scene->whenDrawPixelContours(m_pixelContours);
    }

    if (m_showSubpixelContours && !m_subpixelContours.empty()) {
        scene->whenDrawSubpixelContours(m_subpixelContours);
    }

    if (m_showFitLines && !m_fitTangentLines.empty()) {
        scene->whenDrawLines(m_fitTangentLines, 100, Qt::blue);
    }

    if (m_showFitCurves && !m_fitCurves.empty()) {
        scene->whenDrawBSplineCurves(m_fitCurves);
    }

    if (m_showEndPoints && !m_endPointsByTangentLines.empty()) {
        scene->whenDrawPoints(m_endPointsByTangentLines, Qt::green);
    }

    // if (!m_fitLines.empty()) {
    //     scene->whenDrawLines(m_fitLines, 1000, Qt::yellow);
    // }

    // if (!m_endPointsByFittedLines.empty()) {
    //     scene->whenDrawPoints(m_endPointsByFittedLines, Qt::red);
    // }
    // @TODO:增加取消勾选时，删除相应轮廓的功能
}

// Checkbox槽函数实现
void JointView::on_ckb_pixelContoursSquare_toggled(bool checked) {
    m_showPixelContoursSquare = checked;
    updateDisplay();
}

void JointView::on_ckb_pixelContoursLine_toggled(bool checked) {
    m_showPixelContoursLine = checked;
    updateDisplay();
}

void JointView::on_ckb_subpixelContours_toggled(bool checked) {
    m_showSubpixelContours = checked;
    updateDisplay();
}

void JointView::on_ckb_fitlines_toggled(bool checked) {
    m_showFitLines = checked;
    updateDisplay();
}

void JointView::on_ckb_endPoints_toggled(bool checked) {
    m_showEndPoints = checked;
    updateDisplay();
}

void JointView::on_ckb_fitCurves_toggled(bool checked) {
    m_showFitCurves = checked;
    updateDisplay();
}
