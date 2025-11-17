#include <QGraphicsItem>
#include <QGraphicsProxyWidget>

#include "display_scene.h"
#include "display_view.h"
#include "display_image_item.h"
#include "graphicItems/graphic_item_component.h"
#include "graphicItems/graphic_item_composite.h"
#include "graphicItems/line_item.h"


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


void DisplayScene::whenDrawPoints(const std::vector<cv::Point2f> &points,const QColor &color)
{
    if (points.empty())
        return;

    double size = 0.5;
    for (const auto& point : points) {
        // 创建圆形标记点
        QGraphicsEllipseItem *pointItem = new QGraphicsEllipseItem(point.x - size/2, point.y - size/2, size, size);
        pointItem->setBrush(QBrush(color)); // 粉色
        pointItem->setPen(QPen(Qt::NoPen)); // 黑色边框
        pointItem->setZValue(15); // 设置较高的Z值，确保显示在最上层

        this->addItem(pointItem);
    }
}

// B样条曲线绘制函数实现
void DisplayScene::whenDrawSingleBSplineCurve(const std::vector<cv::Point2f> &controlPoints)
{
    if (controlPoints.size() < 2) {
        return;
    }

    // 创建样条序列
    QtCharts::QSplineSeries *series = new QtCharts::QSplineSeries();
    series->setName("B样条曲线");

    // 添加控制点到序列
    for (const auto& point : controlPoints) {
        series->append(point.x, point.y);
    }

    // 创建图表
    QtCharts::QChart *chart = new QtCharts::QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setBackgroundVisible(false); // 透明背景

    // 设置曲线样式
    QPen pen(QColor(255, 0, 255)); // 洋红色
    pen.setWidth(2);
    series->setPen(pen);

    // 创建图表视图
    QtCharts::QChartView *chartView = new QtCharts::QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: transparent;"); // 透明背景

    // 将图表视图添加到场景
    QGraphicsProxyWidget *proxy = this->addWidget(chartView);
    proxy->setZValue(12); // 设置Z值

    // 存储图表视图以便后续清除
    m_chartViews.append(chartView);

    // 绘制控制点
    for (const auto& point : controlPoints) {
        QGraphicsEllipseItem *controlPointItem = new QGraphicsEllipseItem(
            point.x - 2, point.y - 2, 4, 4);
        controlPointItem->setBrush(QBrush(QColor(0, 255, 255))); // 青色
        controlPointItem->setPen(QPen(Qt::black));
        controlPointItem->setZValue(13); // 比曲线更高
        this->addItem(controlPointItem);
    }
}

// 使用tinyspline对象绘制B样条曲线
void DisplayScene::whenDrawSingleBSplineCurve(const tinyspline::BSpline &spline) {
    std::vector<tinyspline::real> controlPoints = spline.controlPoints();
    size_t numControlPoints = spline.numControlPoints();
    size_t dimension = spline.dimension();

    // 检查维度是否为2（二维曲线）
    if (dimension != 2) {
        return;
    }

    if (numControlPoints < 2) {
        return;
    }

    // 方法1：使用tinyspline采样功能获取曲线上的点
    std::vector<tinyspline::real> sampledPoints = spline.sample(1000); // 采样100个点

    // 创建QPainterPath来绘制曲线
    QPainterPath path;

    if (sampledPoints.size() >= 2) {
        // 移动到第一个点
        path.moveTo(sampledPoints[0], sampledPoints[1]);

        // 连接所有采样点
        for (size_t i = 2; i < sampledPoints.size(); i += 2) {
            if (i + 1 < sampledPoints.size()) {
                path.lineTo(sampledPoints[i], sampledPoints[i + 1]);
            }
        }
    } else {
        // 方法2：如果采样失败，直接连接控制点作为备用方案
        path.moveTo(controlPoints[0], controlPoints[1]);
        for (size_t i = 2; i < controlPoints.size(); i += 2) {
            if (i + 1 < controlPoints.size()) {
                path.lineTo(controlPoints[i], controlPoints[i + 1]);
            }
        }
    }

    // 创建路径图元
    QGraphicsPathItem *pathItem = new QGraphicsPathItem(path);
    QPen pen(QColor(255, 0, 255)); // 洋红色
    pen.setWidthF(0.1); // 设置线宽
    pen.setStyle(Qt::SolidLine);
    pathItem->setPen(pen);
    pathItem->setZValue(12); // 设置Z值

    this->addItem(pathItem);

    // // 绘制控制点
    // for (size_t i = 0; i < numControlPoints; ++i) {
    //     size_t baseIndex = i * dimension;
    //     if (baseIndex + 1 < controlPoints.size()) {
    //         tinyspline::real x = controlPoints[baseIndex];
    //         tinyspline::real y = controlPoints[baseIndex + 1];

    //         QGraphicsEllipseItem *controlPointItem = new QGraphicsEllipseItem(
    //             x - 2, y - 2, 4, 4);
    //         controlPointItem->setBrush(QBrush(QColor(0, 255, 255))); // 青色
    //         controlPointItem->setPen(QPen(Qt::black));
    //         controlPointItem->setZValue(13); // 比曲线更高
    //         this->addItem(controlPointItem);
    //     }
    // }
}

