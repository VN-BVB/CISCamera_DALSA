#include "contour_feature_calculator.h"

OpeningDirection ContourFeatureCalculator::calculateOpeningDirection(const std::vector<cv::Point2f>& contour) {
    if (contour.empty()) return OpeningDirection::UNKNOWN;

    float sumX = 0.0f, sumY = 0.0f;
    for (const auto& point : contour) {
        sumX += point.x;
        sumY += point.y;
    }
    float avgX = sumX / contour.size();
    float avgY = sumY / contour.size();

    bool hasUp = false, hasDown = false, hasLeft = false, hasRight = false;

    for (const auto& point : contour) {
        if (std::abs(point.x - avgX) < 1 && point.y < avgY) hasUp = true;
        if (std::abs(point.x - avgX) < 1 && point.y > avgY) hasDown = true;
        if (std::abs(point.y - avgY) < 1 && point.x < avgX) hasLeft = true;
        if (std::abs(point.y - avgY) < 1 && point.x > avgX) hasRight = true;
    }

    if (!hasUp) return OpeningDirection::UP;
    if (!hasDown) return OpeningDirection::DOWN;
    if (!hasLeft) return OpeningDirection::LEFT;
    if (!hasRight) return OpeningDirection::RIGHT;

    return OpeningDirection::UNKNOWN;
}

std::vector<cv::Point2f> ContourFeatureCalculator::removeDuplicatePoints(const std::vector<cv::Point2f>& contour) {
    std::unordered_set<cv::Point2f, Point2fHash, Point2fEqual> seen;
    std::vector<cv::Point2f> uniquePoints;

    for (const auto& point : contour) {
        if (seen.insert(point).second) {
            uniquePoints.push_back(point);
        }
    }
    return uniquePoints;
}

cv::Point2f ContourFeatureCalculator::calculateStartPoint(OpeningDirection direction, const std::vector<cv::Point2f>& contour) {
    if (direction == OpeningDirection::UNKNOWN || contour.empty()) {
        return cv::Point2f(-1, -1);
    }

    float sumX = 0.0f, sumY = 0.0f;
    for (const auto& point : contour) {
        sumX += point.x;
        sumY += point.y;
    }
    float avgX = sumX / contour.size();
    float avgY = sumY / contour.size();

    cv::Point2f startPoint;
    switch (direction) {
    case OpeningDirection::UP: {
        std::vector<cv::Point2f> leftHalfPoints;
        for (const auto& point : contour) {
            if (point.x < avgX) leftHalfPoints.push_back(point);
        }
        if (leftHalfPoints.empty()) leftHalfPoints = contour;
        startPoint = *std::min_element(leftHalfPoints.begin(), leftHalfPoints.end(),
                                       [](const cv::Point2f& a, const cv::Point2f& b) {
                                           if (a.y == b.y) return a.x < b.x;
                                           return a.y < b.y;
                                       });
        break;
    }
    case OpeningDirection::RIGHT: {
        std::vector<cv::Point2f> topHalfPoints;
        for (const auto& point : contour) {
            if (point.y < avgY) topHalfPoints.push_back(point);
        }
        if (topHalfPoints.empty()) topHalfPoints = contour;
        startPoint = *std::min_element(topHalfPoints.begin(), topHalfPoints.end(),
                                       [](const cv::Point2f& a, const cv::Point2f& b) {
                                           if (a.x == b.x) return a.y < b.y;
                                           return a.x > b.x;
                                       });
        break;
    }
    case OpeningDirection::DOWN: {
        std::vector<cv::Point2f> rightHalfPoints;
        for (const auto& point : contour) {
            if (point.x > avgX) rightHalfPoints.push_back(point);
        }
        if (rightHalfPoints.empty()) rightHalfPoints = contour;
        startPoint = *std::min_element(rightHalfPoints.begin(), rightHalfPoints.end(),
                                       [](const cv::Point2f& a, const cv::Point2f& b) {
                                           if (a.y == b.y) return a.x > b.x;
                                           return a.y > b.y;
                                       });
        break;
    }
    case OpeningDirection::LEFT: {
        std::vector<cv::Point2f> bottomHalfPoints;
        for (const auto& point : contour) {
            if (point.y > avgY) bottomHalfPoints.push_back(point);
        }
        if (bottomHalfPoints.empty()) bottomHalfPoints = contour;
        startPoint = *std::min_element(bottomHalfPoints.begin(), bottomHalfPoints.end(),
                                       [](const cv::Point2f& a, const cv::Point2f& b) {
                                           if (a.x == b.x) return a.y > b.y;
                                           return a.x < b.x;
                                       });
        break;
    }
    default:
        return cv::Point2f(-1, -1);
    }
    return startPoint;
}

