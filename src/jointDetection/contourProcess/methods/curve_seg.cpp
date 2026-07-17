#define _USE_MATH_DEFINES
#include <cmath>

#include <plog/Log.h>

#include "curve_seg.h"
#include "contour_utils.h"
#include "src/utils/geometry_utils.h"
#include "contour_utils.h"

CurveSeg::CurveSeg() : m_minDomain(MIN_DOMAIN), m_maxDomain(MAX_DOMAIN) {}

void CurveSeg::initializeFromPoints(const std::vector<cv::Point2f>& points) { m_points = points; }

/**
  * @brief 使用样条曲线拟合轮廓点集
  * @details 使用tinyspline库对输入的点集进行三次B样条曲线拟合，生成平滑的轮廓曲线。
  *          拟合过程中会设置控制点并计算曲线的有效参数范围。
  *          如果点集数量少于4个，将无法进行样条拟合并输出警告信息。
  */
void CurveSeg::fitSplineCurve() {
    if (m_points.size() < 4) {
        PLOG_WARNING << "警告：点数太少 (" << m_points.size() << ")，至少需要4个点进行样条拟合";
        return;
    }

    // 对轮廓点进行半径滤波，去除离散噪声点
    std::vector<cv::Point2f> filteredPoints = ContourUtils::radiusOutlierRemoval(m_points, 5.0f, 4);
    m_points = filteredPoints;

    try {
        // 创建样条曲线：控制点数量，维度，阶数（3次样条）
        int degree = 3;  // 3次样条
        m_spline = tinyspline::BSpline(static_cast<int>(m_points.size()), 2, degree, tinyspline::BSpline::Type::Opened);

        // 设置控制点
        std::vector<tinyspline::real> ctrlp = m_spline.controlPoints();
        for (size_t i = 0; i < m_points.size(); ++i) {
            ctrlp[i * 2] = m_points[i].x;      // x坐标
            ctrlp[i * 2 + 1] = m_points[i].y;  // y坐标
        }
        m_spline.setControlPoints(ctrlp);

        // 保存控制点用于显示
        m_controlPoints.clear();
        for (size_t i = 0; i < m_points.size(); ++i) {
            m_controlPoints.push_back(m_points[i]);
        }

        // 获取实际的domain范围
        try {
            // 使用domain()方法获取元组
            auto domain_tuple = m_spline.domain();
            m_minDomain = static_cast<float>(domain_tuple.min());
            m_maxDomain = static_cast<float>(domain_tuple.max());
        } catch (const std::exception& e) {
            PLOG_WARNING << "获取domain范围失败，使用默认范围: " << e.what();
            // 使用默认的安全范围
            m_minDomain = MIN_DOMAIN;
            m_maxDomain = MAX_DOMAIN;
        }

        m_isFitted = true;

    } catch (const std::exception& e) {
        PLOG_WARNING << "样条曲线拟合失败: " << e.what();
        m_isFitted = false;
        // 拟合失败时重置domain范围
        m_minDomain = MIN_DOMAIN;
        m_maxDomain = MAX_DOMAIN;
    }
    // 拟合完成后计算曲线长度
    calculateCurveLength();
}

/**
  * @brief 获取拟合样条曲线的采样点集
  * @param numSamples 采样点数量，控制曲线的平滑度和精度
  * @return std::vector<cv::Point2f> 采样点集合，包含拟合曲线上均匀分布的坐标点
  * @details 该函数在样条曲线的参数域内均匀采样，生成指定数量的点来近似表示拟合后的曲线。
  *          如果曲线尚未拟合，将返回空集合并输出警告信息。
  */
std::vector<cv::Point2f> CurveSeg::getFittedPoints(int numSamples) const {
    std::vector<cv::Point2f> fittedPoints;

    if (!m_isFitted) {
        return fittedPoints;
    }

    for (int i = 0; i <= numSamples; ++i) {
        float u = m_minDomain + (m_maxDomain - m_minDomain) * (static_cast<float>(i) / numSamples);
        cv::Point2f point = evaluate(u);
        fittedPoints.push_back(point);
    }

    return fittedPoints;
}

std::vector<cv::Point2f> CurveSeg::getControlPoints() const { return m_controlPoints; }

/**
  * @brief 在样条曲线上评估指定参数值对应的坐标点
  * @param u 样条曲线的参数值，应在有效参数域[m_minDomain, m_maxDomain]范围内
  * @return cv::Point2f 对应参数值u处的二维坐标点
  * @details 该函数使用tinyspline库计算样条曲线在给定参数值处的坐标。
  *          如果曲线尚未拟合或评估过程中出现异常，将返回原点(0,0)并输出错误信息。
  */
cv::Point2f CurveSeg::evaluate(float u) const {
    if (!m_isFitted) {
        return cv::Point2f(0, 0);
    }

    try {
        std::vector<tinyspline::real> result = m_spline.eval(u).result();
        return cv::Point2f(static_cast<float>(result[0]), static_cast<float>(result[1]));
    } catch (const std::exception& e) {
        PLOG_ERROR << "评估样条曲线失败: " << e.what();
        return cv::Point2f(0, 0);
    }
}

