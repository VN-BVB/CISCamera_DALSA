#include "interactiveScene.h"
#include "interactiveView.h"
#include "interactiveImageItem.h"

#include <QGraphicsItem>

/*******************************/
// [InteractiveScenePrivate]
/*******************************/
class InteractiveScenePrivate
{
    Q_DISABLE_COPY(InteractiveScenePrivate) // 禁止拷贝赋值操作
    Q_DECLARE_PUBLIC(InteractiveScene)  // 设置公共访问权限
public:
    InteractiveScenePrivate(InteractiveScene* q):q_ptr(q)
    {
        roiDrawing = false;
    }
    virtual ~InteractiveScenePrivate(){}
public:
    InteractiveScene* const q_ptr;
    bool roiDrawing;
};

/*******************************/
// [InteractiveScene]
/*******************************/
InteractiveScene::InteractiveScene(InteractiveView *parentView)
    :QGraphicsScene(parentView),    // 派生类调用父类构造函数
    m_parentView(parentView),
    m_displayImageItem(new InteractiveImageItem(this)),
    d_ptr(new InteractiveScenePrivate(this))
{
    m_parentView->setScene(this);
    setDisplayImageItem(m_displayImageItem);
}

InteractiveScene::~InteractiveScene()
{}

bool InteractiveScene::whenDisplayImage(const QImage &image, bool bAutoFit)
{
    if (!m_displayImageItem) return false;
    emit sendUpdateDisplayImage(image);
    auto bRet =  m_displayImageItem->displayImage(image);
    if (!bRet) return false;
    m_parentView->whenUpdateDisplayFit();    // 更新父视图图像合适尺寸
    if (bAutoFit)
    {
        m_parentView->whenZoomToDisplayFit();
    }
    return true;
}

void InteractiveScene::whenClearImage()
{
    if (!m_displayImageItem) return ;
    m_displayImageItem->clearImage();
    emit sendClearDisplayImage();
}

void InteractiveScene::whenAddDisplayText(const QString &text, const QPointF &pt, const double &size,
                        const QColor &color, const bool &clear)
{
    if (!m_displayImageItem) return;
    m_displayImageItem->addDisplayText(text, pt, size, color, clear);
}

void InteractiveScene::whenClearDisplayText()
{
    if (!m_displayImageItem) return;
    m_displayImageItem->clearDisplayText();
}

QPixmap InteractiveScene::getDisplayImage()
{
    return m_displayImageItem->pixmap();
}

QSize InteractiveScene::getDisplayImageSize() const
{
    return m_displayImageItem->getDisplayImageSize();
}

void InteractiveScene::setDisplayImageItem(InteractiveImageItem* imageItem)
{
    // 遍历所有图元，确保只有一个图像图元在显示
    foreach (auto item, this->items())
    {
        if (item->type() == InteractiveImageItem::Type)
        {
            this->removeItem(item);
        }
    }
    imageItem->setPos(-0.5, -0.5);  // 向左上角位移半个像素，使像素中心对准场景坐标系中的坐标，因为图元坐标系以像素块中心而不是以像素块左上角为像素坐标，
    this->addItem(imageItem);
    m_parentView->setScene(this);
    if (m_displayImageItem != imageItem)
    {
        m_displayImageItem = imageItem;
    }
}

void InteractiveScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit sendMousePress(event);
    return QGraphicsScene::mousePressEvent(event);
}

void InteractiveScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    emit sendMouseRelease(event);
    return QGraphicsScene::mouseReleaseEvent(event);
}

void InteractiveScene::whenDrawSingleSubpixelContour(const std::vector<cv::Point2f> &subpixelContour)
{
    if (subpixelContour.empty())
        return;

    QPainterPath path;
    path.moveTo(subpixelContour[0].x, subpixelContour[0].y);    // 坐标增加(0.5,0.5)，以像素块中心为像素整数坐标，而不是左上角

    for (size_t i = 1; i < subpixelContour.size(); ++i) {
        path.lineTo(subpixelContour[i].x, subpixelContour[i].y);
    }

    QGraphicsPathItem *pathItem = new QGraphicsPathItem(path);
    QPen pen(Qt::red); // 亚像素用红色，像素级用绿色
    pen.setWidthF(0.1);
    pen.setStyle(Qt::DashLine);    // 亚像素级用虚线，像素级用实线
    pathItem->setPen(pen);
    pathItem->setZValue(10);

    this->addItem(pathItem);
}

