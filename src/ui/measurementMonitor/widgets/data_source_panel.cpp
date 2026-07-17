#include "data_source_panel.h"

#include <QBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QVariant>

#include "../qss_loader.h"

DataSourcePanel::DataSourcePanel(QWidget *parent)
    : QWidget(parent)
    , m_currentMode(Mode::SharedMemory)
    , m_isConnected(false)
    , m_btnSharedMemory(nullptr)
    , m_btnLocalFile(nullptr)
    , m_modeStack(nullptr)
    , m_sharedMemoryPage(nullptr)
    , m_lblConnectionStatus(nullptr)
    , m_btnConnect(nullptr)
    , m_localFilePage(nullptr)
    , m_lblFilePath(nullptr)
    , m_btnOpenFile(nullptr)
    , m_btnExecute(nullptr)
{
    setupUi();
}

void DataSourcePanel::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(12);

    QLabel* titleLabel = new QLabel(QString::fromUtf8("数据源与操作"), this);
    titleLabel->setObjectName("titleLabel");
    mainLayout->addWidget(titleLabel);

    QWidget* modeSwitchWidget = new QWidget(this);
    QHBoxLayout* modeLayout = new QHBoxLayout(modeSwitchWidget);
    modeLayout->setContentsMargins(0, 0, 0, 0);
    modeLayout->setSpacing(5);

    m_btnSharedMemory = new QPushButton(QString::fromUtf8("共享内存"), this);
    m_btnLocalFile = new QPushButton(QString::fromUtf8("本地文件"), this);

    m_btnSharedMemory->setCheckable(true);
    m_btnLocalFile->setCheckable(true);
    m_btnSharedMemory->setChecked(true);

    m_btnSharedMemory->setProperty("mode", static_cast<int>(Mode::SharedMemory));
    m_btnLocalFile->setProperty("mode", static_cast<int>(Mode::LocalFile));

    modeLayout->addWidget(m_btnSharedMemory);
    modeLayout->addWidget(m_btnLocalFile);
    modeLayout->addStretch();

    mainLayout->addWidget(modeSwitchWidget);

    m_modeStack = new QStackedWidget(this);

    // ======= sharedMemoryPage =======
    m_sharedMemoryPage = new QWidget(m_modeStack);
    QVBoxLayout* sharedMemLayout = new QVBoxLayout(m_sharedMemoryPage);
    sharedMemLayout->setContentsMargins(10, 10, 10, 10);
    sharedMemLayout->setSpacing(10);

    QLabel* sharedMemTitle = new QLabel(QString::fromUtf8("共享内存流"), m_sharedMemoryPage);
    sharedMemTitle->setObjectName("sectionTitle");
    sharedMemLayout->addWidget(sharedMemTitle);

    m_lblConnectionStatus = new QLabel(QString::fromUtf8("● 未连接 (Offline)"), m_sharedMemoryPage);
    m_lblConnectionStatus->setObjectName("connectionStatus");
    m_lblConnectionStatus->setProperty("state", "offline");
    sharedMemLayout->addWidget(m_lblConnectionStatus);

    m_btnConnect = new QPushButton(QString::fromUtf8("连接"), m_sharedMemoryPage);
    m_btnConnect->setFixedWidth(80);
    sharedMemLayout->addWidget(m_btnConnect, 0, Qt::AlignRight);

    sharedMemLayout->addStretch();
    m_modeStack->addWidget(m_sharedMemoryPage);

    // ======= localFilePage =======
    m_localFilePage = new QWidget(m_modeStack);
    QVBoxLayout* localFileLayout = new QVBoxLayout(m_localFilePage);
    localFileLayout->setContentsMargins(10, 10, 10, 10);
    localFileLayout->setSpacing(10);

    QLabel* localFileTitle = new QLabel(QString::fromUtf8("本地图像"), m_localFilePage);
    localFileTitle->setObjectName("sectionTitle");
    localFileLayout->addWidget(localFileTitle);

    m_lblFilePath = new QLabel(QString::fromUtf8("未选择文件"), m_localFilePage);
    m_lblFilePath->setObjectName("filePath");
    m_lblFilePath->setProperty("state", "empty");
    m_lblFilePath->setWordWrap(true);
    localFileLayout->addWidget(m_lblFilePath);

    m_btnOpenFile = new QPushButton(QString::fromUtf8("打开..."), m_localFilePage);
    m_btnOpenFile->setFixedWidth(80);
    localFileLayout->addWidget(m_btnOpenFile, 0, Qt::AlignRight);

    localFileLayout->addStretch();
    m_modeStack->addWidget(m_localFilePage);

    mainLayout->addWidget(m_modeStack);

    m_btnExecute = new QPushButton(QString::fromUtf8("▶ 开始自动测量"), this);
    m_btnExecute->setFixedHeight(36);
    mainLayout->addWidget(m_btnExecute);

    connect(m_btnSharedMemory, &QPushButton::clicked, this, &DataSourcePanel::onModeButtonClicked);
    connect(m_btnLocalFile, &QPushButton::clicked, this, &DataSourcePanel::onModeButtonClicked);
    connect(m_btnConnect, &QPushButton::clicked, this, &DataSourcePanel::onConnectClicked);
    connect(m_btnOpenFile, &QPushButton::clicked, this, &DataSourcePanel::onOpenFileClicked);
    connect(m_btnExecute, &QPushButton::clicked, this, &DataSourcePanel::onExecuteClicked);

    updateModeUI();
}

