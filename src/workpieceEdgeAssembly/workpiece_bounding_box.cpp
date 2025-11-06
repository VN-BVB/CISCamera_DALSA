#include "workpiece_bounding_box.h"
#include <algorithm>
#include <cmath>

WorkpieceBoundingBox::WorkpieceBoundingBox() {}

// 辅助函数：判断点是否在旋转矩形内
bool isPointInRotatedRect(const cv::Point2f& point, const cv::RotatedRect& rotatedRect) {
    // 获取旋转矩形的四个角点
    cv::Point2f vertices[4];
    rotatedRect.points(vertices);

    // 计算点到四条边的向量积
    // 如果点都在四条边的同一侧（内部），则点在矩形内
    for (int i = 0; i < 4; i++) {
        cv::Point2f edge = vertices[(i + 1) % 4] - vertices[i];
        cv::Point2f pointToVertex = point - vertices[i];

        // 计算叉积（向量积）
        float crossProduct = edge.x * pointToVertex.y - edge.y * pointToVertex.x;

        // 如果叉积为负，说明点在边的右侧（外部）
        if (crossProduct < 0) {
            return false;
        }
    }

    return true;
}

cv::RotatedRect WorkpieceBoundingBox::generateOuterBoundingBox(const std::vector<std::shared_ptr<ContourBoundingBox>> cbbs)
{
    if (cbbs.empty()) return cv::RotatedRect(cv::Point2f(0,0), cv::Size2f(0,0), 0);

    std::vector<cv::Point2f> allPoints;

    // 收集所有轮廓边界框的角点
    for (const auto& boundingBox : cbbs) {
        // 获取当前边界框的旋转矩形
        cv::RotatedRect rotatedRect = boundingBox->getBoundingRect();

        // 获取旋转矩形的四个角点
        cv::Point2f vertices[4];
        rotatedRect.points(vertices);

        // 将角点添加到总点集中
        for (int i = 0; i < 4; ++i) {
            allPoints.push_back(vertices[i]);
        }
    }

    // 使用所有点生成最小外接旋转矩形
    cv::RotatedRect outerBoundingBox = cv::minAreaRect(allPoints);
    return outerBoundingBox;
}

void WorkpieceBoundingBox::updateOuterBoundingBox() {
    m_outerBoundingBox = generateOuterBoundingBox(m_cbbs);
}

void WorkpieceBoundingBox::addContourBoundingBox(std::shared_ptr<ContourBoundingBox> cbb) {
    m_cbbs.push_back(cbb);
    updateOuterBoundingBox();
}

bool WorkpieceBoundingBox::isLegal(std::shared_ptr<ContourBoundingBox> candidateCbb) const {
    if (!candidateCbb) {
        return false;
    }

    // 1. 判断候选轮廓边界框的中心是否在工件边界框中
    cv::RotatedRect candidateRect = candidateCbb->getBoundingRect();
    cv::Point2f candidateCenter = candidateRect.center;

    // 使用旋转矩形的精确判断方法
    if (isPointInRotatedRect(candidateCenter, m_outerBoundingBox)) {
        // 2. 检查工件中是否有与候选轮廓相背的轮廓
        int candidateOppositeId = candidateCbb->getOppositeId();
        for (const auto& existingCbb : m_cbbs) {
            if (existingCbb->getId() == candidateOppositeId) {
                return false;
            }
        }
        // 3. 候选框不是组成工件的轮廓，但又包含在工件矩形中，则不合法
        int candidateId = candidateCbb->getId();
        for (const auto& existingCbb : m_cbbs) {
            if (existingCbb->getId() == candidateId) {
                return true;
            }
        }
        return false;
    }

    return true;
}

std::vector<int> WorkpieceBoundingBox::getContourIds() const {
    std::vector<int> ids;
    for (const auto& cbb : m_cbbs) {
        ids.push_back(cbb->getId());
    }
    return ids;
}
