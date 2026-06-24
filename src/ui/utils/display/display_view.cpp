#include "display_view.h"

#include <QGraphicsItem>
#include <QKeyEvent>
#include <QScrollBar>
#include <QWheelEvent>

#include "display_image_item.h"
#include "display_scene.h"

/*******************************/
//* [DisplayViewPrivate]
/*******************************/
class DisplayViewPrivate {
    Q_DISABLE_COPY(DisplayViewPrivate)
    Q_DECLARE_PUBLIC(DisplayView)

public:
    DisplayViewPrivate(DisplayView *q) : q_ptr(q) {
        resizeToFif = true;
        doubleClickToFit = true;

        maxZoomCoeff = ViewMaxZoomCoeff_Default;
        minZoomCoeff = ViewMinZoomCoeff_Default;
    }
    virtual ~DisplayViewPrivate() {}

public:
    void init();
    void updateBackground();

public:
    DisplayView *const q_ptr;

    bool resizeToFif;       // 是否重置尺寸缩放至合适大小
    bool doubleClickToFit;  // 是否双击缩放至合适大小

    double maxZoomCoeff;  // 最大缩放系数
    double minZoomCoeff;  // 最大缩放系数
};

/*******************************/
//* [DisplayView]
/*******************************/
#define VIEW_CENTER viewport()->rect().center()
#define VIEW_WIDTH viewport()->rect().width()
#define VIEW_HEIGHT viewport()->rect().height()

// 在构造函数中修改初始鼠标样式
DisplayView::DisplayView(QWidget *parent)
    : QGraphicsView(parent),
    m_translateButton(Qt::LeftButton),
    m_zoomDelta(0.1),
    m_translateSpeed(0.5),
    m_bMouseTranslate(false),
    m_currentMousePos(-1, -1),  // 初始化为无效位置
    d_ptr(new DisplayViewPrivate(this)),
    m_scene(new DisplayScene(this))
{
    // 去掉滚动条
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 不需要设置标准光标，我们将自定义绘制十字线
    setCursor(Qt::BlankCursor);             // 隐藏系统光标
    setRenderHint(QPainter::Antialiasing);  // 启用抗锯齿渲染

    setSceneRect(INT_MIN / 2, INT_MIN / 2, INT_MAX, INT_MAX);  // 设置场景矩形
    centerOn(0, 0);                                            // 将视图中心对准场景的（0， 0）点

    // 设置View属性
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);                  // 设置窗口更新模式为完全更新
    setDragMode(QGraphicsView::NoDrag);                                        // 平移使用自定义鼠标逻辑，禁用拖拽模式
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);                  // 缩放以光标处为锚点
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);                          // 窗口尺寸变化时以光标处为锚点
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);  // 设置渲染提示的组合
    setMouseTracking(true);                                                    // 启用鼠标跟踪
    setCacheMode(QGraphicsView::CacheBackground);  // 设置缓存模式为背景缓存，缓存视图的背景，提高重绘性能（在设置视图背景时有效，此项目没有设置黑白格等背景）
}

// 缩放的增量
void DisplayView::setZoomDelta(qreal delta) {
    // 建议增量范围
    Q_ASSERT_X(delta >= 0.0 && delta <= 1.0, "interactive_view::setZoomDelta", "Delta should be in range [0.0, 1.0].");
    m_zoomDelta = delta;
}

qreal DisplayView::zoomDelta() const { return m_zoomDelta; }

double DisplayView::maxZoomCoeff() const {
    Q_D(const DisplayView);
    return d->maxZoomCoeff;
}

void DisplayView::setMaxZoomCoeff(const double &coeff) {
    Q_D(DisplayView);
    d->maxZoomCoeff = coeff;
}

double DisplayView::minZoomCoeff() const {
    Q_D(const DisplayView);
    return d->minZoomCoeff;
}
void DisplayView::setMinZoomCoeff(const double &coeff) {
    Q_D(DisplayView);
    d->minZoomCoeff = coeff;
}

