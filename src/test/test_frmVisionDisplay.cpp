#include "test_frmVisionDisplay.h"
#include "ui_test_frmVisionDisplay.h"
#include <QVBoxLayout>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <QDebug>

test_FrmVisionDisplay::test_FrmVisionDisplay(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::test_FrmVisionDisplay),
    m_frmDisplay(new FrmVisionDisplay(this)),
    m_btn_begin(new QPushButton("开始", this))
{
    ui->setupUi(this);

    // 设置按钮属性
    m_btn_begin->setFixedSize(80, 30);

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // 创建按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_btn_begin);
    buttonLayout->addStretch();  // 将按钮推到左侧

    // 添加按钮布局和显示控件到主布局
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(m_frmDisplay);

    setLayout(mainLayout);
    setWindowTitle("视觉显示窗口");
    resize(1600, 1400);

    // 连接按钮点击信号到槽函数
    connect(m_btn_begin, &QPushButton::clicked, this, &test_FrmVisionDisplay::onBeginButtonClicked);
}

test_FrmVisionDisplay::~test_FrmVisionDisplay()
{
    delete ui;
}

void test_FrmVisionDisplay::displayImage(const QString &imagePath)
{
    cv::Mat image = cv::imread(imagePath.toStdString(), cv::IMREAD_GRAYSCALE);
    // cv::Mat croppedImg = image(cv::Rect(4200, 4200, 1800, 200));
    // cv::imwrite("E:/work/车门门环拼接/image/test/frontLight/piececropped_img.bmp", image);
    auto smartPtrImage = std::make_shared<cv::Mat>(image);
    if (m_frmDisplay)
    {
        m_frmDisplay->displayImage(smartPtrImage, true);
    }
}

void test_FrmVisionDisplay::displayContours(std::vector<std::vector<cv::Point2f>> contours)
{
    DisplayManager* displayMgr = m_frmDisplay->getDisplayManager();
    if (!displayMgr) return;

    DisplayScene* scene = displayMgr->displayScene();
    if (!scene) return;
    scene->whenDrawSubpixelContours(contours);
}

// 添加按钮点击槽函数实现
void test_FrmVisionDisplay::onBeginButtonClicked()
{
    // 这里实现按钮点击后的功能
    qDebug() << "开始按钮被点击";
    TestEdgeAssembly tea;
    tea.run();
    std::vector<std::vector<cv::Point2f>> contours;
    for (auto& cdata : tea.m_cDatas)
    {
        std::vector<cv::Point2f> contour = cdata.getSubpixelContour();
        contours.push_back(contour);
    }
    displayContours(contours);
}






















