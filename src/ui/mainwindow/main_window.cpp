#include "main_window.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVariant>
#include <QVBoxLayout>

#include "src/ui/measurementMonitor/measurement_monitor.h"
#include "src/ui/measurementMonitor/models/measurement_pipeline.h"
#include "src/ui/measurementMonitor/presenters/measurement_presenter.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_menuBarWidget(nullptr)
    , m_btnMeasurementMonitor(nullptr)
    , m_btnSystemCalibration(nullptr)
    , m_btnConfigManagement(nullptr)
    , m_stackedWidget(nullptr)
    , m_measurementMonitor(nullptr)
    , m_systemCalibrationPage(nullptr)
    , m_configManagementPage(nullptr)
{
    setupUi();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi() {
    setWindowTitle(QString::fromUtf8("CIS Camera Measurement System"));
    resize(1400, 900);

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setupMenuBar();
    mainLayout->addWidget(m_menuBarWidget);

    setupPages();
    mainLayout->addWidget(m_stackedWidget);

    setCentralWidget(centralWidget);
}

void MainWindow::setupMenuBar() {
    m_menuBarWidget = new QWidget(this);
    m_menuBarWidget->setFixedHeight(50);
    m_menuBarWidget->setObjectName("menuBarWidget");

    QHBoxLayout* menuLayout = new QHBoxLayout(m_menuBarWidget);
    menuLayout->setContentsMargins(20, 5, 20, 5);
    menuLayout->setSpacing(10);

    m_btnMeasurementMonitor = new QPushButton(QString::fromUtf8("测量监控"), this);
    m_btnSystemCalibration = new QPushButton(QString::fromUtf8("系统标定"), this);
    m_btnConfigManagement = new QPushButton(QString::fromUtf8("配置管理"), this);

    m_btnMeasurementMonitor->setCheckable(true);
    m_btnSystemCalibration->setCheckable(true);
    m_btnConfigManagement->setCheckable(true);

    m_btnMeasurementMonitor->setChecked(true);

    m_btnMeasurementMonitor->setObjectName("menuButton");
    m_btnSystemCalibration->setObjectName("menuButton");
    m_btnConfigManagement->setObjectName("menuButton");

    m_btnMeasurementMonitor->setProperty("pageIndex", QVariant(0));
    m_btnSystemCalibration->setProperty("pageIndex", QVariant(1));
    m_btnConfigManagement->setProperty("pageIndex", QVariant(2));

    menuLayout->addWidget(m_btnMeasurementMonitor);
    menuLayout->addWidget(m_btnSystemCalibration);
    menuLayout->addWidget(m_btnConfigManagement);
    menuLayout->addStretch();

    setupConnections();
}

void MainWindow::setupPages() {
    m_stackedWidget = new QStackedWidget(this);

    m_measurementMonitor = new MeasurementMonitor(this);
    m_stackedWidget->addWidget(m_measurementMonitor);

    // 组装测量监控的MVP
    m_pipeline  = std::make_unique<MeasurementPipeline>();
    m_presenter = std::make_unique<MeasurementPresenter>(m_measurementMonitor, m_pipeline.get(), this);

    m_systemCalibrationPage = new QWidget(this);
    QVBoxLayout* calibLayout = new QVBoxLayout(m_systemCalibrationPage);
    QLabel* calibLabel = new QLabel(QString::fromUtf8("系统标定页面 - 待实现"), m_systemCalibrationPage);
    calibLabel->setAlignment(Qt::AlignCenter);
    calibLabel->setStyleSheet("color: #808080; font-size: 24px;");
    calibLayout->addWidget(calibLabel);
    m_stackedWidget->addWidget(m_systemCalibrationPage);

    m_configManagementPage = new QWidget(this);
    QVBoxLayout* configLayout = new QVBoxLayout(m_configManagementPage);
    QLabel* configLabel = new QLabel(QString::fromUtf8("配置管理页面 - 待实现"), m_configManagementPage);
    configLabel->setAlignment(Qt::AlignCenter);
    configLabel->setStyleSheet("color: #808080; font-size: 24px;");
    configLayout->addWidget(configLabel);
    m_stackedWidget->addWidget(m_configManagementPage);

    m_stackedWidget->setCurrentIndex(0);
}

void MainWindow::setupConnections() {
    connect(m_btnMeasurementMonitor, &QPushButton::clicked, this, [this]() { onMenuButtonClicked(0); });
    connect(m_btnSystemCalibration, &QPushButton::clicked, this, [this]() { onMenuButtonClicked(1); });
    connect(m_btnConfigManagement, &QPushButton::clicked, this, [this]() { onMenuButtonClicked(2); });
}

void MainWindow::onMenuButtonClicked(int index) {
    m_btnMeasurementMonitor->setChecked(index == 0);
    m_btnSystemCalibration->setChecked(index == 1);
    m_btnConfigManagement->setChecked(index == 2);

    m_stackedWidget->setCurrentIndex(index);
}