/**
  * @brief 计算样条曲线在指定参数值处的切线向量
  * @param u 样条曲线的参数值，应在有效参数域[m_minDomain, m_maxDomain]范围内
  * @return cv::Vec4f 切线向量，格式为(tangent_x, tangent_y, point_x, point_y)
  * @details 该函数通过计算样条曲线的一阶导数来获取切线方向，并返回包含切线向量和对应点坐标的四维向量。
  *          如果曲线尚未拟合或计算过程中出现异常，将返回零向量并输出错误信息。
  */
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
        return cv::Vec4f(static_cast<float>(tangent[0]), static_cast<float>(tangent[1]), point.x, point.y);
    } catch (const std::exception& e) {
        PLOG_ERROR << "计算切线失败: " << e.what();
        return cv::Vec4f(0, 0, 0, 0);
    }
}

/**
  * @brief 在图像上绘制拟合的样条曲线
  * @param image 目标图像，曲线将绘制在此图像上
  * @param color 曲线颜色
  * @param thickness 曲线线宽，控制绘制线条的粗细
  * @details 该函数通过采样拟合曲线上的点，使用OpenCV的polylines函数绘制连续的样条曲线。
  *          采样点数量会根据控制点数量动态调整以确保曲线平滑度，同时会添加点标记增强可见性。
  *          如果曲线尚未拟合，将输出警告信息并直接返回。
  */
void CurveSeg::drawCurve(cv::Mat& image, const cv::Scalar& color, int thickness) const {
    if (!m_isFitted) {
        PLOG_WARNING << "警告：无法绘制未拟合的曲线";
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

/**
  * @brief 在图像上绘制样条曲线的控制点和控制多边形
  * @param image 目标图像，控制点和多边形将绘制在此图像上
  * @param pointColor 控制点颜色
  * @param polygonColor 控制多边形颜色
  */
void CurveSeg::drawControlPoints(cv::Mat& image, const cv::Scalar& pointColor,
                                 const cv::Scalar& polygonColor) const {
    if (m_controlPoints.empty()) return;

    // 绘制控制多边形
    std::vector<cv::Point> intControlPoints;
    for (const auto& pt : m_controlPoints) {
        intControlPoints.push_back(cv::Point(static_cast<int>(pt.x), static_cast<int>(pt.y)));
    }

    for (size_t i = 0; i < intControlPoints.size() - 1; ++i) {
        cv::line(image, intControlPoints[i], intControlPoints[i + 1], polygonColor, 1, cv::LINE_AA);
    }

    // 绘制控制点
    for (const auto& pt : intControlPoints) {
        cv::circle(image, pt, 4, pointColor, -1);          // 实心圆
        cv::circle(image, pt, 4, cv::Scalar(0, 0, 0), 1);  // 黑色边框
    }
}


/**
 * @brief 以参考点为原点建立坐标系，将两个点按逆时针方向排序
 * @param pointA 第一个点
 * @param pointB 第二个点
 * @param referencePoint 参考点
 * @return 排序后的点对，第一个点在前，第二个点在后（按逆时针方向）
 */
std::pair<cv::Point2f, cv::Point2f> CurveSeg::sortPointsCounterClockwise(const cv::Point2f& pointA, const cv::Point2f& pointB,
                                                                         const cv::Point2f& referencePoint) {
    std::pair<cv::Point2f, cv::Point2f> pointPair;
    if (GeometryUtils::isPointClockwiseTo(pointA, pointB, referencePoint)) {
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
std::pair<SplineEndpoints, SplineEndpoints> CurveSeg::sortEndpoints(const cv::Point2f& referencePoint) {
    // 检查轮廓是否已拟合
    if (!m_isFitted) {
        PLOG_WARNING << "轮廓尚未拟合";
        return std::make_pair(SplineEndpoints(cv::Point2f(0, 0), 0.0f), SplineEndpoints(cv::Point2f(0, 0), 0.0f));
    }

    // 获取轮廓的两个端点（首尾点）及其对应的u值
    cv::Point2f endPoint1 = evaluate(m_minDomain);
    cv::Point2f endPoint2 = evaluate(m_maxDomain);

    // 创建端点信息对象
    SplineEndpoints ep1(endPoint1, m_minDomain);
    SplineEndpoints ep2(endPoint2, m_maxDomain);

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
        PLOG_WARNING << "警告：样条曲线尚未拟合";
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
        PLOG_WARNING << "警告：采样点数量不足，无法拟合直线";
        return cv::Vec4f(0, 0, 0, 0);
    }

    cv::Vec4f lineParams = GeometryUtils::fitLine(samplePoints);

    return lineParams;
}

void CurveSeg::calculateCurveLength() {
    if (m_isFitted) {
        try {
            // 使用ChordLengths类计算弦长
            tinyspline::ChordLengths lengths = m_spline.chordLengths();

            // 获取总弧长
            tinyspline::real totalLength = lengths.arcLength();

            // 转换为float类型存储
            m_curveLength = static_cast<float>(totalLength);
        } catch (const std::exception& e) {
            std::cerr << "Error calculating curve length: " << e.what() << std::endl;
            m_curveLength = 0.0f;
        }
    } else {
        m_curveLength = 0.0f;
    }
}


























