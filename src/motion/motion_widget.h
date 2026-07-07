#ifndef MOTION_WIDGET_H
#define MOTION_WIDGET_H

#include <QMetaType>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class MotionWidget;
}
QT_END_NAMESPACE

class MotionWidget : public QWidget {
    Q_OBJECT

public:
    MotionWidget(QWidget *parent = nullptr);
    ~MotionWidget();

    // ---- CISWidget 需要的接口（先给空壳）----
    double getCurrentXPosition() const;
    void setEditAbsPosition(const QString &position);
    void setEditSpeed(const QString &speed);

public slots:
    void on_btn_X_AbsPositionCommand_clicked();
    void on_chk_Stop_toggled(bool checked);

signals:
    // 地轨绝对定位完成
    void sendAbsFinished();

private:
    Ui::MotionWidget *ui;
};

#endif  // MOTION_WIDGET_H
