#include "contour_fitter.h"
#include "src/utils/geometry_utils.h"
#include <plog/Log.h>

/**
 * @brief 将轮廓段拟合为直线段
 * @param segments 输入轮廓段集合，键为段索引，值为轮廓点集
 * @return std::map<int, LineSeg> 拟合后的直线段集合，键为直线段索引
 * @details 该函数遍历输入的轮廓段，对每个轮廓段进行直线拟合。对于索引为2的轮廓段，
 *          会将其分为前100个点和后100个点分别进行直线拟合，生成两条直线段；
 *          对于其他轮廓段，直接对整个轮廓进行直线拟合。
 */
std::map<int, LineSeg> ContourFitter::fitLinesToSegments(const std::map<int, std::vector<cv::Point2f>>& segments) {
    std::map<int, LineSeg> lineSegments;
    int key = 1;
    for (const auto& [index, contour] : segments) {
        LineSeg ls;
        if (index == 2) {
            // 只取contour的前100个元素
            std::vector<cv::Point2f> first100Points;
            first100Points.assign(contour.begin(), contour.begin() + 100);
            ls.initializeFromPoints(first100Points);
            lineSegments[key++] = ls;
            std::vector<cv::Point2f> end100Points;
            end100Points.assign(contour.end() - 100, contour.end());
            ls.initializeFromPoints(end100Points);
            lineSegments[key++] = ls;
        } else {
            ls.initializeFromPoints(contour);
            lineSegments[key++] = ls;
        }
    }
    return lineSegments;
}

/**
 * @brief 将轮廓段拟合为曲线段
 * @param segments 输入轮廓段集合，键为段索引，值为轮廓点集
 * @return std::map<int, CurveSeg> 拟合后的曲线段集合，键为曲线段索引
 */
std::map<int, CurveSeg> ContourFitter::fitCurvesToSegments(const std::map<int, std::vector<cv::Point2f>>& segments) {
    std::map<int, CurveSeg> curveSegments;
    for (const auto& [index, contour] : segments) {
        CurveSeg curve;
        curve.initializeFromPoints(contour);
        curve.fitSplineCurve();
        curveSegments[index] = curve;
    }
    return curveSegments;
}

/**
 * @brief 基于曲线段计算端点
 * @param curveSegments 输入曲线段集合，键为段索引，值为曲线段对象
 * @param centroid 质心坐标，用于端点排序的参考点
 * @param[out] endPoints 计算得到的端点集合
 * @param[out] lines 计算过程中使用的平均直线方程
 * @details 该函数使用逆时针排序的曲线段计算端点。首先获取三条关键曲线段，
 *          然后以质心为参考点对每条曲线段的端点进行逆时针排序，获取端点附近区域
 *          的平均直线，最后通过直线交点计算得到端点位置。
 */
void ContourFitter::calculateEndPoints(const std::map<int, CurveSeg>& curveSegments,
                                       const cv::Point2f centroid,
                                       std::vector<cv::Point2f>& endPoints,
                                       std::vector<cv::Vec4f>& lines)
{
    // 清空输出参数
    endPoints.clear();
    lines.clear();

    // 使用端点附近区域的平均直线代替单点切线

    // 优先使用逆时针排序的曲线段
    if (!curveSegments.empty()) {
        // 获取三条曲线段
        CurveSeg curve1 = curveSegments.at(1);  // 第一条轮廓（逆时针方向的前一个）
        CurveSeg curve2 = curveSegments.at(2);  // 第二条轮廓（点数最多的，中间轮廓）
        CurveSeg curve3 = curveSegments.at(3);  // 第三条轮廓（逆时针方向的后一个）

        // 使用质心作为参考点
        cv::Point2f referencePoint(centroid.x, centroid.y);

        // 对每条曲线段的端点进行逆时针排序
        std::pair<SplineEndpoints, SplineEndpoints> sortedEndpoints1 = curve1.sortEndpoints(referencePoint);
        std::pair<SplineEndpoints, SplineEndpoints> sortedEndpoints2 = curve2.sortEndpoints(referencePoint);
        std::pair<SplineEndpoints, SplineEndpoints> sortedEndpoints3 = curve3.sortEndpoints(referencePoint);

        // 键为1的曲线：取相对于参考点更逆时针的端点（即排序后的第一个端点）
        SplineEndpoints endpoint1_ccw = sortedEndpoints1.first;  // 更逆时针的端点
        // 键为2的曲线：取相对于参考点更顺时针的端点（即排序后的第二个端点）
        SplineEndpoints endpoint2_cw = sortedEndpoints2.second;  // 更顺时针的端点
        // 键为3的曲线：取相对于参考点更顺时针的端点（即排序后的第一个端点）
        SplineEndpoints endpoint3_ccw = sortedEndpoints3.second;  // 更顺时针的端点
        // 键为2的曲线：取相对于参考点更逆时针的端点（即排序后的第一个端点）
        SplineEndpoints endpoint2_ccw = sortedEndpoints2.first;  // 更逆时针的端点

        // 获取端点附近区域的平均直线
        // 根据曲线长度决定区域大小：长度小于50用99%区域，大于50用50%区域
        float regionSize1 = (curve1.getCurveLength() < 100.0f) ? 0.99f : 0.20f;
        float regionSize2_cw = (curve2.getCurveLength() < 100.0f) ? 0.99f : 0.1f;
        float regionSize3 = (curve3.getCurveLength() < 100.0f) ? 0.99f : 0.20f;
        float regionSize2_ccw = (curve2.getCurveLength() < 100.0f) ? 0.99f : 0.1f;
        cv::Vec4f avgLine1 = curve1.getAverageLineNearEndpoint(endpoint1_ccw.u, regionSize1, 150);
        cv::Vec4f avgLine2_cw = curve2.getAverageLineNearEndpoint(endpoint2_cw.u, regionSize2_cw, 150);
        cv::Vec4f avgLine3 = curve3.getAverageLineNearEndpoint(endpoint3_ccw.u, regionSize3, 150);
        cv::Vec4f avgLine2_ccw = curve2.getAverageLineNearEndpoint(endpoint2_ccw.u, regionSize2_ccw, 150);


        // 保存平均直线用于后续使用
        lines.push_back(avgLine1);
        lines.push_back(avgLine2_cw);
        lines.push_back(avgLine3);
        lines.push_back(avgLine2_ccw);

        // 计算交点：键为1的曲线更逆时针端点的平均直线与键为2的曲线更顺时针端点的平均直线求交点
        cv::Point2f cornerPoint1 = GeometryUtils::calculateLineIntersection(avgLine1, avgLine2_cw);
        // 计算交点：键为3的曲线更顺时针端点的平均直线与键为2的曲线更逆时针端点的平均直线求交点
        cv::Point2f cornerPoint2 = GeometryUtils::calculateLineIntersection(avgLine3, avgLine2_ccw);

        endPoints.push_back(cornerPoint1);
        endPoints.push_back(cornerPoint2);

        PLOG_INFO << "端点1坐标: (" << cornerPoint1.x << ", " << cornerPoint1.y << ")";
        PLOG_INFO << "端点2坐标: (" << cornerPoint2.x << ", " << cornerPoint2.y << ")";
    } else {
        PLOG_INFO << "警告：没有可用的曲线段数据，无法计算端点";
    }
}