// 绘制多条B样条曲线
void DisplayScene::whenDrawBSplineCurves(const std::vector<tinyspline::BSpline> &splines)
{
    if (splines.empty()) return;

    // 遍历所有样条曲线
    for (const auto& spline : splines) {
        whenDrawSingleBSplineCurve(spline);
    }
}

void DisplayScene::whenDrawBSplineCurves(const std::vector<CurveSeg> &curves) {
    if (curves.empty()) return;
    std::vector<tinyspline::BSpline> bsplines;
    for (auto& curve : curves) {
        bsplines.push_back(curve.getSpline());
    }
    whenDrawBSplineCurves(bsplines);
    // whenDrawSingleBSplineCurve(curves[1].getSpline());
}

// 绘制旋转矩形
void DisplayScene::whenDisplayRotateRects(const std::vector<cv::RotatedRect>& rotatedRects)
{
    if (rotatedRects.empty())
        return;

    // 使用绿色绘制旋转矩形，与轮廓的红色区分开
    QPen pen(Qt::green);
    pen.setWidthF(2);  // 设置线宽
    pen.setStyle(Qt::SolidLine);  // 实线

    for (const auto& rotatedRect : rotatedRects)
    {
        // 获取旋转矩形的四个角点
        cv::Point2f vertices[4];
        rotatedRect.points(vertices);

        // 创建QPainterPath来绘制旋转矩形
        QPainterPath path;
        path.moveTo(vertices[0].x, vertices[0].y);

        // 连接四个角点形成闭合矩形
        for (int i = 1; i < 4; ++i) {
            path.lineTo(vertices[i].x, vertices[i].y);
        }
        path.closeSubpath();  // 闭合路径

        // 创建路径图元
        QGraphicsPathItem *rectItem = new QGraphicsPathItem(path);
        rectItem->setPen(pen);
        rectItem->setZValue(10);  // 设置Z值，确保显示在图像上方

        this->addItem(rectItem);

        // // 可选：绘制矩形的中心点
        // cv::Point2f center = rotatedRect.center;
        // QGraphicsEllipseItem *centerItem = new QGraphicsEllipseItem(
        //     center.x - 1, center.y - 1, 2, 2);
        // centerItem->setBrush(QBrush(Qt::red));  // 红色中心点
        // centerItem->setPen(QPen(Qt::NoPen));
        // centerItem->setZValue(11);  // 比矩形边框更高
        // this->addItem(centerItem);
    }
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

void DisplayScene::whenDrawLines(const std::vector<cv::Vec4f> &lines, const double length, const QColor &color)
{
    for (const auto& line : lines) {
        auto lineComponent = std::make_shared<LineItem>(line, length, color);
        addGraphicComponent(lineComponent);
        showAllGraphicComponents();
    }
}
















