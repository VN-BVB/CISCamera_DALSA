#ifndef MOTION_WIDGET_H
#define MOTION_WIDGET_H

#include <QMetaType>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class MotionWidget;
}
QT_END_NAMESPACE

class MotionController;

class MotionWidget : public QWidget {
    Q_OBJECT

public:
    MotionWidget(QWidget *parent = nullptr);
    ~MotionWidget();

    // ---- CISWidget 需要的接口 ----
    double getCurrentXPosition() const;
    void setEditAbsPosition(const QString &position);
    void setEditSpeed(const QString &speed);

public slots:
    void on_btn_X_AbsPositionCommand_clicked();
    void on_chk_Stop_toggled(bool checked);

    // 单平台运动 / 单轴运动
    void on_plt_single_move_clicked();
    void on_plt_single_axis_move_clicked();

signals:
    void sendAbsFinished();  // 地轨绝对定位完成 → CISWidget

private slots:
    void connectRail();
    void disconnectRail();
    void on_rail_btn_stop_clicked();

    // 接收 MotionController 信号
    void onRailPosVelUpdated(double pos, double vel);
    void onConnectionStateChanged(const QString &color);
    void onLogMessage(const QString &msg);
    void onPltAxisEnableStatus(const QVector<QVector<bool>> &status);

private:
    Ui::MotionWidget *ui;

    MotionController *controller_ = nullptr;
    QThread *ctrlThread_ = nullptr;

    QString ip_ = "192.168.6.6";
    int port_ = 502;

    friend class CISWidget;
};
#endif  // MOTION_WIDGET_H
