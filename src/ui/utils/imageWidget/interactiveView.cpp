#include <QWheelEvent>
#include <QKeyEvent>
#include <QGraphicsItem>
#include <QScrollBar>
#include "interactiveView.h"
#include "interactiveScene.h"
#include "interactiveImageItem.h"

/*******************************/
//* [InteractiveViewPrivate]
/*******************************/
class InteractiveViewPrivate
{
    Q_DISABLE_COPY(InteractiveViewPrivate)
    Q_DECLARE_PUBLIC(InteractiveView)

public:
    InteractiveViewPrivate(InteractiveView *q):q_ptr(q)
    {
        resizeToFif=true;
        doubleClickToFit=true;

        maxZoomCoeff=ViewMaxZoomCoeff_Default;
        minZoomCoeff=ViewMinZoomCoeff_Default;
    }
    virtual ~InteractiveViewPrivate(){}

public:
    void init();
    void updateBackground();
public:
    InteractiveView              *const q_ptr;

    bool                        resizeToFif;//是否重置尺寸缩放至合适大小
    bool                        doubleClickToFit;//是否双击缩放至合适大小

    double                      maxZoomCoeff;//最大缩放系数
    double                      minZoomCoeff;//最大缩放系数
};



/*******************************/
//* [InteractiveView]
/*******************************/
#define VIEW_CENTER viewport()->rect().center()
#define VIEW_WIDTH viewport()->rect().width()
#define VIEW_HEIGHT viewport()->rect().height()

InteractiveView::InteractiveView(QWidget *parent)
    : QGraphicsView(parent),
    m_translateButton(Qt::LeftButton),
    m_scale(1.0),
    m_zoomDelta(0.1),
    m_translateSpeed(0.5),
    m_bMouseTranslate(false),
    d_ptr(new InteractiveViewPrivate(this)),
    m_scene(new InteractiveScene(this))
{
    // 去掉滚动条
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setCursor(Qt::PointingHandCursor);  // 设置鼠标光标为手型指针
    setRenderHint(QPainter::Antialiasing);  // 启用抗锯齿渲染

    setSceneRect(INT_MIN / 2, INT_MIN / 2, INT_MAX, INT_MAX);   // 设置场景矩形
    centerOn(0, 0); // 将视图中心对准场景的（0， 0）点

    //设置View属性
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);   // 设置窗口更新模式为完全更新
    setDragMode(QGraphicsView::RubberBandDrag); // 设置拖拽模式为橡皮筋选择模式
    setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform); // 设置渲染提示的组合
    setMouseTracking(true); // 启用鼠标跟踪
    setCacheMode(QGraphicsView::CacheBackground);   // 设置缓存模式为背景缓存，缓存视图的背景，提高重绘性能（在设置视图背景时有效，此项目没有设置黑白格等背景）
}

// 平移速度
void InteractiveView::setTranslateSpeed(qreal speed)
{
    // 建议速度范围
    Q_ASSERT_X(speed >= 0.0 && speed <= 2.0,
               "interactive_view::setTranslateSpeed", "Speed should be in range [0.0, 2.0].");
    m_translateSpeed = speed;
}

qreal InteractiveView::translateSpeed() const
{
    return m_translateSpeed;
}

// 缩放的增量
void InteractiveView::setZoomDelta(qreal delta)
{
    // 建议增量范围
    Q_ASSERT_X(delta >= 0.0 && delta <= 1.0,
               "interactive_view::setZoomDelta", "Delta should be in range [0.0, 1.0].");
    m_zoomDelta = delta;
}

qreal InteractiveView::zoomDelta() const
{
    return m_zoomDelta;
}

// 上/下/左/右键向各个方向移动、加/减键进行缩放、空格/回车键旋转
void InteractiveView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Up:
        translate(QPointF(0, -2)); // 上移
        break;
    case Qt::Key_Down:
        translate(QPointF(0, 2)); // 下移
        break;
    case Qt::Key_Left:
        translate(QPointF(-2, 0)); // 左移
        break;
    case Qt::Key_Right:
        translate(QPointF(2, 0)); // 右移
        break;
    case Qt::Key_Plus: // 放大
        zoomIn();
        break;
    case Qt::Key_Minus: // 缩小
        zoomOut();
        break;
    case Qt::Key_Space: // 逆时针旋转
        rotate(-5);
        break;
    case Qt::Key_Enter: // 顺时针旋转
    case Qt::Key_Return:
        rotate(5);
        break;
    default:
        QGraphicsView::keyPressEvent(event);
    }
}

// 平移
void InteractiveView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_bMouseTranslate)
    {
        QPointF mouseDelta = mapToScene(event->pos()) - mapToScene(m_lastMousePos);
        translate(mouseDelta);
    }

    m_lastMousePos = event->pos();

    QGraphicsView::mouseMoveEvent(event);
}

void InteractiveView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == m_translateButton)
    {
        // // 当光标底下没有 item 时，才能移动
        // QPointF point = mapToScene(event->pos());
        // if (scene()->itemAt(point, transform()) == NULL)
        // {
        //     m_bMouseTranslate = true;
        //     m_lastMousePos = event->pos();
        // }
        m_bMouseTranslate = true;
        m_lastMousePos = event->pos();
    }

    QGraphicsView::mousePressEvent(event);
}

void InteractiveView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == m_translateButton)
        m_bMouseTranslate = false;

    QGraphicsView::mouseReleaseEvent(event);
}

