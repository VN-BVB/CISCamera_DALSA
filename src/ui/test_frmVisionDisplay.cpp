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
    QImage qimg;
    if (image.type() == CV_8UC1)
    {
        qimg = QImage(image.data, image.cols, image.rows,
                      image.step, QImage::Format_Grayscale8);
    }
    else
    {
        cv::Mat img_rgb;
        cv::cvtColor(image, img_rgb, cv::COLOR_BGR2RGB);
        qimg = QImage(img_rgb.data, img_rgb.cols, img_rgb.rows, img_rgb.step, QImage::Format_RGB888);
    }
    if (m_frmDisplay)
    {
        m_frmDisplay->displayImage(qimg, true);
    }
}
