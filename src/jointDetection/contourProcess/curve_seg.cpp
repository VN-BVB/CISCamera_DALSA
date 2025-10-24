#define _USE_MATH_DEFINES
#include "curve_seg.h"
#include <cmath>

CurveSeg::CurveSeg() {}

void CurveSeg::initializeFromPoints(const std::vector<cv::Point2f>& points) {
    m_points = points;

}

void CurveSeg::fitSplineCurve() {
    if (m_points.size() < 4) {
        std::cout << "警告：点数太少 (" << m_points.size() << ")，至少需要4个点进行样条拟合" << std::endl;
        return;
    }

    try {
        // 创建样条曲线：控制点数量，维度，阶数（3次样条）
        int degree = 3; // 3次样条
        m_spline = tinyspline::BSpline(static_cast<int>(m_points.size()), 2, degree, tinyspline::BSpline::Type::Opened);

        // 设置控制点
        std::vector<tinyspline::real> ctrlp = m_spline.controlPoints();
        for (size_t i = 0; i < m_points.size(); ++i) {
            ctrlp[i * 2] = m_points[i].x;     // x坐标
            ctrlp[i * 2 + 1] = m_points[i].y; // y坐标
        }
        m_spline.setControlPoints(ctrlp);

        // 保存控制点用于显示
        m_controlPoints.clear();
        for (size_t i = 0; i < m_points.size(); ++i) {
            m_controlPoints.push_back(m_points[i]);
        }

        m_isFitted = true;
        std::cout << "样条曲线拟合成功，使用 " << m_points.size() << " 个控制点" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "样条曲线拟合失败: " << e.what() << std::endl;
        m_isFitted = false;
    }
}

std::vector<cv::Point2f> CurveSeg::getFittedPoints(int numSamples) const {
    std::vector<cv::Point2f> fittedPoints;

    if (!m_isFitted) {
        std::cout << "警告：样条曲线尚未拟合" << std::endl;
        return fittedPoints;
    }

    for (int i = 0; i <= numSamples; ++i) {
        float u = static_cast<float>(i) / numSamples;
        cv::Point2f point = evaluate(u);
        fittedPoints.push_back(point);
    }

    return fittedPoints;
}

std::vector<cv::Point2f> CurveSeg::getControlPoints() const {
    return m_controlPoints;
}

cv::Point2f CurveSeg::evaluate(float u) const {
    if (!m_isFitted) {
        return cv::Point2f(0, 0);
    }

    try {
        std::vector<tinyspline::real> result = m_spline.eval(u).result();
        return cv::Point2f(static_cast<float>(result[0]),
                           static_cast<float>(result[1]));
    } catch (const std::exception& e) {
        std::cout << "评估样条曲线失败: " << e.what() << std::endl;
        return cv::Point2f(0, 0);
    }
}

cv::Vec4f CurveSeg::getTangent(float u) const {
    if (!m_isFitted) {
        return cv::Vec4f(0, 0, 0, 0);
    }

    try {
        // 获取切点坐标
        cv::Point2f point = evaluate(u);
        tinyspline::BSpline derivative = m_spline.derive();
        std::vector<tinyspline::real> tangent = derivative.eval(u).result();
        // return cv::Vec4f(point.x, point.y, tangent[0], tangent[1]);
        return cv::Vec4f(static_cast<float>(tangent[0]),
                         static_cast<float>(tangent[1]),
                         point.x, point.y);
    } catch (const std::exception& e) {
        std::cout << "计算切线失败: " << e.what() << std::endl;
        return cv::Vec4f(0, 0, 0, 0);
    }
}

