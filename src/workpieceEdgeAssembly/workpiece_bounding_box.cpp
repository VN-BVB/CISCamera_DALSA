#include "workpiece_bounding_box.h"
#include <algorithm>

WorkpieceBoundingBox::WorkpieceBoundingBox() {}

cv::Rect2f WorkpieceBoundingBox::generateOuterBoundingBox(const std::vector<std::shared_ptr<ContourBoundingBox>> cbbs)
{
    if (cbbs.empty()) return cv::Rect2f(0,0,0,0);

    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();
    // 遍历所有轮廓边界框，计算最小外接矩形
    for (const auto& boundingBox : cbbs) {
        // 获取当前边界框的矩形
        cv::Rect2f rect = boundingBox->getBoundingRect();

        // 更新最小和最大坐标
        minX = std::min(minX, rect.x);
        minY = std::min(minY, rect.y);
        maxX = std::max(maxX, rect.x + rect.width);
        maxY = std::max(maxY, rect.y + rect.height);
    }

    // 计算外接矩形的宽度和高度
    float width = maxX - minX;
    float height = maxY - minY;

    // 创建并返回最小外接矩形
    cv::Rect2f outerBoundingBox = cv::Rect2f(minX, minY, width, height);
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
    cv::Rect2f candidateRect = candidateCbb->getBoundingRect();
    cv::Point2f candidateCenter(
        candidateRect.x + candidateRect.width / 2.0f,
        candidateRect.y + candidateRect.height / 2.0f
        );

    if (m_outerBoundingBox.contains(candidateCenter)) {
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

























