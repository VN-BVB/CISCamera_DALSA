#ifndef CANNYDEVERNAY_H
#define CANNYDEVERNAY_H

#include <vector>
#include <opencv2/opencv.hpp>


struct EdgePoint {
    double x;
    double y;
};

struct EdgeCurve {
    std::vector<EdgePoint> points;
    bool isClosed;
};


// 将curve中的点转换为cv::Point2f类型
struct Point2fCurve
{
    std::vector<cv::Point2f> points;
    bool isClosed;
};

class CannyDevernay
{
public:
    CannyDevernay();
    ~CannyDevernay();

    // 主要检测接口
    std::vector<Point2fCurve> detectEdges(const cv::Mat& inputImage,
                                       double sigma = 1.5,
                                       double highThreshold = 50,
                                       double lowThreshold = 20);

    // 参数设置
    void setSigma(double sigma);
    void setHighThreshold(double threshold);
    void setLowThreshold(double threshold);

    // 结果获取
    const std::vector<EdgeCurve>& getEdgeCurves() const;

private:
    void error(const char* msg);
    void* xmalloc(size_t size);
    int greater_double(double a, double b);
    double dist(double x1, double y1, double x2, double y2);
    void gaussian_kernel(double* kernel, int n, double sigma, double mean);
    void gaussian_filter(uchar* image, uchar* out, int X, int Y, double sigma);
    double chain(int from, int to, double* Ex, double* Ey, double* Gx, double* Gy, int X, int Y);
    void compute_gradient(double* Gx, double* Gy, double* modG, uchar* image, int X, int Y);
    void compute_edge_points(double* Ex, double* Ey, double* modG, double* Gx, double* Gy, int X, int Y);
    void chain_edge_points(int* next, int* prev, double* Ex, double* Ey, double* Gx, double* Gy, int X, int Y);
    void thresholds_with_hysteresis(int* next, int* prev, double* modG, int X, int Y, double th_h, double th_l);
    void list_chained_edge_points(double** x, double** y, int* N, int** curve_limits, int* M,
                                  int* next, int* prev, double* Ex, double* Ey, int X, int Y);
    void devernay(double** x, double** y, int* N, int** curve_limits, int* M,
                  uchar* image, uchar* gauss, int X, int Y, double sigma, double th_h, double th_l);
    std::vector<Point2fCurve> convertToPoint2fCurves(const std::vector<EdgeCurve> &edgeCurves);
        // 成员变量
        double m_sigma;
    double m_highThreshold;
    double m_lowThreshold;
    std::vector<EdgeCurve> m_edgeCurves;
};

#endif // CANNYDEVERNAY_H
