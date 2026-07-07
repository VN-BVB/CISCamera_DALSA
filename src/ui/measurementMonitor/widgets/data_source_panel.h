#ifndef DATA_SOURCE_PANEL_H
#define DATA_SOURCE_PANEL_H

#include <QPushButton>
#include <QStackedWidget>
#include <QWidget>

class QLabel;

class DataSourcePanel : public QWidget {
    Q_OBJECT

public:
    enum class Mode { SharedMemory, LocalFile };

    explicit DataSourcePanel(QWidget *parent = nullptr);

    void setConnectionStatus(bool connected);
    void setFilePath(const QString &path);
    void setMeasurementEnabled(bool enabled);
    Mode getCurrentMode() const { return m_currentMode; }
    bool isConnected() const { return m_isConnected; }

signals:
    void modeChanged(Mode mode);
    void connectSharedMemoryRequested(int processId);
    void disconnectSharedMemoryRequested();
    void startAutoMeasurementRequested();
    void stopAutoMeasurementRequested();
    void startImageRead(const QString &path);
    void executeSingleMeasurementRequested();

private slots:
    void onModeButtonClicked();
    void onConnectClicked();
    void onOpenFileClicked();
    void onExecuteClicked();

private:
    void setupUi();
    void updateModeUI();
    void updateConnectionUI();

private:
    Mode m_currentMode;
    bool m_isConnected;
    QString m_currentFilePath;

    QPushButton* m_btnSharedMemory;
    QPushButton* m_btnLocalFile;
    QStackedWidget* m_modeStack;

    QWidget* m_sharedMemoryPage;
    QLabel* m_lblConnectionStatus;
    QPushButton* m_btnConnect;

    QWidget* m_localFilePage;
    QLabel* m_lblFilePath;
    QPushButton* m_btnOpenFile;

    QPushButton* m_btnExecute;
};

#endif  // DATA_SOURCE_PANEL_H
