#include "contour_data.h"

ContourData::ContourData() : m_openingDirection(OpeningDirection::UNKNOWN) {}

void ContourData::setPixelContour(const std::vector<cv::Point>& contour) {
    m_pixelContour = contour;
}

void ContourData::setSubpixelContour(const std::vector<cv::Point2f>& contour) {
    m_subpixelContour = contour;
}

void ContourData::clear() {
    m_pixelContour.clear();
    m_subpixelContour.clear();
    m_sortedSubpixelContour.clear();
    m_cornerPoints.clear();
    m_segmentedSubpixelContours.clear();
    m_counterClockwiseContours.clear();
    m_openingDirection = OpeningDirection::UNKNOWN;
}

bool ContourData::isValid() const {
    return !m_subpixelContour.empty();
}