// 放大/缩小
void InteractiveView::wheelEvent(QWheelEvent *event)
{
    // 滚轮的滚动量
    QPoint scrollAmount = event->angleDelta();
    // 正值表示滚轮远离使用者（放大），负值表示朝向使用者（缩小）
    scrollAmount.y() > 0 ? zoomIn() : zoomOut();
}

// 放大
void InteractiveView::zoomIn()
{
    zoom(1 + m_zoomDelta);
}

// 缩小
void InteractiveView::zoomOut()
{
    zoom(1 - m_zoomDelta);
}

// 缩放 - scaleFactor：缩放的比例因子
void InteractiveView::zoom(float scaleFactor)
{
    // 防止过小或过大
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 500) return;

    scale(scaleFactor, scaleFactor);
    m_scale *= scaleFactor;
}

// 平移
void InteractiveView::translate(QPointF delta)
{
    // 根据当前 zoom 缩放平移数
    delta *= m_scale;
    delta *= m_translateSpeed;

    // 获取当前场景中的所有items的边界矩形
    QRectF  scene_bounds;
    QList<QGraphicsItem*> items = scene()->items();
    if (!items.isEmpty()) {
        scene_bounds = items.first()->sceneBoundingRect();
        for (int i = 1; i < items.size(); ++i)
        {
            scene_bounds = scene_bounds.united(items[i]->sceneBoundingRect());
        }
    }
    else {
        // 如果没有items，允许平移
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        QPoint newCenter(VIEW_WIDTH / 2 - delta.x(), VIEW_HEIGHT / 2 - delta.y());
        centerOn(mapToScene(newCenter));
        setTransformationAnchor(QGraphicsView::AnchorViewCenter);
        return;
    }

    // 获取当前视窗在场景中的矩形
    QRectF viewRect = mapToScene(viewport()->rect()).boundingRect();

    // 计算平移后的视窗位置
    QRectF newViewRect = viewRect.translated(-delta);


    bool canTranslate = true;
    // 检查边界,是否已经达到视窗边缘
    // if (scene_bounds.left() >= newViewRect.left() && delta.x() > 0)
    // {
    //     canTranslate = false;
    // }
    // if (scene_bounds.right() <= newViewRect.right() && delta.x() < 0)
    // {
    //     canTranslate = false;
    // }
    // if (scene_bounds.top() >= newViewRect.top() && delta.y() > 0)
    // {
    //     canTranslate = false;
    // }
    // if (scene_bounds.bottom() <= newViewRect.bottom() && delta.y() < 0)
    // {
    //     canTranslate = false;
    // }

    if (canTranslate)
    {
        // view 根据鼠标下的点作为锚点来定位 scene
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        QPoint newCenter(VIEW_WIDTH / 2 - delta.x(), VIEW_HEIGHT / 2 - delta.y());
        centerOn(mapToScene(newCenter));

        // scene 在 view 的中心点作为锚点
        setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    }
}

void InteractiveView::whenUpdateDisplayFit()
{
    int imageWidth = m_scene->getDisplayImageSize().width();
    int imageHeight = m_scene->getDisplayImageSize().height();
    if (this->width() < 1 || imageWidth < 1)
    {
        return;
    }
    // 图像自适应方法
    double winWidth = this->width();
    double winHeight = this->height();
    double scaleWidth = (imageWidth + 1) / winWidth;    // 加1确保后续流程正确，防止除零错误、比较错误等
    double scaleHeight = (imageHeight + 1) / winHeight;
    double row1, column1;
    double s = 0;
    if (scaleWidth >= scaleHeight)
    {
        row1= -(1) * ((winHeight * scaleWidth) - imageHeight) / 2;
        // row1 = 0;
        column1 = 0;
        s = 1 / scaleWidth;
    }
    else
    {
        row1= 0;
        column1 = -(1.0) * ((winWidth * scaleHeight) - imageWidth) / 2 ;
        // column1 = 0;
        s=1 / scaleHeight;
    }


    if (m_rZoomFit != s || m_rFitPixX != column1 * s)
    {
        m_rZoomFit = s;
        m_rFitPixX = column1 * s;
        m_rFitPixY = row1 * s;
        whenZoomToDisplayFit();
    }
}

/*
 将图像缩放到合适视图的大小，并调整显示位置
*/
void InteractiveView::whenZoomToDisplayFit()
{
    zoomByValue(m_rZoomFit);
    QScrollBar *pHbar = this->horizontalScrollBar();
    pHbar->setSliderPosition(m_rFitPixX);
    QScrollBar *pVbar = this->verticalScrollBar();
    pVbar->setSliderPosition(m_rFitPixY);
    // centerOn(m_scene->getDisplayImageItem()->getDisplayImageCenter());
}

void InteractiveView::zoomByValue(const double &val)
{
    double tmp = val / m_rZoomValue;
    // 绝对缩放
    m_rZoomValue *= tmp;
    // 相对于上一次缩放
    this->scale(tmp, tmp);  // 在x，y方向应用相同的缩放因子
}

double InteractiveView::maxZoomCoeff() const
{
    Q_D(const InteractiveView);
    return d->maxZoomCoeff;
}

void InteractiveView::setMaxZoomCoeff(const double &coeff)
{
    Q_D(InteractiveView);
    d->maxZoomCoeff=coeff;
}

double InteractiveView::minZoomCoeff() const
{
    Q_D(const InteractiveView);
    return d->minZoomCoeff;
}
void InteractiveView::setMinZoomCoeff(const double &coeff)
{
    Q_D(InteractiveView);
    d->minZoomCoeff=coeff;
}



























