
#include <QVBoxLayout>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <plog/Log.h>

#include "test_frmVisionDisplay.h"
#include "ui_test_frmVisionDisplay.h"
#include "src/ui/utils/display/graphicItems/graphic_item_component.h"
#include "src/ui/utils/display/graphicItems/graphic_item_composite.h"
#include "src/ui/utils/display/graphicItems/line_item.h"
#include "src/ui/utils/display/graphicItems/point_item.h"
#include "src/ui/utils/display/graphicItems/bspline_item.h"
#include "src/ui/utils/display/graphicItems/rotated_rect_item.h"

test_FrmVisionDisplay::test_FrmVisionDisplay(QWidget* parent)
    : QWidget(parent),
    ui(new Ui::test_FrmVisionDisplay),
    m_frmDisplay(new FrmVisionDisplay(this))
{
    ui->setupUi(this);
    m_btn_begin = new QPushButton("start", this);
    m_btn_draw_lines = new QPushButton("draw lines",this);
    m_btn_draw_contours = new QPushButton("draw contours",this);
    m_btn_draw_points = new QPushButton("draw points",this);
    m_btn_draw_bspline = new QPushButton("draw bspline",this);
    m_btn_draw_rotated_rect = new QPushButton("draw rotated rect",this);

    // 设置按钮属性
    m_btn_begin->setFixedSize(80, 30);
    m_btn_draw_lines->setFixedSize(200, 30);
    m_btn_draw_contours->setFixedSize(200, 30);
    m_btn_draw_points->setFixedSize(200, 30);
    m_btn_draw_bspline->setFixedSize(200, 30);
    m_btn_draw_rotated_rect->setFixedSize(200, 30);

    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // 创建按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_btn_begin);
    buttonLayout->addWidget(m_btn_draw_lines);
    buttonLayout->addWidget(m_btn_draw_contours);
    buttonLayout->addWidget(m_btn_draw_points);
    buttonLayout->addWidget(m_btn_draw_bspline);
    buttonLayout->addWidget(m_btn_draw_rotated_rect);
    buttonLayout->addStretch();  // 将按钮推到左侧

    // 添加按钮布局和显示控件到主布局
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(m_frmDisplay);

    setLayout(mainLayout);
    setWindowTitle("视觉显示窗口");
    resize(1600, 1400);

    // 连接按钮点击信号到槽函数
    connect(m_btn_begin, &QPushButton::clicked, this, &test_FrmVisionDisplay::onBeginButtonClicked);
    connect(m_btn_draw_lines, &QPushButton::clicked, this, &test_FrmVisionDisplay::onDrawLinesBtnClicked);
    connect(m_btn_draw_contours, &QPushButton::clicked, this, &test_FrmVisionDisplay::onDrawContoursBtnClicked);
    connect(m_btn_draw_points, &QPushButton::clicked, this, &test_FrmVisionDisplay::onDrawPointsBtnClicked);
    connect(m_btn_draw_bspline, &QPushButton::clicked, this, &test_FrmVisionDisplay::onDrawBSplineBtnClicked);
    connect(m_btn_draw_rotated_rect, &QPushButton::clicked, this, &test_FrmVisionDisplay::onDrawRotatedRectBtnClicked);
}

test_FrmVisionDisplay::~test_FrmVisionDisplay() { delete ui; }

void test_FrmVisionDisplay::displayImage(const QString& imagePath) {
    cv::Mat image = cv::imread(imagePath.toStdString(), cv::IMREAD_GRAYSCALE);
    // cv::Mat croppedImg = image(cv::Rect(4200, 4200, 1800, 200));
    // cv::imwrite("E:/work/车门门环拼接/image/test/frontLight/piececropped_img.bmp", image);
    auto smartPtrImage = std::make_shared<cv::Mat>(image);
    if (m_frmDisplay) {
        m_frmDisplay->displayImage(smartPtrImage, true);
    }
}