// 平移速度
void DisplayView::setTranslateSpeed(qreal speed) {
    // 建议速度范围
    Q_ASSERT_X(speed >= 0.0 && speed <= 2.0, "interactive_view::setTranslateSpeed", "Speed should be in range [0.0, 2.0].");
    m_translateSpeed = speed;
}

qreal DisplayView::translateSpeed() const { return m_translateSpeed; }

// 上/下/左/右键向各个方向移动、加/减键进行缩放、空格/回车键旋转
void DisplayView::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Up:
        translate(QPointF(0, -2));  // 上移
        break;
    case Qt::Key_Down:
        translate(QPointF(0, 2));  // 下移
        break;
    case Qt::Key_Left:
        translate(QPointF(-2, 0));  // 左移
        break;
    case Qt::Key_Right:
        translate(QPointF(2, 0));  // 右移
        break;
    case Qt::Key_Plus:  // 放大
        zoomUp();
        break;
    case Qt::Key_Minus:  // 缩小
        zoomDown();
        break;
    case Qt::Key_Space:  // 逆时针旋转
        rotate(-5);
        break;
    case Qt::Key_Enter:  // 顺时针旋转
    case Qt::Key_Return:
        rotate(5);
        break;
    default:
        QGraphicsView::keyPressEvent(event);
    }
}

// 平移
void DisplayView::mouseMoveEvent(QMouseEvent *event) {
    if (m_bMouseTranslate) {
        // 按视图像素增量滚动，与当前缩放级别无关，实现 1:1 跟手
        QPoint delta = event->pos() - m_lastMousePos;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
    }

    // 更新当前鼠标位置并触发重绘
    m_currentMousePos = event->pos();
    m_lastMousePos = event->pos();
    viewport()->update();  // 触发重绘

    QGraphicsView::mouseMoveEvent(event);
}

// 在鼠标按下事件中修改光标样式
void DisplayView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == m_translateButton) {
        m_bMouseTranslate = true;
        m_lastMousePos = event->pos();
        // 不需要改变光标样式，保持隐藏状态
    }

    QGraphicsView::mousePressEvent(event);
}

// 在鼠标释放事件中修改光标样式
void DisplayView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == m_translateButton) {
        m_bMouseTranslate = false;
        // 不需要改变光标样式，保持隐藏状态
    }

    QGraphicsView::mouseReleaseEvent(event);
}

// 添加paintEvent方法绘制贯穿窗口的黑色十字线
void DisplayView::paintEvent(QPaintEvent *event) {
    // 首先调用基类的paintEvent绘制场景内容
    QGraphicsView::paintEvent(event);

    // 如果鼠标位置有效，则绘制十字线
    if (m_currentMousePos.x() >= 0 && m_currentMousePos.y() >= 0) {
        QPainter painter(viewport());
        QPen pen(Qt::green, 1, Qt::SolidLine);
        painter.setPen(pen);
        // 获取视口矩形
        QRect viewportRect = viewport()->rect();

        // 绘制垂直线（贯穿整个窗口高度）
        painter.drawLine(m_currentMousePos.x(), 0, m_currentMousePos.x(), viewportRect.height());

        // 绘制水平线（贯穿整个窗口宽度）
        painter.drawLine(0, m_currentMousePos.y(), viewportRect.width(), m_currentMousePos.y());
    }
}

