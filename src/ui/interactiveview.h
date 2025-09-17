#ifndef INTERACTIVE_VIEW_H
#define INTERACTIVE_VIEW_H

#include <QGraphicsView>

/*
使用前向声明，比直接包含头文件（#include <QWheelEvent>）效率高，优化编译效率
为什么可以这样：
    因为当只使用类的指针或引用时，编译器只需要知道类的存在，不需要知道类的完整定义
    只有在调用类的成员函数或访问成员变量时才需要完整定义
这种做法是常见的最佳实践：
    头文件：使用前向声明，最小化包含关系
    源文件：包含完整头文件，使用类的完整功能
*/
class QWheelEvent;
class QKeyEvent;

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
    void zoom(float scaleFactor);  // 缩放 - scaleFactor：缩放的比例因子
    void translate(QPointF delta); // 平移

private:
    Qt::MouseButton m_translateButton; // 平移按钮
    qreal m_translateSpeed;            // 平移速度
    qreal m_zoomDelta;                 // 缩放的增量
    bool m_bMouseTranslate;            // 平移标识
    QPoint m_lastMousePos;             // 鼠标最后按下的位置
    qreal m_scale;                     // 缩放值
};

#endif // INTERACTIVE_VIEW_H
