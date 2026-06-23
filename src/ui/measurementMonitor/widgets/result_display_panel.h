#ifndef RESULT_DISPLAY_PANEL_H
#define RESULT_DISPLAY_PANEL_H

#include <QCheckBox>
#include <QTabWidget>
#include <QWidget>

class QLabel;
class MeasurementDataCard;

struct MeasurementData {
    double gapWidth;
    double offset;
    double angle;
    bool gapWidthValid;
    bool offsetValid;
    bool angleValid;

    MeasurementData()
        : gapWidth(0.0)
        , offset(0.0)
        , angle(0.0)
        , gapWidthValid(false)
        , offsetValid(false)
        , angleValid(false)
    {}
};

struct DisplayOverlayFlags {
    bool showRoiFrame;
    bool showFittedCenterLine;
    bool showMeasurementPoints;
    bool showSubpixelContour;

    DisplayOverlayFlags()
        : showRoiFrame(true)
        , showFittedCenterLine(false)
        , showMeasurementPoints(true)
        , showSubpixelContour(false)
    {}
};

class ResultDisplayPanel : public QWidget {
    Q_OBJECT

public:
    explicit ResultDisplayPanel(QWidget *parent = nullptr);

    void updateMeasurementData(const MeasurementData &data);
    void setStatus(const QString &status);
    DisplayOverlayFlags getDisplayOverlayFlags() const;

signals:
    void displayOverlayFlagsChanged(const DisplayOverlayFlags &flags);

private slots:
    void onCheckboxToggled(bool checked);

private:
    void setupUi();
    void setupMeasurementResultsTab();
    void setupDisplaySettingsTab();

private:
    QTabWidget* m_tabWidget;

    QWidget* m_resultsTab;
    QLabel* m_statusLabel;
    MeasurementDataCard* m_gapWidthCard;
    MeasurementDataCard* m_offsetCard;
    MeasurementDataCard* m_angleCard;

    QWidget* m_settingsTab;
    QCheckBox* m_chkRoiFrame;
    QCheckBox* m_chkFittedCenterLine;
    QCheckBox* m_chkMeasurementPoints;
    QCheckBox* m_chkSubpixelContour;

    DisplayOverlayFlags m_overlayFlags;
};

#endif  // RESULT_DISPLAY_PANEL_H
