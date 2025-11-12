#ifndef JOINT_VIEW_H
#define JOINT_VIEW_H

#include <QGraphicsView>
#include <QThread>
#include <QWidget>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include "src/jointDetection/image_read_worker.h"
#include "src/jointDetection/image_process_worker.h"
#include "src/jointDetection/edgeDetection/canny_devernay.h"
#include "src/ui/utils/display/frm_display.h"
#include "src/jointDetection/contourProcess/methods/curve_seg.h"
#include "src/jointDetection/joint_seam.h"

namespace Ui {
class JointView;
}

class JointView : public QWidget {
    Q_OBJECT

public:
    explicit JointView(QWidget *parent = nullptr);
    ~JointView();

signals:
    void startImageRead(const QString &path);
    void startImageProcess(std::shared_ptr<cv::Mat> image);

private slots:
    void on_pb_open_clicked();
    void handleImageRead(std::shared_ptr<cv::Mat> image);
    // [[deprecated("这个槽函数接收的都是jointSeam的属性，请直接使用传递jointSeam的版本，以获得更多操作")]]
    void handleImageProcessed(std::shared_ptr<cv::Mat> processedImage, std::vector<std::vector<cv::Point2f>> subpixelContours,
                              std::vector<std::vector<cv::Point>> pixelContour, std::vector<cv::Vec4f> lines,
                              std::vector<CurveSeg> curves);
    void handleImageProcessed(std::shared_ptr<cv::Mat> processedImage, std::shared_ptr<JointSeam> jointSeam);
    void handleImageProcessedCannyDevenay(std::shared_ptr<cv::Mat> processedImage, std::vector<Point2fCurve> edgeCurves);
    void handleError(const QString &error);

    void updateDisplay();
    // Checkbox槽函数
    void on_ckb_pixelContoursSquare_toggled(bool checked);
    void on_ckb_pixelContoursLine_toggled(bool checked);
    void on_ckb_subpixelContours_toggled(bool checked);
    void on_ckb_fitlines_toggled(bool checked);
    void on_ckb_endPoints_toggled(bool checked);
    void on_ckb_fitCurves_toggled(bool checked);

private:
    Ui::JointView *ui;
    QThread readThread;
    QThread processThread;
    ImageReadWorker *readWorker;
    ImageProcessWorker *processWorker;

    // @TODO:使用日志记录每个步骤处理时间
    std::chrono::high_resolution_clock::time_point startTime;  // 图像处理开始时间

    // 存储当前显示的数据
    std::shared_ptr<cv::Mat> m_currentImage;
    std::vector<std::vector<cv::Point2f>> m_subpixelContours;
    std::vector<std::vector<cv::Point>> m_pixelContours;
    std::vector<cv::Vec4f> m_fitTangentLines;  // 曲线端点切线
    std::vector<CurveSeg> m_fitCurves;
    std::vector<cv::Point2f> m_endPointsByTangentLines;
    std::vector<cv::Vec4f> m_fitLines;  // 拟合线段
    std::vector<cv::Point2f> m_endPointsByFittedLines;

    // 显示控制标志
    bool m_showPixelContoursSquare = false;
    bool m_showPixelContoursLine = false;
    bool m_showSubpixelContours = false;
    bool m_showFitLines = false;
    bool m_showEndPoints = false;
    bool m_showFitCurves = false;
};

#endif  // JOINT_VIEW_H