void DataSourcePanel::onModeButtonClicked() {
    QPushButton* btn = qobject_cast<QPushButton*>(sender());    // 获取当前触发槽函数的对象
    if (!btn) return;

    int mode = btn->property("mode").toInt();
    m_currentMode = static_cast<Mode>(mode);

    m_btnSharedMemory->setChecked(m_currentMode == Mode::SharedMemory);
    m_btnLocalFile->setChecked(m_currentMode == Mode::LocalFile);

    updateModeUI();
    emit modeChanged(m_currentMode);
}

void DataSourcePanel::updateModeUI() {
    if (m_currentMode == Mode::SharedMemory) {
        m_modeStack->setCurrentWidget(m_sharedMemoryPage);
        m_btnExecute->setText(QString::fromUtf8("开始自动测量"));
    } else {
        m_modeStack->setCurrentWidget(m_localFilePage);
        m_btnExecute->setText(QString::fromUtf8("执行单次测量"));
    }
}

void DataSourcePanel::onConnectClicked() {
    if (m_isConnected) {
        m_isConnected = false;
        emit disconnectSharedMemoryRequested();
    } else {
        emit connectSharedMemoryRequested(0);
    }
    updateConnectionUI();
}

void DataSourcePanel::updateConnectionUI() {
    if (m_isConnected) {
        m_lblConnectionStatus->setText(QString::fromUtf8("● 已连接 (Online)"));
        qss_loader::setState(m_lblConnectionStatus, "state", "online");
        m_btnConnect->setText(QString::fromUtf8("断开"));
    } else {
        m_lblConnectionStatus->setText(QString::fromUtf8("● 未连接 (Offline)"));
        qss_loader::setState(m_lblConnectionStatus, "state", "offline");
        m_btnConnect->setText(QString::fromUtf8("连接"));
    }
}

void DataSourcePanel::onOpenFileClicked() {
    QString path = QFileDialog::getOpenFileName(this,
                                                 QString::fromUtf8("选择图像文件"),
                                                 QString(),
                                                 "Images (*.png *.jpg *.bmp *.tif)");
    if (!path.isEmpty()) {
        setFilePath(path);
        emit startImageRead(path);
    }
}

void DataSourcePanel::onExecuteClicked() {
    if (m_currentMode == Mode::SharedMemory) {
        if (m_isConnected) {
            emit stopAutoMeasurementRequested();
        } else {
            emit startAutoMeasurementRequested();
        }
    } else {
        emit executeSingleMeasurementRequested();
    }
}

void DataSourcePanel::setConnectionStatus(bool connected) {
    m_isConnected = connected;
    updateConnectionUI();
}

void DataSourcePanel::setFilePath(const QString &path) {
    m_currentFilePath = path;
    QFileInfo fileInfo(path);
    m_lblFilePath->setText(fileInfo.fileName());
    qss_loader::setState(m_lblFilePath, "state", path.isEmpty() ? "empty" : "set");
}

void DataSourcePanel::setMeasurementEnabled(bool enabled) {
    m_btnExecute->setEnabled(enabled);
}