void test_FrmVisionDisplay::displayContours(std::vector<std::vector<cv::Point2f>> contours) {
    DisplayManager* displayMgr = m_frmDisplay->getDisplayManager();
    if (!displayMgr) return;

    DisplayScene* scene = displayMgr->displayScene();
    if (!scene) return;
    for (const auto& contour : contours) {
        if (!contour.empty()) {
            auto contourComponent = std::make_shared<ContourItem> (contour, ContourItem::subpixelContour);
            scene->whenAddGraphicComponent(contourComponent);
        }
    }
}

void test_FrmVisionDisplay::displayPoints(std::vector<cv::Point2f> points)
{
    DisplayManager* displayMgr = m_frmDisplay->getDisplayManager();
    if (!displayMgr) return;

    DisplayScene* scene = displayMgr->displayScene();
    if (!scene) return;
    if (!points.empty()) {
        auto pointComponent = std::make_shared<PointItem>(points);
        scene->whenAddGraphicComponent(pointComponent);
    }
}

void test_FrmVisionDisplay::displayBSpline(std::vector<cv::Point2f> controlPoints)
{
    DisplayManager* displayMgr = m_frmDisplay->getDisplayManager();
    if (!displayMgr) return;

    DisplayScene* scene = displayMgr->displayScene();
    if (!scene) return;
    if (controlPoints.size() >= 2) {
        auto splineComponent = std::make_shared<BSplineItem>(controlPoints);
        scene->whenAddGraphicComponent(splineComponent);
    }
}

void test_FrmVisionDisplay::displayRotateRects(std::vector<cv::RotatedRect>& rotatedRects) {
    DisplayManager* displayMgr = m_frmDisplay->getDisplayManager();
    if (!displayMgr) return;

    DisplayScene* scene = displayMgr->displayScene();
    if (!scene) return;
    if (!rotatedRects.empty()) {
        auto rotatedRectComponent = std::make_shared<RotatedRectItem>(rotatedRects);
        scene->whenAddGraphicComponent(rotatedRectComponent);
    }
}

void test_FrmVisionDisplay::displayLines(std::vector<cv::Vec4f> lines) {
    DisplayManager* displayMgr = m_frmDisplay->getDisplayManager();
    if (!displayMgr) return;

    DisplayScene* scene = displayMgr->displayScene();
    if (!scene) return;
    for (const auto& line : lines) {
        auto lineComponent = std::make_shared<LineItem> (line,100,Qt::blue);
        scene->whenAddGraphicComponent(lineComponent);
    }
}

// 开始按钮点击槽函数实现
void test_FrmVisionDisplay::onBeginButtonClicked() {
    // 这里实现按钮点击后的功能
    // 创建计时器并开始计时
    QElapsedTimer timer;
    timer.start();
    PLOG_INFO << "开始按钮被点击";
    TestEdgeAssembly tea;
    tea.run();
    // 计算并输出耗时
    qint64 elapsed = timer.elapsed();
    PLOG_INFO << "函数执行完成，耗时:" << elapsed << "毫秒";
    std::vector<std::vector<cv::Point2f>> contours;
    for (auto& cdata : tea.m_cDatas) {
        std::vector<cv::Point2f> contour = cdata.getSubpixelContour();
        contours.push_back(contour);
    }
    displayRotateRects(tea.m_workpieceRotateRect);
    displayContours(contours);

}

void test_FrmVisionDisplay::onDrawLinesBtnClicked()
{
    // 构造几条经过原点的直线
    std::vector<cv::Vec4f> lines;

    // 直线1: 水平线 (y=0)
    lines.push_back(cv::Vec4f(1.0f, 0.0f, 0.0f, 0.0f));

    // 直线2: 垂直线 (x=0)
    lines.push_back(cv::Vec4f(0.0f, 1.0f, 0.0f, 0.0f));

    // 直线3: 45度斜线 (y=x)
    lines.push_back(cv::Vec4f(0.7071f, 0.7071f, 0.0f, 0.0f));

    // 直线4: 135度斜线 (y=-x)
    lines.push_back(cv::Vec4f(-0.7071f, 0.7071f, 0.0f, 0.0f));

    // 直线5: 30度斜线
    lines.push_back(cv::Vec4f(0.8660f, 0.5f, 0.0f, 0.0f));

    // 直线6: 60度斜线
    lines.push_back(cv::Vec4f(0.5f, 0.8660f, 0.0f, 0.0f));

    displayLines(lines);

    PLOG_INFO << "绘制了" << lines.size() << "条经过原点的直线";
}