void InteractiveScene::whenDrawSinglePixelContour(const std::vector<cv::Point> &pixelContour)
{
    if (pixelContour.empty())
        return;

    QPainterPath path;
    path.moveTo(pixelContour[0].x, pixelContour[0].y);    // 坐标增加(0.5,0.5)，以像素块中心为像素整数坐标，而不是左上角

    for (size_t i = 1; i < pixelContour.size(); ++i) {
        path.lineTo(pixelContour[i].x, pixelContour[i].y);
    }

    QGraphicsPathItem *pathItem = new QGraphicsPathItem(path);
    QPen pen(Qt::green); // 亚像素用红色，像素级用绿色
    pen.setWidthF(0.1);
    pen.setStyle(Qt::SolidLine);    // 亚像素级用虚线，像素级用实线
    pathItem->setPen(pen);
    pathItem->setZValue(10);

    this->addItem(pathItem);

    // 将轮廓坐标处的像素化成橙色
    for (const auto &point : pixelContour)
    {
        // 创建橙色矩形标记每个像素位置
        QGraphicsRectItem *pixelMarker = new QGraphicsRectItem(
            point.x - 0.4, point.y - 0.4, 0.8, 0.8);
        pixelMarker->setBrush(QBrush(QColor(255, 165, 0))); // 橙色填充
        pixelMarker->setPen(QPen(Qt::NoPen));               // 无边框
        pixelMarker->setZValue(11);                         // 比轮廓线更高，确保显示在上层
        this->addItem(pixelMarker);
    }
}

void InteractiveScene::whenDrawSubpixelContours(const std::vector<std::vector<cv::Point2f>> &subpixelContours)
{
    whenClearContours();
    for (auto contour : subpixelContours)
    {
        whenDrawSingleSubpixelContour(contour);
    }
}

void InteractiveScene::whenDrawPixelContours(const std::vector<std::vector<cv::Point>> &pixelContours)
{
    whenClearContours();
    for (auto contour : pixelContours)
    {
        whenDrawSinglePixelContour(contour);
    }
}

void InteractiveScene::whenClearContours()
{
    // @TODO:预留，之后将所有contour变成contourItem，然后用一个数组同一管理，清除时便清空这个数组
    return;
}

void InteractiveScene::whenDrawLines(const std::vector<cv::Vec4f> &lines)
{
    if (lines.empty())
        return;

    // 使用蓝色绘制直线，与轮廓的红色和绿色区分开
    QPen pen(Qt::blue);
    pen.setWidthF(0.2);  // 设置线宽
    pen.setStyle(Qt::SolidLine);  // 实线

    // 获取图像尺寸来确定直线的绘制范围
    QSize imageSize = getDisplayImageSize();
    if (imageSize.isEmpty()) {
        // 如果没有图像，使用默认的绘制范围
        imageSize = QSize(1000, 1000);
    }

    for (const auto& line : lines)
    {
        // cv::Vec4f包含直线参数：
        // [0]: 方向向量的x分量 (vx)
        // [1]: 方向向量的y分量 (vy)
        // [2]: 直线上一个点的x坐标 (x0)
        // [3]: 直线上一个点的y坐标 (y0)
        float vx = line[0];
        float vy = line[1];
        float x0 = line[2];
        float y0 = line[3];

        // 计算直线的起点和终点
        // 使用图像边界来确定直线的绘制范围
        float length = std::max(imageSize.width(), imageSize.height()) * 2.0f; // 足够长的直线

        // 计算起点和终点
        float x1 = x0 - length * vx;
        float y1 = y0 - length * vy;
        float x2 = x0 + length * vx;
        float y2 = y0 + length * vy;

        // 创建直线图元
        QGraphicsLineItem *lineItem = new QGraphicsLineItem(x1, y1, x2, y2);
        lineItem->setPen(pen);
        lineItem->setZValue(10);  // 设置Z值，确保显示在图像上方

        this->addItem(lineItem);
    }
}

void InteractiveScene::whenDrawPoints(const std::vector<cv::Point2f> &points)
{
    if (points.empty())
        return;

    int size = 5;
    for (const auto& point : points) {
        // 创建圆形标记点
        QGraphicsEllipseItem *pointItem = new QGraphicsEllipseItem(point.x - size/2, point.y - size/2, size, size);
        pointItem->setBrush(QBrush(QColor(255, 165, 255))); // 填充颜色
        pointItem->setPen(QPen(Qt::black)); // 黑色边框
        pointItem->setZValue(15); // 设置较高的Z值，确保显示在最上层

        this->addItem(pointItem);
    }
}