std::vector<cv::Point2f> ContourFeatureCalculator::sortContour(const std::vector<cv::Point2f>& contour, int firstPointIdx) {
    if (contour.empty()) return {};

    std::vector<cv::Point2f> sortedContour;
    std::vector<bool> visited(contour.size(), false);

    int currentIndex = firstPointIdx;
    sortedContour.push_back(contour[currentIndex]);
    visited[currentIndex] = true;

    while (sortedContour.size() < contour.size()) {
        double minDistance = std::numeric_limits<double>::max();
        int nearestIndex = -1;

        for (int i = 0; i < contour.size(); ++i) {
            if (!visited[i]) {
                double distance = cv::norm(contour[currentIndex] - contour[i]);
                if (distance < minDistance) {
                    minDistance = distance;
                    nearestIndex = i;
                }
            }
        }

        if (nearestIndex != -1) {
            sortedContour.push_back(contour[nearestIndex]);
            visited[nearestIndex] = true;
            currentIndex = nearestIndex;
        } else {
            break;
        }
    }
    return sortedContour;
}

std::vector<cv::Point2f> ContourFeatureCalculator::detectCornerPoints(const std::vector<cv::Point2f>& contour) {
    return detectCornerPointsByDouglasPeucker(contour);
}

std::vector<cv::Point2f> ContourFeatureCalculator::detectCornerPointsByDouglasPeucker(const std::vector<cv::Point2f>& contour, double epsilon) {
    if (contour.size() < 3) return {};

    std::vector<cv::Point> intContour;
    for (const auto& pt : contour) {
        intContour.push_back(cv::Point(static_cast<int>(pt.x), static_cast<int>(pt.y)));
    }

    std::vector<cv::Point> approx;
    cv::approxPolyDP(intContour, approx, epsilon, false);

    std::vector<cv::Point2f> cornerPoints;
    for (const auto& pt : approx) {
        cornerPoints.push_back(cv::Point2f(static_cast<float>(pt.x), static_cast<float>(pt.y)));
    }
    return cornerPoints;
}

std::vector<cv::Point2f> ContourFeatureCalculator::removePointsNearCorners(const std::vector<cv::Point2f>& contour,
                                                                           const std::vector<cv::Point2f>& cornerPoints,
                                                                           double radius) {
    if (contour.empty() || cornerPoints.empty()) return contour;

    std::vector<cv::Point2f> filteredContour;
    double radiusSquared = radius * radius;

    for (const auto& point : contour) {
        bool isNearCorner = false;
        for (const auto& corner : cornerPoints) {
            double dx = point.x - corner.x;
            double dy = point.y - corner.y;
            if (dx * dx + dy * dy <= radiusSquared) {
                isNearCorner = true;
                break;
            }
        }
        if (!isNearCorner) {
            filteredContour.push_back(point);
        }
    }
    return filteredContour;
}

cv::Rect ContourFeatureCalculator::calculateBoundingRect(const std::vector<cv::Point2f>& contour) {
    return cv::boundingRect(contour);
}

double ContourFeatureCalculator::calculateArea(const std::vector<cv::Point2f>& contour) {
    return cv::contourArea(contour);
}

double ContourFeatureCalculator::calculatePerimeter(const std::vector<cv::Point2f>& contour) {
    return cv::arcLength(contour, true);
}

double ContourFeatureCalculator::calculateCurvature(const cv::Point2f& prev, const cv::Point2f& curr, const cv::Point2f& next) {
    cv::Point2f v1 = curr - prev;
    cv::Point2f v2 = next - curr;
    double len1 = cv::norm(v1);
    double len2 = cv::norm(v2);

    if (len1 < 1e-10 || len2 < 1e-10) return 0.0;

    v1 /= len1;
    v2 /= len2;
    double cosAngle = v1.dot(v2);
    cosAngle = std::max(-1.0, std::min(1.0, cosAngle));
    return std::acos(cosAngle);
}
