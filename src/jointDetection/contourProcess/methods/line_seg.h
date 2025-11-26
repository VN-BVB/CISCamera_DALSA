#ifndef LINESEGMENTH
#define LINESEGMENTH

#include <opencv2/opencv.hpp>


/**
 * @brief 线段特征，存储一条线段的所有特征
 */
class LineSeg
{
public:
    LineSeg();
    void initializeFromPoints(const std::vector<cv::Point>& points);
    void initializeFromPoints(const std::vector<cv::Point2f>& points);
    void clear();
    bool isValid() const;
    std::string getSummary()const;
    cv::Vec4f getLineEquation() {return m_lineEquation;}

private:
    void calculateBasicFeatures();

private:
    // 线段上的点集
    std::vector<cv::Point> m_pixelPoints;           // 像素级点集
    std::vector<cv::Point2f> m_subpixelPoints;      // 亚像素级点集

    // 直线拟合相关特征
    cv::Vec4f m_lineEquation;                      // 直线方程 (vx, vy, x0, y0)

    // 几何特征
    cv::Point m_startPoint;                        // 线段起点
    cv::Point m_endPoint;                          // 线段终点
    double m_length;                               // 线段长度
    double m_angle;                                // 线段角度（弧度）
    cv::Point m_midpoint;                          // 线段中点
};

#endif // LINESEGMENTH
