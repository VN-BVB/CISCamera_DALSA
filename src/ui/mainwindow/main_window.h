#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include <memory>

class MeasurementMonitor;
class MeasurementPipeline;
class MeasurementPresenter;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupUi();
    void setupMenuBar();
    void setupPages();
    void setupConnections();

private slots:
    void onMenuButtonClicked(int index);

private:
    QWidget* m_menuBarWidget;
    QPushButton* m_btnMeasurementMonitor;
    QPushButton* m_btnSystemCalibration;
    QPushButton* m_btnConfigManagement;
    QStackedWidget* m_stackedWidget;

    MeasurementMonitor* m_measurementMonitor;
    QWidget* m_systemCalibrationPage;
    QWidget* m_configManagementPage;

    std::unique_ptr<MeasurementPipeline>  m_pipeline;
    std::unique_ptr<MeasurementPresenter> m_presenter;
};

#endif  // MAIN_WINDOW_H
