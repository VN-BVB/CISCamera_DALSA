#define _USE_MATH_DEFINES
#include "curve_seg.h"
#include "contour_utils.h"
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
    if (ContourUtils::isPointClockwiseTo(pointA, pointB, referencePoint)) {
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

/**
* @brief 获取端点附近区域的平均直线
* @param endpointU 端点对应的u值
* @param regionSize 采样区域大小（占整个参数域的比例，默认0.1表示10%）
* @param numSamples 采样点数量
* @return 拟合的平均直线方程 (vx, vy, x0, y0)
*/
cv::Vec4f CurveSeg::getAverageLineNearEndpoint(float endpointU, float regionSize, int numSamples) const {
    if (!m_isFitted) {
        std::cout << "警告：样条曲线尚未拟合" << std::endl;
        return cv::Vec4f(0, 0, 0, 0);
    }

    // 计算采样区域范围
    float uStart, uEnd;
    if (endpointU == m_minDomain) {
        // 如果是起始端点，采样区域为 [m_minDomain, m_minDomain + regionSize * (m_maxDomain - m_minDomain)]
        uStart = m_minDomain;
        uEnd = m_minDomain + regionSize * (m_maxDomain - m_minDomain);
    } else {
        // 如果是结束端点，采样区域为 [m_maxDomain - regionSize * (m_maxDomain - m_minDomain), m_maxDomain]
        uStart = m_maxDomain - regionSize * (m_maxDomain - m_minDomain);
        uEnd = m_maxDomain;
    }

    // 确保采样范围在有效域内
    uStart = std::max(uStart, m_minDomain);
    uEnd = std::min(uEnd, m_maxDomain);

    // 在采样区域内均匀采样点
    std::vector<cv::Point2f> samplePoints;
    for (int i = 0; i < numSamples; ++i) {
        float u = uStart + (uEnd - uStart) * (static_cast<float>(i) / (numSamples - 1));
        cv::Point2f point = evaluate(u);
        samplePoints.push_back(point);
    }

    // 使用最小二乘法拟合直线
    if (samplePoints.size() < 2) {
        std::cout << "警告：采样点数量不足，无法拟合直线" << std::endl;
        return cv::Vec4f(0, 0, 0, 0);
    }

    // 计算点的均值
    cv::Point2f meanPoint(0, 0);
    for (const auto& pt : samplePoints) {
        meanPoint.x += pt.x;
        meanPoint.y += pt.y;
    }
    meanPoint.x /= samplePoints.size();
    meanPoint.y /= samplePoints.size();

    // 计算协方差矩阵
    float xx = 0, xy = 0, yy = 0;
    for (const auto& pt : samplePoints) {
        float dx = pt.x - meanPoint.x;
        float dy = pt.y - meanPoint.y;
        xx += dx * dx;
        xy += dx * dy;
        yy += dy * dy;
    }

    // 计算特征向量（直线方向）
    float det = xx * yy - xy * xy;
    if (std::abs(det) < 1e-10) {
        // 如果协方差矩阵奇异，使用端点切线方向
        cv::Vec4f tangent = getTangent(endpointU);
        return tangent;
    }

    // 计算特征值和特征向量
    float trace = xx + yy;
    float lambda1 = (trace + std::sqrt(trace * trace - 4 * det)) / 2;
    float lambda2 = (trace - std::sqrt(trace * trace - 4 * det)) / 2;

    // 选择最大特征值对应的特征向量
    float vx, vy;
    if (lambda1 > lambda2) {
        vx = yy - lambda1;
        vy = -xy;
    } else {
        vx = yy - lambda2;
        vy = -xy;
    }

    // 归一化方向向量
    float norm = std::sqrt(vx * vx + vy * vy);
    if (norm > 0) {
        vx /= norm;
        vy /= norm;
    } else {
        // 如果方向向量为零，使用默认方向
        vx = 1;
        vy = 0;
    }

    // 确保方向向量指向正确的方向（从端点向外）
    cv::Point2f endpoint = evaluate(endpointU);
    cv::Point2f samplePoint = evaluate((endpointU == m_minDomain) ? uEnd : uStart);
    cv::Point2f direction = samplePoint - endpoint;

    // 检查方向是否一致
    float dotProduct = vx * direction.x + vy * direction.y;
    if (dotProduct < 0) {
        vx = -vx;
        vy = -vy;
    }

    return cv::Vec4f(vx, vy, endpoint.x, endpoint.y);
}




























