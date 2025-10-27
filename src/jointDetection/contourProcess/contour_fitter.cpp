#include "contour_fitter.h"
#include <QDebug>

std::map<int, LineSeg> ContourFitter::fitLinesToSegments(const std::map<int, std::vector<cv::Point2f>>& segments) {
    std::map<int, LineSeg> lineSegments;
    for (const auto& [index, contour] : segments) {
        LineSeg ls;
        ls.initializeFromPoints(contour);
        lineSegments[index] = ls;
    }
    return lineSegments;
}

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

std::vector<cv::Point2f> ContourFitter::calculateEndPoints(const std::map<int, CurveSeg>& curveSegments,
                                                           const cv::Point2f centroid,
                                                           std::vector<cv::Vec4f>& lines) {
    std::vector<cv::Point2f> endPoints;
    // 先按逆时针标记线，再获得每条线端点的逆时针标记，最后根据这个确定选取轮廓的哪端切线进行计算

    // 优先使用逆时针排序的曲线段
    if (!curveSegments.empty()) {
        // 获取三条曲线段
        CurveSeg curve1 = curveSegments.at(1);  // 第一条轮廓（逆时针方向的前一个）
        CurveSeg curve2 = curveSegments.at(2);  // 第二条轮廓（点数最多的，中间轮廓）
        CurveSeg curve3 = curveSegments.at(3);  // 第三条轮廓（逆时针方向的后一个）

        // 使用质心作为参考点
        cv::Point2f referencePoint(centroid.x, centroid.y);

        // 对每条曲线段的端点进行逆时针排序
        std::pair<EndpointInfo, EndpointInfo> sortedEndpoints1 = curve1.sortEndpoints(referencePoint);
        std::pair<EndpointInfo, EndpointInfo> sortedEndpoints2 = curve2.sortEndpoints(referencePoint);
        std::pair<EndpointInfo, EndpointInfo> sortedEndpoints3 = curve3.sortEndpoints(referencePoint);

        // 键为1的曲线：取相对于参考点更逆时针的端点（即排序后的第一个端点）
        EndpointInfo endpoint1_ccw = sortedEndpoints1.first;  // 更逆时针的端点
        // 键为2的曲线：取相对于参考点更顺时针的端点（即排序后的第二个端点）
        EndpointInfo endpoint2_cw = sortedEndpoints2.second;  // 更顺时针的端点
        // 键为3的曲线：取相对于参考点更顺时针的端点（即排序后的第一个端点）
        EndpointInfo endpoint3_ccw = sortedEndpoints3.second;  // 更顺时针的端点
        // 键为2的曲线：取相对于参考点更逆时针的端点（即排序后的第一个端点）
        EndpointInfo endpoint2_ccw = sortedEndpoints2.first;  // 更逆时针的端点

        // 获取对应端点的切线
        cv::Vec4f tangent1 = curve1.getTangent(endpoint1_ccw.u);  // 键为1的曲线更逆时针端点的切线
        cv::Vec4f tangent2_cw = curve2.getTangent(endpoint2_cw.u);  // 键为2的曲线更顺时针端点的切线
        cv::Vec4f tangent3 = curve3.getTangent(endpoint3_ccw.u);  // 键为3的曲线更顺时针端点的切线
        cv::Vec4f tangent2_ccw = curve2.getTangent(endpoint2_ccw.u);  // 键为2的曲线更逆时针端点的切线

        // 保存切线用于后续使用
        lines.push_back(tangent1);
        lines.push_back(tangent2_cw);
        lines.push_back(tangent3);
        lines.push_back(tangent2_ccw);

        // 计算交点：键为1的曲线更逆时针端点的切线与键为2的曲线更顺时针端点的切线求交点
        cv::Point2f cornerPoint1 = calculateLineIntersection(tangent1, tangent2_cw);
        // 计算交点：键为3的曲线更顺时针端点的切线与键为2的曲线更逆时针端点的切线求交点
        cv::Point2f cornerPoint2 = calculateLineIntersection(tangent3, tangent2_ccw);

        endPoints.push_back(cornerPoint1);
        endPoints.push_back(cornerPoint2);

        qDebug() << "使用逆时针排序曲线段计算端点完成";
        qDebug() << "端点1坐标: (" << cornerPoint1.x << ", " << cornerPoint1.y << ")";
        qDebug() << "端点2坐标: (" << cornerPoint2.x << ", " << cornerPoint2.y << ")";

    } else {
        qDebug() << "警告：没有可用的曲线段数据，无法计算端点";
    }
    return endPoints;
}

cv::Point2f ContourFitter::calculateLineIntersection(const cv::Vec4f& line1, const cv::Vec4f& line2) {
    float vx1 = line1[0], vy1 = line1[1], x01 = line1[2], y01 = line1[3];
    float vx2 = line2[0], vy2 = line2[1], x02 = line2[2], y02 = line2[3];

    // 计算交点
    float denominator = vx1 * vy2 - vy1 * vx2;
    if (std::abs(denominator) < 1e-10) {
        return cv::Point2f(-1, -1); // 平行线
    }

    float t = ((x02 - x01) * vy2 - (y02 - y01) * vx2) / denominator;
    float x = x01 + t * vx1;
    float y = y01 + t * vy1;

    return cv::Point2f(x, y);
}