void test_FrmVisionDisplay::onDrawContoursBtnClicked()
{
    std::vector<std::vector<cv::Point2f>> contours;
    std::vector<cv::Point2f> contourPoints1 = {{0,0}, {100,0}, {100,100}, {0,100}};
    std::vector<cv::Point2f> contourPoints2 = {{500,0}, {500,0}, {100,100}, {0,500}};
    contours.push_back(contourPoints1);
    contours.push_back(contourPoints2);
    displayContours(contours);
    PLOG_INFO << "绘制了" << contours.size() << "条轮廓";
}

void test_FrmVisionDisplay::onDrawPointsBtnClicked()
{
    std::vector<cv::Point2f> points1 = {{0,0}, {100,0}, {100,100}, {0,100}};
    displayPoints(points1);
    LOG_INFO << "绘制了" << points1.size() << "个点";
}

void test_FrmVisionDisplay::onDrawBSplineBtnClicked()
{
    // 构造B样条曲线的控制点
    std::vector<cv::Point2f> controlPoints;

    // 添加一些控制点，形成一个简单的曲线
    controlPoints.push_back(cv::Point2f(0, 0));
    controlPoints.push_back(cv::Point2f(100, 50));
    controlPoints.push_back(cv::Point2f(200, -50));
    controlPoints.push_back(cv::Point2f(300, 100));
    controlPoints.push_back(cv::Point2f(400, 0));

    displayBSpline(controlPoints);

    // 同时绘制控制点，方便查看
    displayPoints(controlPoints);

    PLOG_INFO << "绘制了一条B样条曲线，控制点数量：" << controlPoints.size();
}

// 添加旋转矩形按钮点击槽函数实现
void test_FrmVisionDisplay::onDrawRotatedRectBtnClicked()
{
    // 构造几个旋转矩形用于测试
    std::vector<cv::RotatedRect> rotatedRects;

    // 创建第一个旋转矩形：中心在(200, 200)，宽度100，高度50，角度0度（不旋转）
    cv::Point2f center1(200, 200);
    cv::Size2f size1(100, 50);
    float angle1 = 0.0f;
    rotatedRects.push_back(cv::RotatedRect(center1, size1, angle1));

    // 创建第二个旋转矩形：中心在(400, 200)，宽度100，高度50，角度45度
    cv::Point2f center2(400, 200);
    cv::Size2f size2(100, 50);
    float angle2 = 45.0f;
    rotatedRects.push_back(cv::RotatedRect(center2, size2, angle2));

    // 创建第三个旋转矩形：中心在(200, 400)，宽度150，高度80，角度30度
    cv::Point2f center3(200, 400);
    cv::Size2f size3(150, 80);
    float angle3 = 30.0f;
    rotatedRects.push_back(cv::RotatedRect(center3, size3, angle3));

    // 创建第四个旋转矩形：中心在(400, 400)，宽度80，高度120，角度-30度
    cv::Point2f center4(400, 400);
    cv::Size2f size4(80, 120);
    float angle4 = -30.0f;
    rotatedRects.push_back(cv::RotatedRect(center4, size4, angle4));

    // 显示旋转矩形
    displayRotateRects(rotatedRects);

    // 同时显示矩形中心点，方便查看
    std::vector<cv::Point2f> centers;
    centers.push_back(center1);
    centers.push_back(center2);
    centers.push_back(center3);
    centers.push_back(center4);
    displayPoints(centers);

    PLOG_INFO << "绘制了" << rotatedRects.size() << "个旋转矩形";
}