void DisplayView::mouseDoubleClickEvent(QMouseEvent *event) {
    Q_D(DisplayView);
    if (d->doubleClickToFit) {
        whenZoomToDisplayFit();
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}

// 放大/缩小
void DisplayView::wheelEvent(QWheelEvent *event) {
    Q_D(DisplayView);
    int deltaY = event->angleDelta().y();
    if ((deltaY > 0) && (m_rZoomValue >= d->maxZoomCoeff))  // 最大放大
    {
        return;
    } else if ((deltaY < 0) && (m_rZoomValue <= d->minZoomCoeff))  // 最小缩小
    {
        return;
    } else {
        double tmp = m_rZoomValue;
        if (deltaY > 0) {
            tmp *= 1.1;
        } else {
            tmp *= 0.9;
        }
        zoomByValue(tmp);
    }
}

// 放大
void DisplayView::zoomUp() {
    Q_D(DisplayView);
    if (m_rZoomValue >= d->maxZoomCoeff)  // 最大放大
    {
        return;
    } else {
        double tmp = m_rZoomValue;
        tmp *= (1 + m_zoomDelta);  // 每次放大10%
        zoomByValue(tmp);
    }
}

// 缩小
void DisplayView::zoomDown() {
    Q_D(DisplayView);
    if (m_rZoomValue <= d->minZoomCoeff)  // 最小缩小
    {
        return;
    } else {
        double tmp = m_rZoomValue;
        tmp *= (1 - m_zoomDelta);  // 每次缩小10%
        zoomByValue(tmp);
    }
}

// 平移
void DisplayView::translate(QPointF delta) {
    // 按视图像素增量滚动，与缩放级别无关；键盘方向键也走此路径
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() - static_cast<int>(delta.x()));
    verticalScrollBar()->setValue(verticalScrollBar()->value() - static_cast<int>(delta.y()));
}

void DisplayView::whenUpdateDisplayFit() {
    // 获取场景中所有图元的外接矩形
    QRectF sceneBoundingRect = m_scene->itemsBoundingRect();

    if (sceneBoundingRect.isEmpty() || this->width() < 1) {
        return;
    }

    // 计算缩放比例，确保所有图元都能显示在视图中
    double winWidth = this->width() - 20;  // 减去边距
    double winHeight = this->height() - 20;

    double scaleWidth = sceneBoundingRect.width() / winWidth;
    double scaleHeight = sceneBoundingRect.height() / winHeight;

    // 取较大的缩放比例，确保所有内容都能显示
    double scale = std::max(scaleWidth, scaleHeight);
    double s = (scale > 0) ? 1 / scale : 1.0;

    // 计算中心点位置
    double centerX = sceneBoundingRect.center().x();
    double centerY = sceneBoundingRect.center().y();

    // 更新缩放和位置信息
    if (m_rZoomFit != s || m_rFitPixX != centerX * s || m_rFitPixY != centerY * s) {
        m_rZoomFit = s;
        m_rFitPixX = centerX * s;
        m_rFitPixY = centerY * s;
        whenZoomToDisplayFit();
    }
}

/*
 将图像缩放到合适视图的大小，并调整显示位置
*/
void DisplayView::whenZoomToDisplayFit() {
    // 基于当前视口和场景重新计算 fit 参数，避免使用过期缓存（窗口尺寸变化、图像更换等）
    QRectF sceneBoundingRect = m_scene->itemsBoundingRect();
    if (!sceneBoundingRect.isEmpty() && viewport()->width() > 0) {
        double winWidth = viewport()->width() - 20.0;   // 留少量边距
        double winHeight = viewport()->height() - 20.0;
        double scale = std::max(sceneBoundingRect.width() / winWidth,
                                sceneBoundingRect.height() / winHeight);
        m_rZoomFit = (scale > 0) ? 1.0 / scale : 1.0;
        m_rFitPixX = sceneBoundingRect.center().x() * m_rZoomFit;
        m_rFitPixY = sceneBoundingRect.center().y() * m_rZoomFit;
    }

    // fit 是居中显示，临时切回中心锚点，避免受 AnchorUnderMouse 影响
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);

    // 先重置缩放
    this->resetTransform();

    // 应用新的缩放比例
    this->scale(m_rZoomFit, m_rZoomFit);

    // 滚动到中心位置
    this->centerOn(m_rFitPixX / m_rZoomFit, m_rFitPixY / m_rZoomFit);

    // 确保所有内容都在视图内
    this->ensureVisible(m_scene->itemsBoundingRect());

    // 同步当前缩放值，避免后续 wheel/+/- 的钳制与比例计算失真
    m_rZoomValue = m_rZoomFit;

    // 恢复光标锚点，便于后续缩放
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

void DisplayView::zoomByValue(const double &val) {
    double tmp = val / m_rZoomValue;
    // 绝对缩放
    m_rZoomValue *= tmp;
    // 相对于上一次缩放
    this->scale(tmp, tmp);  // 在x，y方向应用相同的缩放因子
}
