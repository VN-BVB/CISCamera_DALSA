#include "contour_bounding_box.h"

ContourBoundingBox::ContourBoundingBox() {}

void ContourBoundingBox::initContourData(int id, const ContourData& contourData) {
    if (!contourData.isValid()) return;
    m_id = id;
    m_contourData = contourData;
    // 生成包围框信息
    m_boundingRect = generateBoundingBox(m_contourData);
    // 获取旋转矩形的四个角点
    cv::Point2f vertices[4];
    m_boundingRect.points(vertices);
    m_topLeft = vertices[0];
    m_topRight = vertices[1];
    m_bottomRight = vertices[2];
    m_bottomLeft = vertices[3];
    m_centerPoint = m_boundingRect.center;
    // 设置相背轮廓id
    m_oppositeId =  setOppositeTo(m_id);
}

cv::RotatedRect ContourBoundingBox::generateBoundingBox(const ContourData& contourData) {

    std::vector<cv::Point2f> contour = contourData.getSubpixelContour();
    if (contour.empty()) {
        return cv::RotatedRect(cv::Point2f(0, 0), cv::Size2f(0, 0), 0);
    }

    cv::RotatedRect rotatedRect = cv::minAreaRect(contour);
    return rotatedRect;
}

int ContourBoundingBox::setOppositeTo(int id) {
    if (id % 2 == 0) {
        return id + 1;
    } else {
        return id - 1;
    }
}
