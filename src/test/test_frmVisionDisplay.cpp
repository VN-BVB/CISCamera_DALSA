#include "test_frmVisionDisplay.h"
#include "ui_test_frmVisionDisplay.h"
#include <QVBoxLayout>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <plog/Log.h>

test_FrmVisionDisplay::test_FrmVisionDisplay(QWidget* parent)
    : QWidget(parent),
    ui(new Ui::test_FrmVisionDisplay),
    m_frmDisplay(new FrmVisionDisplay(this))
{
    ui->setupUi(this);
    m_btn_begin = new QPushButton("start", this);
    m_btn_draw_lines = new QPushButton("draw lines",this);
    m_btn_draw_contours = new QPushButton("draw contours",this);

    // 设置按钮属性
    m_btn_begin->setFixedSize(80, 30);
    m_btn_draw_lines->setFixedSize(200, 30);
    m_btn_draw_contours->setFixedSize(200, 30);

    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // 创建按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_btn_begin);
    buttonLayout->addWidget(m_btn_draw_lines);
    buttonLayout->addWidget(m_btn_draw_contours);
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
    scene->whenDrawContours(contours);
}

void test_FrmVisionDisplay::displayRotateRects(std::vector<cv::RotatedRect>& RotatedRects) {
    DisplayManager* displayMgr = m_frmDisplay->getDisplayManager();
    if (!displayMgr) return;

    DisplayScene* scene = displayMgr->displayScene();
    if (!scene) return;
    scene->whenDisplayRotateRects(RotatedRects);
}

void test_FrmVisionDisplay::displayLines(std::vector<cv::Vec4f> lines) {
    DisplayManager* displayMgr = m_frmDisplay->getDisplayManager();
    if (!displayMgr) return;

    DisplayScene* scene = displayMgr->displayScene();
    if (!scene) return;
    scene->whenDrawLines(lines, 100);
    scene->whenDrawLines(lines, 10, Qt::yellow);
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













