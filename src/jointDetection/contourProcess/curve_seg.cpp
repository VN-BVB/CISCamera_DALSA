#define _USE_MATH_DEFINES
#include "curve_seg.h"
#include <cmath>

CurveSeg::CurveSeg() : m_minDomain(MIN_DOMAIN), m_maxDomain(MAX_DOMAIN){}

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

        // 使用tinyspline的C++接口获取实际的domain范围
        try {
            // 方法1：使用domain()方法获取元组
            auto domain_tuple = m_spline.domain();
            m_minDomain = static_cast<float>(domain_tuple.min());
            m_maxDomain = static_cast<float>(domain_tuple.max());

            std::cout << "样条曲线拟合成功，使用 " << m_points.size() << " 个控制点" << std::endl;
            std::cout << "实际Domain范围: [" << m_minDomain << ", " << m_maxDomain << "]" << std::endl;

        } catch (const std::exception& e) {
            std::cout << "获取domain范围失败，使用默认范围: " << e.what() << std::endl;
            // 使用默认的安全范围
            m_minDomain = MIN_DOMAIN;
            m_maxDomain = MAX_DOMAIN;
        }


        m_isFitted = true;
        std::cout << "样条曲线拟合成功，使用 " << m_points.size() << " 个控制点" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "样条曲线拟合失败: " << e.what() << std::endl;
        m_isFitted = false;
        // 拟合失败时重置domain范围
        m_minDomain = MIN_DOMAIN;
        m_maxDomain = MAX_DOMAIN;
    }
}

std::vector<cv::Point2f> CurveSeg::getFittedPoints(int numSamples) const {
    std::vector<cv::Point2f> fittedPoints;

    if (!m_isFitted) {
        std::cout << "警告：样条曲线尚未拟合" << std::endl;
        return fittedPoints;
    }

    for (int i = 0; i <= numSamples; ++i) {
        float u = m_minDomain + (m_maxDomain - m_minDomain) * (static_cast<float>(i) / numSamples);
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
* @brief 判断点A是否在点B的顺时针方向（相对于参考点）
* @param pointA 第一个点
* @param pointB 第二个点
* @param referencePoint 参考点
* @return 如果点A在点B的顺时针方向返回true，否则返回false
*/
bool CurveSeg::isPointClockwiseTo(const cv::Point2f& pointA, const cv::Point2f& pointB, const cv::Point2f& referencePoint) const
{

    // 将参考点作为原点，计算相对坐标
    cv::Point2f relA = pointA - referencePoint;
    cv::Point2f relB = pointB - referencePoint;

    // 计算叉积 det = (ax * by - ay * bx)
    float det = relA.x * relB.y - relA.y * relB.x;

    // 如果叉积为正，b在a顺时针方向
    if (det > 0)
        return false;

    // 如果叉积为负，a在b顺时针方向
    if (det < 0)
        return true;

    // 叉积为0，共线情况，按距离排序（距离小的在顺时针方向）
    float d1 = relA.x * relA.x + relA.y * relA.y;
    float d2 = relB.x * relB.x + relB.y * relB.y;
    return d1 < d2;
}

/**
* @brief 以参考点为原点建立坐标系，将两个点按逆时针方向排序
* @param pointA 第一个点
* @param pointB 第二个点
* @param referencePoint 参考点
* @return 排序后的点对，第一个点在前，第二个点在后（按逆时针方向）
*/
std::pair<cv::Point2f, cv::Point2f> CurveSeg::sortPointsCounterClockwise(const cv::Point2f& pointA,
                                                                         const cv::Point2f& pointB,
                                                                         const cv::Point2f& referencePoint)
{
    std::pair<cv::Point2f, cv::Point2f> pointPair;
    if (isPointClockwiseTo(pointA, pointB, referencePoint)) {
        pointPair.first = pointA;
        pointPair.second = pointB;
    } else {
        pointPair.first = pointB;
        pointPair.second = pointA;
    }
    return pointPair;
}

/**
* @brief 自动获取轮廓端点并按参考点逆时针方向排序
* @param referencePoint 参考点
* @return 排序后的端点对，包含点的坐标和对应的u值
*/
std::pair<EndpointInfo, EndpointInfo> CurveSeg::sortEndpoints(const cv::Point2f& referencePoint)
{
    // 检查轮廓是否已拟合
    if (!m_isFitted) {
        std::cout << "轮廓尚未拟合" << std::endl;
        return std::make_pair(EndpointInfo(cv::Point2f(0, 0), 0.0f),
                              EndpointInfo(cv::Point2f(0, 0), 0.0f));
    }

    // 获取轮廓的两个端点（首尾点）及其对应的u值
    cv::Point2f endPoint1 = evaluate(m_minDomain);
    cv::Point2f endPoint2 = evaluate(m_maxDomain);

    // 创建端点信息对象
    EndpointInfo ep1(endPoint1, m_minDomain);
    EndpointInfo ep2(endPoint2, m_maxDomain);

    // 按逆时针方向排序
    std::pair<cv::Point2f, cv::Point2f> sortedPoints = sortPointsCounterClockwise(endPoint1, endPoint2, referencePoint);

    // 根据排序结果返回对应的端点信息
    if (sortedPoints.first == endPoint1) {
        return std::make_pair(ep1, ep2);
    } else {
        return std::make_pair(ep2, ep1);
    }
}






























