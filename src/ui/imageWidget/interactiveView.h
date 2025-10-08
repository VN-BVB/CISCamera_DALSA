#ifndef INTERACTIVE_VIEW_H
#define INTERACTIVE_VIEW_H

#include "interactiveGlobal.h"
#include <QGraphicsView>

const double ViewMaxZoomCoeff_Default = 50;
const double ViewMinZoomCoeff_Default = 0.1;

class QWheelEvent;
class QKeyEvent;
class InteractiveScene;
class InteractiveImageItem;
class InteractiveViewPrivatel;

class InteractiveView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit InteractiveView(QWidget *parent = 0);

    // 平移速度
    void setTranslateSpeed(qreal speed);
    qreal translateSpeed() const;

    // 缩放的增量
    void setZoomDelta(qreal delta);
    qreal zoomDelta() const;

protected:
    // 上/下/左/右键向各个方向移动、加/减键进行缩放、空格/回车键旋转
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE; // Q_DECL_OVERRIDE宏标记重写基类的虚函数， 可以换成override
    // 平移
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    // 放大/缩小
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;

public Q_SLOTS:
    void zoomIn();                 // 放大
    void zoomOut();                // 缩小
    void zoom(float scaleFactor);  // 缩放
    void translate(QPointF delta); // 平移

public slots:
    // 更新缩放自适应显示大小参数
    void whenUpdateDisplayFit();
    // 缩放到自适应显示
    void whenZoomToDisplayFit();

protected:
    // 进行缩放
    void zoomByValue(const double &val);

private:
    Qt::MouseButton m_translateButton; // 平移按钮
    qreal m_translateSpeed;            // 平移速度
    qreal m_zoomDelta;                 // 缩放的增量
    bool m_bMouseTranslate;            // 平移标识
    QPoint m_lastMousePos;             // 鼠标最后按下的位置
    qreal m_scale;                     // 缩放值

protected: //view控件状态
    // 当前缩放值
    double m_rZoomValue = 1;
    // 缩放至适合比例
    double m_rZoomFit = 1;
    // 自适应缩放时的适合图像X坐标
    double m_rFitPixX = 0;
    // 自适应缩放时的适合图像Y坐标
    double m_rFitPixY = 0;

protected:
    // 显示的场景
    InteractiveScene *m_scene;
};

#endif // INTERACTIVE_VIEW_H
