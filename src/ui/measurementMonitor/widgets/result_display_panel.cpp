#include "result_display_panel.h"

#include <QBoxLayout>
#include <QLabel>
#include <QScrollArea>

#include "measurement_data_card.h"

ResultDisplayPanel::ResultDisplayPanel(QWidget *parent)
    : QWidget(parent)
    , m_tabWidget(nullptr)
    , m_resultsTab(nullptr)
    , m_statusLabel(nullptr)
    , m_gapWidthCard(nullptr)
    , m_offsetCard(nullptr)
    , m_angleCard(nullptr)
    , m_settingsTab(nullptr)
    , m_chkRoiFrame(nullptr)
    , m_chkFittedCenterLine(nullptr)
    , m_chkMeasurementPoints(nullptr)
    , m_chkSubpixelContour(nullptr)
{
    setupUi();
}

void ResultDisplayPanel::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setDocumentMode(true);

    setupMeasurementResultsTab();
    setupDisplaySettingsTab();

    mainLayout->addWidget(m_tabWidget);
}

void ResultDisplayPanel::setupMeasurementResultsTab() {
    m_resultsTab = new QWidget(m_tabWidget);
    QVBoxLayout* layout = new QVBoxLayout(m_resultsTab);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(12);

    m_statusLabel = new QLabel(QString::fromUtf8("Status: IDLE"), m_resultsTab);
    m_statusLabel->setStyleSheet("font-size: 13px; color: #808080; padding: 5px;");
    layout->addWidget(m_statusLabel);

    m_gapWidthCard = new MeasurementDataCard(QString::fromUtf8("Gap Width"), QString::fromUtf8("mm"), m_resultsTab);
    layout->addWidget(m_gapWidthCard);

    m_offsetCard = new MeasurementDataCard(QString::fromUtf8("Offset"), QString::fromUtf8("mm"), m_resultsTab);
    layout->addWidget(m_offsetCard);

    m_angleCard = new MeasurementDataCard(QString::fromUtf8("Angle"), QString::fromUtf8("°"), m_resultsTab);
    layout->addWidget(m_angleCard);

    layout->addStretch();

    m_tabWidget->addTab(m_resultsTab, QString::fromUtf8("实时测量结果"));
}

void ResultDisplayPanel::setupDisplaySettingsTab() {
    m_settingsTab = new QWidget(m_tabWidget);
    QVBoxLayout* layout = new QVBoxLayout(m_settingsTab);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(8);

    QLabel* descLabel = new QLabel(QString::fromUtf8("勾选以在左侧图像上叠加显示辅助信息："), m_settingsTab);
    descLabel->setStyleSheet("font-size: 12px; color: #AAAAAA;");
    descLabel->setWordWrap(true);
    layout->addWidget(descLabel);

    layout->addSpacing(10);

    m_chkRoiFrame = new QCheckBox(QString::fromUtf8("ROI 区域框"), m_settingsTab);
    m_chkRoiFrame->setChecked(m_overlayFlags.showRoiFrame);
    layout->addWidget(m_chkRoiFrame);

    QLabel* roiDesc = new QLabel(QString::fromUtf8("蓝色虚线，标记分析区域"), m_settingsTab);
    roiDesc->setStyleSheet("font-size: 11px; color: #808080; margin-left: 24px;");
    layout->addWidget(roiDesc);

    layout->addSpacing(5);

    m_chkFittedCenterLine = new QCheckBox(QString::fromUtf8("拟合中心线"), m_settingsTab);
    m_chkFittedCenterLine->setChecked(m_overlayFlags.showFittedCenterLine);
    layout->addWidget(m_chkFittedCenterLine);

    QLabel* lineDesc = new QLabel(QString::fromUtf8("绿色实线，拼缝中心位置"), m_settingsTab);
    lineDesc->setStyleSheet("font-size: 11px; color: #808080; margin-left: 24px;");
    layout->addWidget(lineDesc);

    layout->addSpacing(5);

    m_chkMeasurementPoints = new QCheckBox(QString::fromUtf8("测量特征点"), m_settingsTab);
    m_chkMeasurementPoints->setChecked(m_overlayFlags.showMeasurementPoints);
    layout->addWidget(m_chkMeasurementPoints);

    QLabel* pointDesc = new QLabel(QString::fromUtf8("红色点，边缘关键计算点"), m_settingsTab);
    pointDesc->setStyleSheet("font-size: 11px; color: #808080; margin-left: 24px;");
    layout->addWidget(pointDesc);

    layout->addSpacing(5);

    m_chkSubpixelContour = new QCheckBox(QString::fromUtf8("亚像素轮廓"), m_settingsTab);
    m_chkSubpixelContour->setChecked(m_overlayFlags.showSubpixelContour);
    layout->addWidget(m_chkSubpixelContour);

    QLabel* contourDesc = new QLabel(QString::fromUtf8("亚像素精度的轮廓信息"), m_settingsTab);
    contourDesc->setStyleSheet("font-size: 11px; color: #808080; margin-left: 24px;");
    layout->addWidget(contourDesc);

    layout->addStretch();

    connect(m_chkRoiFrame, &QCheckBox::toggled, this, &ResultDisplayPanel::onCheckboxToggled);
    connect(m_chkFittedCenterLine, &QCheckBox::toggled, this, &ResultDisplayPanel::onCheckboxToggled);
    connect(m_chkMeasurementPoints, &QCheckBox::toggled, this, &ResultDisplayPanel::onCheckboxToggled);
    connect(m_chkSubpixelContour, &QCheckBox::toggled, this, &ResultDisplayPanel::onCheckboxToggled);

    m_tabWidget->addTab(m_settingsTab, QString::fromUtf8("图像显示设置"));
}

void ResultDisplayPanel::updateMeasurementData(const MeasurementData &data) {
    m_gapWidthCard->setValue(data.gapWidth, data.gapWidthValid);
    m_offsetCard->setValue(data.offset, data.offsetValid);
    m_angleCard->setValue(data.angle, data.angleValid);
}

void ResultDisplayPanel::setStatus(const QString &status) {
    QString displayStatus = QString::fromUtf8("Status: ") + status;
    m_statusLabel->setText(displayStatus);

    if (status == QString::fromUtf8("IDLE")) {
        m_statusLabel->setStyleSheet("font-size: 13px; color: #808080; padding: 5px;");
    } else if (status == QString::fromUtf8("MEASURING")) {
        m_statusLabel->setStyleSheet("font-size: 13px; color: #FFC107; padding: 5px;");
    } else if (status == QString::fromUtf8("ERROR")) {
        m_statusLabel->setStyleSheet("font-size: 13px; color: #F44336; padding: 5px;");
    } else {
        m_statusLabel->setStyleSheet("font-size: 13px; color: #4CAF50; padding: 5px;");
    }
}

DisplayOverlayFlags ResultDisplayPanel::getDisplayOverlayFlags() const {
    return m_overlayFlags;
}

void ResultDisplayPanel::onCheckboxToggled(bool checked) {
    Q_UNUSED(checked);

    m_overlayFlags.showRoiFrame = m_chkRoiFrame->isChecked();
    m_overlayFlags.showFittedCenterLine = m_chkFittedCenterLine->isChecked();
    m_overlayFlags.showMeasurementPoints = m_chkMeasurementPoints->isChecked();
    m_overlayFlags.showSubpixelContour = m_chkSubpixelContour->isChecked();

    emit displayOverlayFlagsChanged(m_overlayFlags);
}
