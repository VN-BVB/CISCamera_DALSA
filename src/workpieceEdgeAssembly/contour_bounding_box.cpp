#include "contour_bounding_box.h"

ContourBoundingBox::ContourBoundingBox() {}

void ContourBoundingBox::initContourData(int id, const ContourData& contourData) {
    if (!contourData.isValid()) return;
    m_id = id;
    m_contourData = contourData;
    // 生成包围框信息
    m_boundingRect = generateBoundingBox(m_contourData);
    m_topLeft = cv::Point2f(m_boundingRect.x, m_boundingRect.y);
    m_topRight = cv::Point2f(m_boundingRect.x + m_boundingRect.width, m_boundingRect.y);
    m_bottomRight = cv::Point2f(m_boundingRect.x + m_boundingRect.width, m_boundingRect.y + m_boundingRect.height);
    m_bottomLeft = cv::Point2f(m_boundingRect.x, m_boundingRect.y + m_boundingRect.height);
    // 设置相背轮廓id
    m_oppositeId =  setOppositeTo(m_id);
}

cv::Rect2f ContourBoundingBox::generateBoundingBox(const ContourData& contourData) {

    std::vector<cv::Point2f> contour = contourData.getSubpixelContour();
    if (contour.empty()) {
        return cv::Rect2f(0, 0, 0, 0);
    }

    cv::Rect2f boundingRect = cv::boundingRect(contour);
    return boundingRect;
}

int ContourBoundingBox::setOppositeTo(int id) {
    if (id % 2 == 0) {
        return id + 1;
    } else {
        return id - 1;
    }
}