/**
 * @brief 碰撞情况下基于缝隙中心线计算端点
 * @param segments 输入轮廓段集合，键为段索引，值为原始轮廓点集（不经样条拟合）
 * @param[out] endPoints 计算得到的端点集合
 * @param[out] lines 计算过程中使用的拟合直线
 * @param centerLine 缝隙中心线 (vx, vy, x0, y0)
 * @details 取第一条/第三条轮廓段的最小二乘拟合直线（cv::fitLine DIST_L2），
 *          分别与缝隙中心线求交，得到两个端点。碰撞时样条拟合可能失败，
 *          故直接对原始段点拟合，绕开样条。
 */
void ContourFitter::calculateEndPointsFromCenterLine(const std::map<int, std::vector<cv::Point2f>>& segments,
                                                     std::vector<cv::Point2f>& endPoints,
                                                     std::vector<cv::Vec4f>& lines,
                                                     const cv::Vec4f& centerLine)
{
    // 清空输出参数
    endPoints.clear();
    lines.clear();

    if (segments.empty()) {
        PLOG_INFO << "警告：没有可用的轮廓段数据，无法计算端点";
        return;
    }

    // 第一条/第三条轮廓段的最小二乘拟合直线 与 缝隙中心线求交
    const std::vector<cv::Point2f>& pts1 = segments.at(1);
    const std::vector<cv::Point2f>& pts3 = segments.at(3);

    cv::Vec4f line1, line3;
    cv::fitLine(pts1, line1, cv::DIST_L2, 0, 0.01, 0.01);
    cv::fitLine(pts3, line3, cv::DIST_L2, 0, 0.01, 0.01);

    cv::Point2f endPoint1 = GeometryUtils::calculateLineIntersection(line1, centerLine);
    cv::Point2f endPoint2 = GeometryUtils::calculateLineIntersection(line3, centerLine);

    lines.push_back(line1);
    lines.push_back(line3);
    endPoints.push_back(endPoint1);
    endPoints.push_back(endPoint2);

    PLOG_INFO << "[碰撞] 端点1坐标: (" << endPoint1.x << ", " << endPoint1.y << ")";
    PLOG_INFO << "[碰撞] 端点2坐标: (" << endPoint2.x << ", " << endPoint2.y << ")";
}

// ... existing code ...
/**
 * @brief 基于直线段计算端点
 * @param lineSegments 输入直线段集合，键为段索引，值为直线段对象
 * @return std::vector<cv::Point2f> 计算得到的端点集合
 */
std::vector<cv::Point2f> ContourFitter::calculateEndPoints(const std::map<int, LineSeg>& lineSegments) {
    std::vector<cv::Point2f> endPoints;
    if (!lineSegments.empty()) {
        LineSeg line1 = lineSegments.at(1);
        LineSeg line2 = lineSegments.at(2);
        LineSeg line3 = lineSegments.at(3);
        LineSeg line4 = lineSegments.at(4);
        cv::Point2f cornerPoint1 = GeometryUtils::calculateLineIntersection(line1.getLineEquation(), line2.getLineEquation());
        cv::Point2f cornerPoint2 = GeometryUtils::calculateLineIntersection(line3.getLineEquation(), line4.getLineEquation());
        endPoints.push_back(cornerPoint1);
        endPoints.push_back(cornerPoint2);
    }
    return endPoints;
}

