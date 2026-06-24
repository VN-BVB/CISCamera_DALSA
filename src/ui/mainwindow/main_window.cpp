#include "main_window.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVariant>
#include <QVBoxLayout>

#include "src/ui/measurementMonitor/measurement_monitor.h"

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
    applyDarkStyle();
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

void MainWindow::applyDarkStyle() {
    QString style = R"(
        QMainWindow, QWidget {
            background-color: #1E1E1E;
            color: #FFFFFF;
        }
        #menuBarWidget {
            background-color: #2D2D2D;
            border-bottom: 1px solid #3D3D3D;
        }
        #menuButton {
            background-color: transparent;
            color: #AAAAAA;
            border: none;
            padding: 10px 24px;
            border-radius: 4px;
            font-size: 14px;
            font-weight: bold;
        }
        #menuButton:hover {
            background-color: #3D3D3D;
            color: #FFFFFF;
        }
        #menuButton:checked {
            background-color: #0078D4;
            color: #FFFFFF;
        }
        QPushButton {
            background-color: #0078D4;
            color: #FFFFFF;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: #1084D8;
        }
        QPushButton:pressed {
            background-color: #006CBD;
        }
        QPushButton:disabled {
            background-color: #3D3D3D;
            color: #808080;
        }
        QTabWidget::pane {
            border: 1px solid #3D3D3D;
            background-color: #2D2D2D;
            border-radius: 4px;
        }
        QTabBar::tab {
            background-color: #2D2D2D;
            color: #AAAAAA;
            padding: 8px 16px;
            border: none;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        QTabBar::tab:selected {
            background-color: #0078D4;
            color: #FFFFFF;
        }
        QTabBar::tab:hover:!selected {
            background-color: #3D3D3D;
        }
        QCheckBox {
            color: #FFFFFF;
            spacing: 8px;
            font-size: 13px;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid #0078D4;
            border-radius: 3px;
        }
        QCheckBox::indicator:checked {
            background-color: #0078D4;
            border-color: #0078D4;
        }
        QCheckBox::indicator:hover {
            border-color: #1084D8;
        }
        QSplitter::handle {
            background-color: #3D3D3D;
        }
        QTextEdit {
            background-color: #252526;
            color: #D4D4D4;
            border: 1px solid #3D3D3D;
            border-radius: 4px;
            font-family: Consolas, 'Courier New', monospace;
            font-size: 12px;
        }
        QScrollArea {
            border: none;
            background-color: transparent;
        }
        QScrollBar:vertical {
            background-color: #2D2D2D;
            width: 12px;
            border-radius: 6px;
        }
        QScrollBar::handle:vertical {
            background-color: #555555;
            border-radius: 6px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: #666666;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QScrollBar:horizontal {
            background-color: #2D2D2D;
            height: 12px;
            border-radius: 6px;
        }
        QScrollBar::handle:horizontal {
            background-color: #555555;
            border-radius: 6px;
            min-width: 30px;
        }
        QScrollBar::handle:horizontal:hover {
            background-color: #666666;
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
        }
        QLabel {
            color: #FFFFFF;
            background-color: transparent;
        }
        QFrame {
            background-color: #2D2D2D;
            border: 1px solid #3D3D3D;
            border-radius: 4px;
        }
        QStackedWidget {
            border: none;
        }
    )";
    this->setStyleSheet(style);
}
