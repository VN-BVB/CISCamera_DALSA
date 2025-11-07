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
#include "src/jointDetection/contourProcess/curve_seg.h"

class DisplayView;
class DisplayImageItem;
class DisplayScenePrivate;
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

public slots:
    // 显示图像
    bool whenDisplayImage(const QImage &image, bool bAutoFit = false);
    // 清除图像
    void whenClearImage();
    // 显示文本
    void whenAddDisplayText(const QString &text, const QPointF &pt=QPointF(0,0), const double &size=1,
                        const QColor &color=QColor(Qt::green), const bool &clear=false);
    // 清除文本
    void whenClearDisplayText();

public slots:
    // 绘制单条亚像素轮廓
    void whenDrawSingleSubpixelContour(const std::vector<cv::Point2f> &subpixelContour);
    // 绘制单条像素轮廓
    void whenDrawSinglePixelContour(const std::vector<cv::Point> &pixelContour);
    // 绘制多条亚像素轮廓
    void whenDrawSubpixelContours(const std::vector<std::vector<cv::Point2f>> &subpixelContours);
    // 绘制多条像素轮廓
    void whenDrawPixelContours(const std::vector<std::vector<cv::Point>> &pixelContours);
    // 清除轮廓
    void whenClearContours();
    // 绘制多条直线
    void whenDrawLines(const std::vector<cv::Vec4f> &lines, const double length = 1, const QColor &color = Qt::blue);
    // 绘制点集
    void whenDrawPoints(const std::vector<cv::Point2f> &Points, const QColor &color);
    // 绘制B样条曲线
    void whenDrawSingleBSplineCurve(const std::vector<cv::Point2f> &controlPoints);
    void whenDrawSingleBSplineCurve(const tinyspline::BSpline &spline);
    void whenDrawBSplineCurves(const std::vector<tinyspline::BSpline> &splines);
    void whenDrawBSplineCurves(const std::vector<CurveSeg> &curves);
    // 绘制旋转矩形
    void whenDisplayRotateRects(const std::vector<cv::RotatedRect>& RotatedRects);

    // @TODO:将轮廓显示全整理成图元类

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
protected:
    QList<QtCharts::QChartView*> m_chartViews; // 存储图表视图
    const QScopedPointer<DisplayScenePrivate> d_ptr;    // Qt的智能指针
private:
    Q_DECLARE_PRIVATE(DisplayScene) // PIMPL设计模式，将类的实现细节隐藏在一个单独的私有类中，隐藏实现细节，加快编译速度
};

#endif // DISPLAY_SCENE_H
