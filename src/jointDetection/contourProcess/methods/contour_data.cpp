#include "contour_data.h"

ContourData::ContourData() : m_id(-1), m_openingDirection(0, 0) {}

void ContourData::setPixelContour(const std::vector<cv::Point>& contour) {
    m_pixelContour = contour;
}

void ContourData::setSubpixelContour(const std::vector<cv::Point2f>& contour) {
    m_subpixelContour = contour;
}

void ContourData::clear() {
    m_id = -1;
    m_pixelContour.clear();
    m_subpixelContour.clear();
    m_sortedSubpixelContour.clear();
    m_cornerPoints.clear();
    m_segmentedSubpixelContours.clear();
    m_counterClockwiseContours.clear();
    m_intersections.clear();
    m_openingDirection = cv::Point2f(0, 0);
    m_isCollision = false;
    m_centerLine = cv::Vec4f(0, 0, 0, 0);
}

bool ContourData::isValid() const {
    return !m_subpixelContour.empty();
}
