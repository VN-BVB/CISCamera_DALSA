#include "test_frmVisionDisplay.h"
#include "ui_test_frmVisionDisplay.h"
#include <QVBoxLayout>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

test_FrmVisionDisplay::test_FrmVisionDisplay(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::test_FrmVisionDisplay),
    m_frmDisplay(new FrmVisionDisplay(this))
{
    ui->setupUi(this);

    // 设置布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_frmDisplay);

    setLayout(mainLayout);
    setWindowTitle("视觉显示窗口");
    resize(800, 600);
}

test_FrmVisionDisplay::~test_FrmVisionDisplay()
{
    delete ui;
}

void test_FrmVisionDisplay::displayImage(const QString &imagePath)
{
    cv::Mat image = cv::imread(imagePath.toStdString(), cv::IMREAD_GRAYSCALE);
    if (m_frmDisplay)
    {
        m_frmDisplay->displayImage(image, true);
    }
}

