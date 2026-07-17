#include <QVBoxLayout>
#include <QHBoxLayout>

#include "test_multiRoi.h"
#include "ui_test_multiRoi.h"

test_multiRoi::test_multiRoi(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::test_multiRoi),
    m_frmDisplay(new FrmVisionDisplay(this))
{
    ui->setupUi(this);
    m_btn_display_multiRoi = new QPushButton("display multiRoi", this);

    m_btn_display_multiRoi->setFixedSize(200, 30);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_btn_display_multiRoi);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(m_frmDisplay);

    setLayout(mainLayout);
    setWindowTitle("视觉显示窗口");
    resize(1600, 1400);

    connect(m_btn_display_multiRoi, &QPushButton::clicked, this, &test_multiRoi::displayMultiRoi);
}

test_multiRoi::~test_multiRoi()
{
    delete ui;
}

void test_multiRoi::displayMultiRoi()
{
    DisplayManager* displayMgr = m_frmDisplay->getDisplayManager();
    if (!displayMgr) return;

    DisplayScene* scene = displayMgr->displayScene();
    if (!scene) return;

    QImage mainImage("E:/work/车门门环拼接/image/背面打光/9/1/1815_7147.bmp");
    QImage overlayImage1("E:/work/车门门环拼接/image/背面打光/9/1/4359_3917.bmp");
    QImage overlayImage2("E:/work/车门门环拼接/image/背面打光/9/1/4382_23770.bmp");
    QImage overlayImage3("E:/work/车门门环拼接/image/背面打光/9/1/6611_3858.bmp");

    DisplayImageItem* iamgeItem1 = scene->whenAddDisplayImage(overlayImage1, QPointF(500, 100));
    DisplayImageItem* iamgeItem2 = scene->whenAddDisplayImage(overlayImage2, QPointF(1000, 100));
    DisplayImageItem* iamgeItem3 = scene->whenAddDisplayImage(overlayImage3, QPointF(4000, 1000));
}

void test_multiRoi::onDisplayMultiRoiCliked()
{
    displayMultiRoi();
}