void CurveSeg::drawCurve(cv::Mat& image, const cv::Scalar& color, int thickness) const {
    if (!m_isFitted) {
        std::cout << "警告：无法绘制未拟合的曲线" << std::endl;
        return;
    }

    // 增加采样点数量，根据控制点数量动态调整
    int numSamples = std::max(200, static_cast<int>(m_controlPoints.size()) * 50);
    std::vector<cv::Point2f> fittedPoints = getFittedPoints(numSamples);
    if (fittedPoints.size() < 2) return;

    // 将浮点坐标转换为整数坐标用于绘制
    std::vector<cv::Point> intPoints;
    for (const auto& pt : fittedPoints) {
        intPoints.push_back(cv::Point(static_cast<int>(pt.x), static_cast<int>(pt.y)));
    }

    // 绘制样条曲线 - 使用polylines绘制连续曲线，避免线段不连续
    cv::polylines(image, intPoints, false, color, thickness, cv::LINE_AA);

    // 同时绘制点标记，确保曲线可见
    for (const auto& pt : intPoints) {
        if (pt.x >= 0 && pt.x < image.cols && pt.y >= 0 && pt.y < image.rows) {
            cv::circle(image, pt, 1, color, -1);
        }
    }
}

void CurveSeg::drawControlPoints(cv::Mat& image, const cv::Scalar& pointColor,
                                 const cv::Scalar& polygonColor) const {
    if (m_controlPoints.empty()) return;

    // 绘制控制多边形
    std::vector<cv::Point> intControlPoints;
    for (const auto& pt : m_controlPoints) {
        intControlPoints.push_back(cv::Point(static_cast<int>(pt.x), static_cast<int>(pt.y)));
    }

    for (size_t i = 0; i < intControlPoints.size() - 1; ++i) {
        cv::line(image, intControlPoints[i], intControlPoints[i+1],
                 polygonColor, 1, cv::LINE_AA);
    }

    // 绘制控制点
    for (const auto& pt : intControlPoints) {
        cv::circle(image, pt, 4, pointColor, -1); // 实心圆
        cv::circle(image, pt, 4, cv::Scalar(0, 0, 0), 1); // 黑色边框
    }
}


/**
* @brief 以质心为原点建立坐标系，将两个点按逆时针方向排序
* @param point1 第一个点
* @param point2 第二个点
* @param referencePoint 参考点
* @return 排序后的点对，第一个点在前，第二个点在后（按逆时针方向）
*/
std::pair<cv::Point2f, cv::Point2f> CurveSeg::sortPointsCounterClockwise(const cv::Point2f& point1,
                                                                         const cv::Point2f& point2,
                                                                         const cv::Point2f& referencePoint)
{
    // 将质心转换为Point2f类型
    cv::Point2f centroid(referencePoint.x, referencePoint.y);

    // 计算两个点相对于质心的向量
    cv::Point2f vec1 = point1 - centroid;
    cv::Point2f vec2 = point2 - centroid;

    // 计算两个向量的角度（相对于x轴正方向）
    double angle1 = std::atan2(vec1.y, vec1.x);
    double angle2 = std::atan2(vec2.y, vec2.x);

    // 确保角度在[0, 2π)范围内
    if (angle1 < 0) angle1 += 2 * M_PI;
    if (angle2 < 0) angle2 += 2 * M_PI;

    // 按角度大小排序（逆时针方向）
    if (angle1 <= angle2) {
        return std::make_pair(point1, point2);
    } else {
        return std::make_pair(point2, point1);
    }
}

/**
* @brief 自动获取轮廓端点并按参考点逆时针方向排序
* @param referencePoint 参考点
* @return 排序后的端点对，第一个点在前，第二个点在后（按逆时针方向）
*/
std::pair<cv::Point2f, cv::Point2f> CurveSeg::sortEndpoints(const cv::Point2f& referencePoint)
{
    // 检查轮廓点是否为空
    if (m_isFitted) {
        std::cout << "轮廓尚未拟合" << std::endl;
        return std::make_pair(cv::Point2f(0, 0), cv::Point2f(0, 0));
    }

    // 获取轮廓的两个端点（首尾点）
    cv::Point2f endPoint1 = evaluate(MIN_DOMAIN);
    cv::Point2f endPoint2 = evaluate(MAX_DOMAIN);

    return sortPointsCounterClockwise(endPoint1, endPoint2, referencePoint);
}





























