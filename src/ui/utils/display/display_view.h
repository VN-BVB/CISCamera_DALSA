#ifndef DISPLAY_VIEW_H
#define DISPLAY_VIEW_H

#include <QGraphicsView>

#include "display_global.h"

const double ViewMaxZoomCoeff_Default = 100;
const double ViewMinZoomCoeff_Default = 0.00001;

class QWheelEvent;
class QKeyEvent;
class DisplayScene;
class DisplayImageItem;
class DisplayViewPrivate;

class DisplayView : public QGraphicsView {
    Q_OBJECT

    friend class DisplayScene;

public:
    explicit DisplayView(QWidget *parent = 0);

    // 缩放的增量
    void setZoomDelta(qreal delta);
    qreal zoomDelta() const;
    // 最大缩放系数
    double maxZoomCoeff() const;
    // 设置最大缩放系数
    void setMaxZoomCoeff(const double &coeff);
    // 最小缩放系数
    double minZoomCoeff() const;
    // 设置最小缩放系数
    void setMinZoomCoeff(const double &coeff);

    // 平移速度
    void setTranslateSpeed(qreal speed);
    qreal translateSpeed() const;

public:  // 公共接口
    // 获取场景
    DisplayScene *getScene() { return m_scene; }

protected:  // 视窗事件
    // 上/下/左/右键向各个方向移动、加/减键进行缩放、空格/回车键旋转
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;  // Q_DECL_OVERRIDE宏标记重写基类的虚函数， 可以换成override
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    // 重写paintEvent绘制十字线
    void paintEvent(QPaintEvent *event) override;

public slots:
    void zoomUp();                  // 放大
    void zoomDown();                // 缩小
    void translate(QPointF delta);  // 平移
    // 更新缩放自适应显示大小参数
    void whenUpdateDisplayFit();
    // 缩放到自适应显示
    void whenZoomToDisplayFit();

signals:
    // 鼠标移动时发出，携带场景坐标系下的位置
    void sendMouseScenePos(const QPointF& scenePos);

private:
    // 在类的私有成员变量部分添加
private:
    Qt::MouseButton m_translateButton;  // 平移按钮
    qreal m_translateSpeed;             // 平移速度
    qreal m_zoomDelta;                  // 缩放的增量
    bool m_bMouseTranslate;             // 平移标识
    QPoint m_lastMousePos;              // 鼠标最后按下的位置
    QPoint m_currentMousePos;           // 当前鼠标位置，用于绘制十字线

    // 在protected部分添加paintEvent声明
protected:
    // 进行缩放
    void zoomByValue(const double &val);

protected:  // view控件状态
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
    DisplayScene *m_scene;

protected:
    const QScopedPointer<DisplayViewPrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(DisplayView)
};

#endif  // DISPLAY_VIEW_H
