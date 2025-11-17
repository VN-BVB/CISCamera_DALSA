#ifndef DISPLAY_SCENE_H
#define DISPLAY_SCENE_H

#include <QGraphicsScene>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QChart>

#include "tinysplinecxx.h"
#include "src/jointDetection/contourProcess/methods/curve_seg.h"
#include "graphicItems/graphic_item_component.h"
#include "graphicItems/contour_item.h"

class DisplayView;
class DisplayImageItem;
class DisplayScenePrivate;
class GraphicItemComposite;
class DisplayScene : public QGraphicsScene
{
    Q_OBJECT

    friend class DisplayView;   // 双向友元虽然破坏了封装性，但在View和Scene这种紧密耦合的框架组件中是合理的，可以简化实现，避免调用公共接口产生的开销
public:
    explicit DisplayScene(DisplayView *parentView = nullptr);
    ~DisplayScene();
public:
    // 获取视图
    DisplayView* getView() const {return m_parentView;}
    // 获取图像显示图元
    DisplayImageItem* getDisplayImageItem() const {return m_displayImageItem;}
    // 获取显示的图像
    QPixmap getDisplayImage();
    // 获取显示的图像的尺寸
    QSize getDisplayImageSize() const;

    // 图形组件管理方法
    void addGraphicComponent(std::shared_ptr<GraphicsItemComponent> component);
    void removeGraphicComponent(std::shared_ptr<GraphicsItemComponent> component);
    void showAllGraphicComponents();
    void clearAllGraphicComponents();
    // 获取图形组件组合
    std::shared_ptr<GraphicItemComposite> getGraphicItemComposite() const { return m_graphicItemComposite; }


    // =====================================图像显示槽函数=====================================
public slots:
    bool whenDisplayImage(const QImage &image, bool bAutoFit = false);
    void whenClearImage();

    // =====================================文本显示槽函数=====================================
public slots:
    void whenAddDisplayText(const QString &text, const QPointF &pt=QPointF(0,0), const double &size=1,
                        const QColor &color=QColor(Qt::green), const bool &clear=false);
    void whenClearDisplayText();

    // =====================================图形显示槽函数=====================================
public slots:
    // 绘制B样条曲线
    void whenDrawSingleBSplineCurve(const std::vector<cv::Point2f> &controlPoints);
    void whenDrawSingleBSplineCurve(const tinyspline::BSpline &spline);
    void whenDrawBSplineCurves(const std::vector<tinyspline::BSpline> &splines);
    void whenDrawBSplineCurves(const std::vector<CurveSeg> &curves);
    // 绘制旋转矩形
    void whenDisplayRotateRects(const std::vector<cv::RotatedRect>& RotatedRects);

    // ==========================图形组件系统绘制===================
    // @TODO:这里好像有点不好，每回都在调用槽函数时指定绘制属性，应该将这些属性抽象到component基类中实现
    void whenDrawContours(const std::vector<std::vector<cv::Point2f>> &contours,
                          const QColor& color = Qt::transparent,
                          double lineWidth = -1.0,
                          Qt::PenStyle lineStyle = Qt::SolidLine,
                          double zValue = 10.0);
    void whenDrawPoints(const std::vector<cv::Point2f> &points,
                        const QColor& color = Qt::red,
                        double size = 0.5,
                        double zValue = 15.0);
    void whenDrawLines(const std::vector<cv::Vec4f> &lines, const double length = 1, const QColor &color = Qt::blue);

protected:
    // 设置显示图像图元
    void setDisplayImageItem(DisplayImageItem* imageItem);
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
signals:
    // 更新显示图像
    void sendUpdateDisplayImage(const QImage &image);
    // 清除图像
    void sendClearDisplayImage();
    // 鼠标点击信号
    void sendMousePress(QGraphicsSceneMouseEvent* event);
    void sendMouseRelease(QGraphicsSceneMouseEvent* event);
protected:
    // 父视图
    DisplayView *m_parentView = nullptr;
    // 图像显示图元
    DisplayImageItem *m_displayImageItem = nullptr;
    // 图形图元组合管理器
    std::shared_ptr<GraphicItemComposite> m_graphicItemComposite;
protected:
    QList<QtCharts::QChartView*> m_chartViews; // 存储图表视图
    const QScopedPointer<DisplayScenePrivate> d_ptr;    // Qt的智能指针
private:
    Q_DECLARE_PRIVATE(DisplayScene) // PIMPL设计模式，将类的实现细节隐藏在一个单独的私有类中，隐藏实现细节，加快编译速度
};

#endif // DISPLAY_SCENE_H
