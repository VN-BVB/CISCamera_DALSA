#include <QGraphicsItem>
#include <QGraphicsProxyWidget>

#include "display_scene.h"
#include "display_view.h"
#include "display_image_item.h"
#include "graphicItems/graphic_item_component.h"
#include "graphicItems/graphic_item_composite.h"
#include "graphicItems/line_item.h"
#include "graphicItems/point_item.h"
#include "graphicItems/bspline_item.h"
#include "graphicItems/rotated_rect_item.h"

/*******************************/
// [DisplayScenePrivate]
/*******************************/
class DisplayScenePrivate
{
    Q_DISABLE_COPY(DisplayScenePrivate) // 禁止拷贝赋值操作
    Q_DECLARE_PUBLIC(DisplayScene)  // 设置公共访问权限
public:
    DisplayScenePrivate(DisplayScene* q):q_ptr(q)
    {
        roiDrawing = false;
    }
    virtual ~DisplayScenePrivate(){}
public:
    DisplayScene* const q_ptr;
    bool roiDrawing;
};

/*******************************/
// [DisplayScene]
/*******************************/
DisplayScene::DisplayScene(DisplayView *parentView)
    :QGraphicsScene(parentView),    // 派生类调用父类构造函数
    m_parentView(parentView),
    m_displayImageItem(new DisplayImageItem(this)),
    m_graphicItemComposite(std::make_shared<GraphicItemComposite>()),
    d_ptr(new DisplayScenePrivate(this))
{
    m_parentView->setScene(this);
    setDisplayImageItem(m_displayImageItem);
}

DisplayScene::~DisplayScene()
{}

bool DisplayScene::whenDisplayImage(const QImage &image, bool bAutoFit)
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

void DisplayScene::whenClearImage()
{
    if (!m_displayImageItem) return ;
    m_displayImageItem->clearImage();
    emit sendClearDisplayImage();
}

void DisplayScene::whenAddDisplayText(const QString &text, const QPointF &pt, const double &size,
                                      const QColor &color, const bool &clear)
{
    if (!m_displayImageItem) return;
    m_displayImageItem->addDisplayText(text, pt, size, color, clear);
}

void DisplayScene::whenClearDisplayText()
{
    if (!m_displayImageItem) return;
    m_displayImageItem->clearDisplayText();
}

QPixmap DisplayScene::getDisplayImage()
{
    return m_displayImageItem->pixmap();
}

QSize DisplayScene::getDisplayImageSize() const
{
    return m_displayImageItem->getDisplayImageSize();
}

void DisplayScene::setDisplayImageItem(DisplayImageItem* imageItem)
{
    // 遍历所有图元，确保只有一个图像图元在显示
    foreach (auto item, this->items())
    {
        if (item->type() == DisplayImageItem::Type)
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

void DisplayScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit sendMousePress(event);
    return QGraphicsScene::mousePressEvent(event);
}

void DisplayScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    emit sendMouseRelease(event);
    return QGraphicsScene::mouseReleaseEvent(event);
}

void DisplayScene::addGraphicComponent(std::shared_ptr<GraphicsItemComponent> component)
{
    m_graphicItemComposite->addComponent(component);
}

void DisplayScene::removeGraphicComponent(std::shared_ptr<GraphicsItemComponent> component)
{
    m_graphicItemComposite->removeComponent(component);
}

// 显示所有图形组件
void DisplayScene::showAllGraphicComponents()
{
    m_graphicItemComposite->addToScene(this);
}

void DisplayScene::clearAllGraphicComponents()
{
    m_graphicItemComposite->removeFromScene(this);
}

void DisplayScene::whenDrawContours(const std::vector<std::vector<cv::Point2f>> &contours,
                      const QColor &color,
                      double lineWidth,
                      Qt::PenStyle lineStyle,
                      double zValue)
{
    for (const auto& contour : contours) {
        if (!contour.empty()) {
            auto contourComponent = std::make_shared<ContourItem> (contour,ContourItem::subpixelContour,color, lineWidth, lineStyle, zValue);
            addGraphicComponent(contourComponent);
            showAllGraphicComponents();
        }
    }
}

void DisplayScene::whenDrawPoints(const std::vector<cv::Point2f> &points,
                                  const QColor& color,
                                  double size,
                                  double zValue)
{
    if (points.empty()) return;

    auto pointComponent = std::make_shared<PointItem>(points);
    addGraphicComponent(pointComponent);
    showAllGraphicComponents();
}

void DisplayScene::whenDrawLines(const std::vector<cv::Vec4f> &lines, const double length, const QColor &color)
{
    for (const auto& line : lines) {
        auto lineComponent = std::make_shared<LineItem>(line, length, color);
        addGraphicComponent(lineComponent);
    }
    showAllGraphicComponents();
}

// 修改绘制单个B样条曲线的方法
void DisplayScene::whenDrawSingleBSplineCurve(const std::vector<cv::Point2f> &controlPoints)
{
    if (controlPoints.size() < 2) {
        return;
    }

    // 创建B样条曲线组件
    auto splineComponent = std::make_shared<BSplineItem>(controlPoints);
    addGraphicComponent(splineComponent);
    showAllGraphicComponents();
}

// 修改使用tinyspline对象绘制B样条曲线的方法
void DisplayScene::whenDrawSingleBSplineCurve(const tinyspline::BSpline &spline)
{
    // 创建B样条曲线组件
    auto splineComponent = std::make_shared<BSplineItem>(spline);
    addGraphicComponent(splineComponent);
    showAllGraphicComponents();
}

// 修改绘制多条B样条曲线的方法
void DisplayScene::whenDrawBSplineCurves(const std::vector<tinyspline::BSpline> &splines)
{
    if (splines.empty()) return;

    // 遍历所有样条曲线
    for (const auto& spline : splines) {
        auto splineComponent = std::make_shared<BSplineItem>(spline);
        addGraphicComponent(splineComponent);
    }
    showAllGraphicComponents();
}

void DisplayScene::whenDrawBSplineCurves(const std::vector<CurveSeg> &curves)
{
    if (curves.empty()) return;

    // 遍历所有曲线段
    for (const auto& curve : curves) {
        auto splineComponent = std::make_shared<BSplineItem>(curve.getSpline());
        addGraphicComponent(splineComponent);
    }
    showAllGraphicComponents();
}

// 绘制旋转矩形
void DisplayScene::whenDisplayRotateRects(const std::vector<cv::RotatedRect>& rotatedRects)
{
    if (rotatedRects.empty())
        return;

    // 创建旋转矩形组件
    auto rotatedRectComponent = std::make_shared<RotatedRectItem>(rotatedRects);
    addGraphicComponent(rotatedRectComponent);
    showAllGraphicComponents();
}












