#ifndef RAIL_WIDGET_H
#define RAIL_WIDGET_H

#include <QMetaType>
#include <QWidget>

#include "plc_variableaddress.h"
#include "rail.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class RailWidget;
}
QT_END_NAMESPACE

class RailWidget : public QWidget {
    Q_OBJECT

public:
    RailWidget(QWidget *parent = nullptr);
    ~RailWidget();

    void updatePositionUI(float position);
    void updateSpeedUI(float speed);
    void setEditAbsPosition(QString position);
    void setEditSpeed(QString speed);

signals:
    void sendConnectToPLC(QString ip, int port);
    void sendDisconnectToPLC();
    void sendWriteCoils(int address, const QVector<bool> &values);
    void sendWriteRegisters(int address, const QVector<quint16> &values);
    void sendMove2AbsPosition(float val, float pos);  // 地轨移动到指定位置
    void sendForward(float vel);                      // 正向点动
    void sendReverse(float vel);                      // 反向点动

public slots:
    void whenAppendCalibrationLog(const QString message);
    void whenUpdatePositionAndSpeed(float position, float speed);
    void whenUpdateAMState(const QString messageAxis, const QString messageMotion);

private slots:
    void connectRail();
    void disconnectRail();
    void on_btn_X_AbsPositionCommand_clicked();
    void on_chk_Stop_toggled(bool checked);
    void on_chk_ImmediateStop_toggled(bool checked);
    void on_btn_X_JogForward_pressed();
    void on_btn_X_JogForward_released();
    void on_btn_X_JogReverse_pressed();
    void on_btn_X_JogReverse_released();
    void on_btn_regressOrigin_clicked();
    void on_horizontalSlider_X_AbsPosition_sliderMoved(int position);
    void on_horizontalSlider_X_AbsSpeed_sliderMoved(int position);
    void on_btn_chk_Rest_clicked();
    void on_edit_X_AbsSpeed_textChanged(const QString &arg1);

private:
    Ui::RailWidget *ui;

    Rail *rail = new Rail;
    QThread *railThread = new QThread;  // 标定线程
    QString ip = "192.168.100.88";
    int port = 502;

    friend class RailWeldingMainWindow;
};
#endif  // RAIL_WIDGET_H
