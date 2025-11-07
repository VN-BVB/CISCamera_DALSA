#include "edge_assembly.h"
#include <iostream>
#include <QDebug>

EdgeAssembly::EdgeAssembly(const std::vector<std::shared_ptr<ContourBoundingBox>> cbbs)
{
    m_cbbs = cbbs;
}


void EdgeAssembly::makeTestExample(JointSeam jointSeam) {


}

// 辅助函数：计算两个向量的叉积
float crossProduct(const cv::Point2f& a, const cv::Point2f& b) {
    return a.x * b.y - a.y * b.x;
}

// 辅助函数：判断点是否在线段上
bool isPointOnSegment(const cv::Point2f& p, const cv::Point2f& a, const cv::Point2f& b) {
    // 检查点p是否在线段ab的边界框内
    if (p.x < std::min(a.x, b.x) || p.x > std::max(a.x, b.x) ||
        p.y < std::min(a.y, b.y) || p.y > std::max(a.y, b.y)) {
        return false;
    }

    // 检查点p是否在直线ab上（叉积为0表示共线）
    float cross = crossProduct(b - a, p - a);
    return std::abs(cross) < 1e-6;
}

// 辅助函数：判断两条线段是否相交
bool doSegmentsIntersect(const cv::Point2f& p1, const cv::Point2f& p2,
                         const cv::Point2f& q1, const cv::Point2f& q2) {
    // 使用快速排斥实验和跨立实验判断线段相交

    // 快速排斥实验：检查两个线段的边界框是否相交
    if (std::max(p1.x, p2.x) < std::min(q1.x, q2.x) ||
        std::max(q1.x, q2.x) < std::min(p1.x, p2.x) ||
        std::max(p1.y, p2.y) < std::min(q1.y, q2.y) ||
        std::max(q1.y, q2.y) < std::min(p1.y, p2.y)) {
        return false;
    }

    // 跨立实验：检查点q1和q2是否在线段p1p2的两侧
    float cross1 = crossProduct(p2 - p1, q1 - p1);
    float cross2 = crossProduct(p2 - p1, q2 - p1);

    // 检查点p1和p2是否在线段q1q2的两侧
    float cross3 = crossProduct(q2 - q1, p1 - q1);
    float cross4 = crossProduct(q2 - q1, p2 - q1);

    // 如果两个叉积的乘积小于等于0，说明线段相交
    if (cross1 * cross2 <= 0 && cross3 * cross4 <= 0) {
        return true;
    }

    // 检查端点重合的情况
    if (isPointOnSegment(p1, q1, q2) || isPointOnSegment(p2, q1, q2) ||
        isPointOnSegment(q1, p1, p2) || isPointOnSegment(q2, p1, p2)) {
        return true;
    }

    return false;
}


void EdgeAssembly::generateWorkpiece() {
    // 获取所有未配对的轮廓
    std::vector<std::shared_ptr<ContourBoundingBox>> unpairedContours;
    for (auto& c : m_cbbs) {
        if (!c->getIsPaired()) {
            unpairedContours.push_back(c);
        }
    }


    // 生成2轮廓组合
    for (int i = 0; i < unpairedContours.size(); ++i) {
        // 如果当前轮廓已配对，跳过
        if (unpairedContours[i]->getIsPaired()) {
            continue;
        }
        for (int j = i + 1; j < unpairedContours.size(); ++j) {
            // 如果目标轮廓已配对，跳过
            if (unpairedContours[j]->getIsPaired()) {
                continue;
            }

            WorkpieceBoundingBox wp;
            wp.addContourBoundingBox(unpairedContours[i]);
            wp.addContourBoundingBox(unpairedContours[j]);

            // 检查组合是否合法：遍历所有未组合轮廓，判断是否能合法加入
            bool isLegalCombination = true;
            for (const auto& candidate : m_cbbs) {
                if (!wp.isLegal(candidate)) {
                    isLegalCombination = false;
                    break;
                }
            }

            if (isLegalCombination) {
                m_possibleWorkpieces.push_back(wp);
            }
        }
    }

    // 生成3轮廓组合
    for (int i = 0; i < unpairedContours.size(); ++i) {
        // 如果当前轮廓已配对，跳过
        if (unpairedContours[i]->getIsPaired()) {
            continue;
        }
        for (int j = i + 1; j < unpairedContours.size(); ++j) {
            // 如果目标轮廓已配对，跳过
            if (unpairedContours[j]->getIsPaired()) {
                continue;
            }
            for (int k = j + 1; k < unpairedContours.size(); ++k) {
                // 如果目标轮廓已配对，跳过
                if (unpairedContours[k]->getIsPaired()) {
                    continue;
                }

                WorkpieceBoundingBox wp;
                wp.addContourBoundingBox(unpairedContours[i]);
                wp.addContourBoundingBox(unpairedContours[j]);
                wp.addContourBoundingBox(unpairedContours[k]);

                // 检查组合是否合法：遍历所有未组合轮廓，判断是否能合法加入
                bool isLegalCombination = true;
                for (const auto& candidate : unpairedContours) {
                    if (!wp.isLegal(candidate)) {
                        isLegalCombination = false;
                        break;
                    }
                }

                if (isLegalCombination) {
                    m_possibleWorkpieces.push_back(wp);
                    // 标记这三个轮廓为已配对
                    // unpairedContours[i]->setIsPaired(true);
                    // unpairedContours[j]->setIsPaired(true);
                    // unpairedContours[k]->setIsPaired(true);
                    // break; // 跳出内层循环
                }
            }
        }
    }

    qDebug() << "可能的工件组合";
    int i = 0;
    for (auto& workPiece : m_possibleWorkpieces) {
        std::vector<int> ids = workPiece.getContourIds();
        QString str = QString::number(i++) + ": ";
        for (auto& id : ids) {
            str += QString("%1,").arg(id);
        }
        qDebug() << str;
    }
}

// 辅助函数：生成组合,递归
void EdgeAssembly::generateCombinations(int n, int k, int start, std::vector<int>& current,
                                        std::vector<std::vector<int>>& result) {
    if (current.size() == k) {
        result.push_back(current);
        return;
    }

    for (int i = start; i < n; ++i) {
        current.push_back(i);
        generateCombinations(n, k, i + 1, current, result);
        current.pop_back();
    }
}

// 检查组合是否合法：所有工件不能包含重复的轮廓id，且包含所有轮廓，且不同工件的中心线段不得有交点
bool EdgeAssembly::isCombinationValid(const std::vector<WorkpieceBoundingBox>& workpieces,
                                      const std::vector<int>& combination) {
    std::set<int> allContourIds;

    for (int index : combination) {
        if (index < 0 || index >= workpieces.size()) {
            return false;
        }

        std::vector<int> contourIds = workpieces[index].getContourIds();
        for (int id : contourIds) {
            // 如果id已经存在，说明有重复轮廓，组合不合法
            if (allContourIds.find(id) != allContourIds.end()) {
                return false;
            }
            allContourIds.insert(id);
        }
    }

    // 获取所有轮廓的id
    std::set<int> allExpectedContourIds;
    for (const auto& cbb : m_cbbs) {
        allExpectedContourIds.insert(cbb->getId());
    }

    // 检查组合中的轮廓id是否与所有轮廓的id一致
    if (allContourIds != allExpectedContourIds) {
        return false;
    }

    // 遍历组合中所有两个工件的组合情况
    // 检查不同工件之间的中心线段是否有交点
    for (size_t i = 0; i < combination.size(); ++i) {
        for (size_t j = i + 1; j < combination.size(); ++j) {
            int index1 = combination[i];
            int index2 = combination[j];

            const WorkpieceBoundingBox& workpiece1 = workpieces[index1];
            const WorkpieceBoundingBox& workpiece2 = workpieces[index2];

            // 获取两个工件的中心线段集合
            std::vector<std::pair<cv::Point2f, cv::Point2f>> segments1 = workpiece1.getCenterPointConnections();
            std::vector<std::pair<cv::Point2f, cv::Point2f>> segments2 = workpiece2.getCenterPointConnections();

            // 检查工件1的所有线段与工件2的所有线段是否有交点
            for (const auto& seg1 : segments1) {
                for (const auto& seg2 : segments2) {
                    // 如果两条线段相交，则组合不合法
                    if (doSegmentsIntersect(seg1.first, seg1.second, seg2.first, seg2.second)) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

// // 根据门环工件个数，组合工件，保证轮廓不重合
// void EdgeAssembly::validateCombinations(int n) {
//     if (m_possibleWorkpieces.empty() || n <= 0) {
//         return;
//     }

//     // 如果n大于工件数量，则无法选择n个工件
//     if (n > m_possibleWorkpieces.size()) {
//         return;
//     }

//     // 存储合法的组合
//     std::vector<std::vector<int>> validCombinations;

//     // 优化的组合生成算法：在生成过程中避免重复轮廓ID
//     std::vector<int> current;
//     std::set<int> usedContourIds;
//     generateOptimizedCombinations(0, n, current, usedContourIds, validCombinations);

//     m_validCombinations = validCombinations;

//     // 输出结果（这里可以根据需要存储或处理合法组合）
//     qDebug() << "找到 " << validCombinations.size() << " 个合法的 " << n << " 工件组合：" ;

//     for (size_t i = 0; i < validCombinations.size(); ++i) {
//         QString str1 = "组合 " + QString::number(i + 1) + ": [";
//         for (size_t j = 0; j < validCombinations[i].size(); ++j) {
//             str1 += QString::number(validCombinations[i][j]);
//             if (j < validCombinations[i].size() - 1) {
//                 str1 += ", ";
//             }
//         }
//         str1 += "]";
//         qDebug() << str1;

//         // 输出每个组合中工件的轮廓id
//         QString str2 = "  轮廓id: ";
//         std::set<int> allIds;
//         for (int index : validCombinations[i]) {
//             std::vector<int> ids = m_possibleWorkpieces[index].getContourIds();
//             for (int id : ids) {
//                 allIds.insert(id);
//             }
//         }
//         for (int id : allIds) {
//             str2 += QString::number(id);
//             str2 += ",";
//         };
//         qDebug() << str2;
//     }
//     // 新增：计算最有可能的工件组合
//     calculateMostLikelyCombination();
// }

// // 优化的组合生成算法：递归生成组合，避免包含重复轮廓ID的工件,且工件中心线段不能相交
// void EdgeAssembly::generateOptimizedCombinations(int start, int k,
//                                                  std::vector<int>& current,
//                                                  std::set<int>& usedContourIds,
//                                                  std::vector<std::vector<int>>& result) {
//     if (current.size() == k) {
//         // 检查是否包含了所有轮廓
//         std::set<int> allExpectedContourIds;
//         for (const auto& cbb : m_cbbs) {
//             allExpectedContourIds.insert(cbb->getId());
//         }

//         if (usedContourIds == allExpectedContourIds) {
//             // 检查线段相交情况
//             if (isCombinationValidWithoutIdCheck(m_possibleWorkpieces, current)) {
//                 result.push_back(current);
//             }
//         }
//         return;
//     }

//     for (int i = start; i < m_possibleWorkpieces.size(); ++i) {
//         // 检查当前工件是否包含已使用的轮廓ID
//         std::vector<int> contourIds = m_possibleWorkpieces[i].getContourIds();
//         bool hasDuplicate = false;

//         for (int id : contourIds) {
//             if (usedContourIds.find(id) != usedContourIds.end()) {
//                 hasDuplicate = true;
//                 break;
//             }
//         }

//         // 如果包含重复轮廓ID，跳过该工件
//         if (hasDuplicate) {
//             continue;
//         }

//         // 添加当前工件到组合中
//         current.push_back(i);
//         for (int id : contourIds) {
//             usedContourIds.insert(id);
//         }

//         // 递归生成剩余组合
//         generateOptimizedCombinations(i + 1, k, current, usedContourIds, result);

//         // 回溯：移除当前工件
//         current.pop_back();
//         for (int id : contourIds) {
//             usedContourIds.erase(id);
//         }
//     }
// }




void EdgeAssembly::outputResult() {
    // ... existing code ...
}

// 计算工件组合的总线段长度
float EdgeAssembly::calculateCombinationTotalLength(const std::vector<int>& combination) const {
    float totalLength = 0.0f;

    // 遍历组合中的每个工件
    for (int workpieceIndex : combination) {
        if (workpieceIndex < 0 || workpieceIndex >= m_possibleWorkpieces.size()) {
            continue;
        }

        // 获取当前工件的中心点连接线段
        const WorkpieceBoundingBox& workpiece = m_possibleWorkpieces[workpieceIndex];
        std::vector<std::pair<cv::Point2f, cv::Point2f>> connections = workpiece.getCenterPointConnections();

        // 计算当前工件所有线段的长度之和
        for (const auto& connection : connections) {
            cv::Point2f p1 = connection.first;
            cv::Point2f p2 = connection.second;
            float dx = p2.x - p1.x;
            float dy = p2.y - p1.y;
            float segmentLength = std::sqrt(dx * dx + dy * dy);
            totalLength += segmentLength;
        }
    }

    return totalLength;
}

// 计算最有可能的工件组合（总线段长度最小的组合）
void EdgeAssembly::calculateMostLikelyCombination() {
    if (m_validCombinations.empty()) {
        m_mostLikelyCombination.clear();
        return;
    }

    float minTotalLength = std::numeric_limits<float>::max();
    std::vector<int> bestCombination;

    // 遍历所有合法的组合，计算每个组合的总线段长度
    for (const auto& combination : m_validCombinations) {
        float totalLength = calculateCombinationTotalLength(combination);

        // 如果找到更小的总长度，更新最佳组合
        if (totalLength < minTotalLength) {
            minTotalLength = totalLength;
            bestCombination = combination;
        }
    }

    // 设置最有可能的组合
    m_mostLikelyCombination = bestCombination;

    // 输出结果
    qDebug() << "最有可能的工件组合（总线段长度最小）：";
    qDebug() << "总线段长度：" << minTotalLength;
    qDebug() << "组合索引：" << bestCombination;

    // 输出组合中每个工件的轮廓ID
    QString contourIdsStr = "轮廓ID：";
    std::set<int> allIds;
    for (int index : bestCombination) {
        std::vector<int> ids = m_possibleWorkpieces[index].getContourIds();
        for (int id : ids) {
            allIds.insert(id);
        }
    }
    for (int id : allIds) {
        contourIdsStr += QString::number(id) + ",";
    }
    qDebug() << contourIdsStr;
}

// // 优化的组合生成算法：使用提前剪枝和启发式搜索
// void EdgeAssembly::generateOptimizedCombinations(int start, int k,
//                                                  std::vector<int>& current,
//                                                  std::set<int>& usedContourIds,
//                                                  std::vector<std::vector<int>>& result) {
//     // 提前剪枝：如果剩余工件数量不足以完成组合，直接返回
//     int remainingWorkpieces = m_possibleWorkpieces.size() - start;
//     if (remainingWorkpieces < k - static_cast<int>(current.size())) {
//         return;
//     }

//     if (current.size() == k) {
//         // 检查是否包含了所有轮廓
//         std::set<int> allExpectedContourIds;
//         for (const auto& cbb : m_cbbs) {
//             allExpectedContourIds.insert(cbb->getId());
//         }

//         if (usedContourIds == allExpectedContourIds) {
//             // 检查线段相交情况
//             if (isCombinationValidWithoutIdCheck(m_possibleWorkpieces, current)) {
//                 result.push_back(current);
//             }
//         }
//         return;
//     }

//     // 启发式排序：优先选择包含较少轮廓的工件（减少后续冲突）
//     std::vector<int> indices;
//     for (int i = start; i < m_possibleWorkpieces.size(); ++i) {
//         indices.push_back(i);
//     }

//     // 按工件包含的轮廓数量排序（少的在前）
//     std::sort(indices.begin(), indices.end(), [this](int a, int b) {
//         return m_possibleWorkpieces[a].getContourIds().size() <
//                m_possibleWorkpieces[b].getContourIds().size();
//     });

//     for (int idx : indices) {
//         int i = idx;

//         // 检查当前工件是否包含已使用的轮廓ID
//         std::vector<int> contourIds = m_possibleWorkpieces[i].getContourIds();
//         bool hasDuplicate = false;

//         for (int id : contourIds) {
//             if (usedContourIds.find(id) != usedContourIds.end()) {
//                 hasDuplicate = true;
//                 break;
//             }
//         }

//         // 如果包含重复轮廓ID，跳过该工件
//         if (hasDuplicate) {
//             continue;
//         }

//         // 提前剪枝：检查当前工件与已选工件的线段是否相交
//         bool hasIntersection = false;
//         for (int selectedIdx : current) {
//             const WorkpieceBoundingBox& currentWp = m_possibleWorkpieces[i];
//             const WorkpieceBoundingBox& selectedWp = m_possibleWorkpieces[selectedIdx];

//             if (doWorkpiecesIntersect(currentWp, selectedWp)) {
//                 hasIntersection = true;
//                 break;
//             }
//         }

//         if (hasIntersection) {
//             continue;
//         }

//         // 添加当前工件到组合中
//         current.push_back(i);
//         for (int id : contourIds) {
//             usedContourIds.insert(id);
//         }

//         // 递归生成剩余组合
//         generateOptimizedCombinations(i + 1, k, current, usedContourIds, result);

//         // 回溯：移除当前工件
//         current.pop_back();
//         for (int id : contourIds) {
//             usedContourIds.erase(id);
//         }
//     }
// }

// 检查两个工件是否相交（提前剪枝用）
bool EdgeAssembly::doWorkpiecesIntersect(const WorkpieceBoundingBox& wp1, const WorkpieceBoundingBox& wp2) const {
    // 获取两个工件的中心线段集合
    std::vector<std::pair<cv::Point2f, cv::Point2f>> segments1 = wp1.getCenterPointConnections();
    std::vector<std::pair<cv::Point2f, cv::Point2f>> segments2 = wp2.getCenterPointConnections();

    // 检查工件1的所有线段与工件2的所有线段是否有交点
    for (const auto& seg1 : segments1) {
        for (const auto& seg2 : segments2) {
            if (doSegmentsIntersect(seg1.first, seg1.second, seg2.first, seg2.second)) {
                return true;
            }
        }
    }
    return false;
}

// 检查组合是否合法：只检查线段相交情况（不检查轮廓ID重复，因为已经在生成过程中避免了）
bool EdgeAssembly::isCombinationValidWithoutIdCheck(const std::vector<WorkpieceBoundingBox>& workpieces,
                                                    const std::vector<int>& combination) {
    // 遍历组合中所有两个工件的组合情况
    // 检查不同工件之间的中心线段是否有交点
    for (size_t i = 0; i < combination.size(); ++i) {
        for (size_t j = i + 1; j < combination.size(); ++j) {
            int index1 = combination[i];
            int index2 = combination[j];

            const WorkpieceBoundingBox& workpiece1 = workpieces[index1];
            const WorkpieceBoundingBox& workpiece2 = workpieces[index2];

            // 获取两个工件的中心线段集合
            std::vector<std::pair<cv::Point2f, cv::Point2f>> segments1 = workpiece1.getCenterPointConnections();
            std::vector<std::pair<cv::Point2f, cv::Point2f>> segments2 = workpiece2.getCenterPointConnections();

            // 检查工件1的所有线段与工件2的所有线段是否有交点
            for (const auto& seg1 : segments1) {
                for (const auto& seg2 : segments2) {
                    // 如果两条线段相交，则组合不合法
                    if (doSegmentsIntersect(seg1.first, seg1.second, seg2.first, seg2.second)) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}


// WorkpieceInfo 构造函数实现
EdgeAssembly::WorkpieceInfo::WorkpieceInfo(int idx, const WorkpieceBoundingBox& wp) {
    index = idx;
    contourMask = 0;
    contourCount = 0;

    std::vector<int> ids = wp.getContourIds();
    contourCount = ids.size();
    for (int id : ids) {
        if (id < 64) { // 假设最多64个轮廓
            contourMask |= (1ULL << id);
        }
    }

    // 计算边界框面积作为启发式信息
    cv::RotatedRect rect = wp.getouterBoundingBox();
    boundingBoxArea = rect.size.width * rect.size.height;
}

// 计算位掩码中1的个数（Windows兼容版本）
int EdgeAssembly::countBits(ContourSet mask) const {
    int count = 0;
    while (mask) {
        count += (mask & 1);
        mask >>= 1;
    }
    return count;
}

// 创建目标轮廓掩码
EdgeAssembly::ContourSet EdgeAssembly::createTargetMask() const {
    ContourSet targetMask = 0;
    for (const auto& cbb : m_cbbs) {
        int id = cbb->getId();
        if (id < 64) {
            targetMask |= (1ULL << id);
        }
    }
    return targetMask;
}

// 获取从指定位置开始的最大轮廓数
int EdgeAssembly::getMaxContoursPerWorkpiece(const std::vector<WorkpieceInfo>& infos, int start) const {
    int maxContours = 0;
    for (int i = start; i < infos.size(); ++i) {
        if (infos[i].contourCount > maxContours) {
            maxContours = infos[i].contourCount;
        }
    }
    return maxContours;
}

// 检查工件是否与已选工件相交
bool EdgeAssembly::hasIntersectionWithSelected(const std::vector<int>& selected, int newIndex) const {
    const WorkpieceBoundingBox& newWp = m_possibleWorkpieces[newIndex];

    for (int idx : selected) {
        const WorkpieceBoundingBox& existingWp = m_possibleWorkpieces[idx];
        if (doWorkpiecesIntersect(newWp, existingWp)) {
            return true;
        }
    }
    return false;
}

// 动态规划+位运算优化的组合生成算法
void EdgeAssembly::generateOptimizedCombinationsDP(int k, std::vector<std::vector<int>>& result) {
    if (m_possibleWorkpieces.empty() || k <= 0) {
        return;
    }

    // 预处理工件信息
    std::vector<WorkpieceInfo> workpieceInfos;
    for (int i = 0; i < m_possibleWorkpieces.size(); ++i) {
        workpieceInfos.emplace_back(i, m_possibleWorkpieces[i]);
    }

    // 按轮廓数量降序排序（启发式：先选包含轮廓多的工件）
    std::sort(workpieceInfos.begin(), workpieceInfos.end(),
              [](const WorkpieceInfo& a, const WorkpieceInfo& b) {
                  return a.contourCount > b.contourCount;
              });

    // 目标轮廓掩码（所有轮廓）
    ContourSet targetMask = createTargetMask();

    // 使用DFS+剪枝
    std::vector<int> current;
    ContourSet usedMask = 0;
    dfsCombinations(workpieceInfos, 0, k, targetMask, usedMask, current, result);
}

// DFS搜索组合（动态规划核心）
void EdgeAssembly::dfsCombinations(const std::vector<WorkpieceInfo>& infos, int start, int k,
                                   ContourSet targetMask, ContourSet usedMask,
                                   std::vector<int>& current, std::vector<std::vector<int>>& result) {
    // 剪枝1：如果已经找到足够组合（限制最大组合数）
    if (!result.empty() && result.size() >= 10) {
        return;
    }

    // 剪枝2：如果当前组合已经包含所有轮廓
    if ((usedMask & targetMask) == targetMask) {
        if (current.size() <= k) {
            // 检查线段相交
            if (isCombinationValidWithoutIdCheck(m_possibleWorkpieces, current)) {
                result.push_back(current);
            }
        }
        return;
    }

    // 剪枝3：如果已经达到k个工件但还没覆盖所有轮廓
    if (current.size() == k) {
        return;
    }

    // 剪枝4：计算剩余轮廓和最大可能覆盖
    int remainingContours = countBits(targetMask & ~usedMask);
    int maxContoursPerWp = getMaxContoursPerWorkpiece(infos, start);
    int remainingSlots = k - current.size();

    // 如果剩余轮廓数大于剩余槽位能覆盖的最大轮廓数，剪枝
    if (remainingContours > remainingSlots * maxContoursPerWp) {
        return;
    }

    // 剪枝5：如果剩余工件数量不足
    if (start >= infos.size()) {
        return;
    }

    for (int i = start; i < infos.size(); ++i) {
        const WorkpieceInfo& info = infos[i];

        // 剪枝6：如果当前工件包含已使用的轮廓
        if (info.contourMask & usedMask) {
            continue;
        }

        // 剪枝7：如果当前工件与已选工件相交
        if (hasIntersectionWithSelected(current, info.index)) {
            continue;
        }

        // 剪枝8：如果添加当前工件后，剩余轮廓无法被剩余工件覆盖
        ContourSet newUsedMask = usedMask | info.contourMask;
        int newRemainingContours = countBits(targetMask & ~newUsedMask);
        int newRemainingSlots = k - current.size() - 1;

        if (newRemainingContours > 0 && newRemainingSlots == 0) {
            continue;
        }

        if (newRemainingContours > newRemainingSlots * getMaxContoursPerWorkpiece(infos, i + 1)) {
            continue;
        }

        current.push_back(info.index);
        dfsCombinations(infos, i + 1, k, targetMask, newUsedMask, current, result);
        current.pop_back();

        // 剪枝9：如果已经找到足够组合，提前终止
        if (!result.empty() && result.size() >= 10) {
            break;
        }
    }
}

// 修改validateCombinations方法，使用新的优化算法
void EdgeAssembly::validateCombinations(int n) {
    if (m_possibleWorkpieces.empty() || n <= 0) {
        return;
    }

    if (n > m_possibleWorkpieces.size()) {
        return;
    }

    std::vector<std::vector<int>> validCombinations;

    // 根据轮廓数量选择优化策略
    if (m_cbbs.size() <= 64) { // 如果轮廓数不超过64，使用位运算优化
        qDebug() << "使用动态规划+位运算优化算法";
        generateOptimizedCombinationsDP(n, validCombinations);
    } else {
        // 回退到原来的算法（但使用更多剪枝）
        qDebug() << "轮廓数量超过64，使用原始算法（带剪枝）";
        std::vector<int> current;
        std::set<int> usedContourIds;
        generateOptimizedCombinations(0, n, current, usedContourIds, validCombinations);
    }

    m_validCombinations = validCombinations;

    // 输出结果
    qDebug() << "找到 " << validCombinations.size() << " 个合法的 " << n << " 工件组合：" ;

    for (size_t i = 0; i < validCombinations.size(); ++i) {
        QString str1 = "组合 " + QString::number(i + 1) + ": [";
        for (size_t j = 0; j < validCombinations[i].size(); ++j) {
            str1 += QString::number(validCombinations[i][j]);
            if (j < validCombinations[i].size() - 1) {
                str1 += ", ";
            }
        }
        str1 += "]";
        qDebug() << str1;

        // 输出每个组合中工件的轮廓id
        QString str2 = "  轮廓id: ";
        std::set<int> allIds;
        for (int index : validCombinations[i]) {
            std::vector<int> ids = m_possibleWorkpieces[index].getContourIds();
            for (int id : ids) {
                allIds.insert(id);
            }
        }
        for (int id : allIds) {
            str2 += QString::number(id);
            str2 += ",";
        };
        qDebug() << str2;
    }

    // 新增：计算最有可能的工件组合
    calculateMostLikelyCombination();
}

// 优化的组合生成算法：使用多种剪枝策略和启发式搜索
void EdgeAssembly::generateOptimizedCombinations(int start, int k,
                                                 std::vector<int>& current,
                                                 std::set<int>& usedContourIds,
                                                 std::vector<std::vector<int>>& result) {
    // 提前剪枝1：如果剩余工件数量不足以完成组合，直接返回
    int remainingWorkpieces = m_possibleWorkpieces.size() - start;
    if (remainingWorkpieces < k - static_cast<int>(current.size())) {
        return;
    }

    // 提前剪枝2：如果当前已使用的轮廓ID数量已经超过总轮廓数，直接返回
    if (usedContourIds.size() > m_cbbs.size()) {
        return;
    }

    // 提前剪枝3：如果剩余轮廓不足以覆盖所有轮廓，直接返回
    int remainingContours = m_cbbs.size() - usedContourIds.size();
    int maxPossibleContoursFromRemaining = 0;
    for (int i = start; i < m_possibleWorkpieces.size(); ++i) {
        maxPossibleContoursFromRemaining += m_possibleWorkpieces[i].getContourIds().size();
    }
    if (maxPossibleContoursFromRemaining < remainingContours) {
        return;
    }

    if (current.size() == k) {
        // 检查是否包含了所有轮廓
        if (usedContourIds.size() == m_cbbs.size()) {
            // 检查线段相交情况
            if (isCombinationValidWithoutIdCheck(m_possibleWorkpieces, current)) {
                result.push_back(current);
            }
        }
        return;
    }

    // 启发式排序：优先选择包含较多轮廓的工件（更快覆盖所有轮廓）
    std::vector<int> indices;
    for (int i = start; i < m_possibleWorkpieces.size(); ++i) {
        indices.push_back(i);
    }

    // 按工件包含的轮廓数量排序（多的在前，更快覆盖所有轮廓）
    std::sort(indices.begin(), indices.end(), [this](int a, int b) {
        return m_possibleWorkpieces[a].getContourIds().size() >
               m_possibleWorkpieces[b].getContourIds().size();
    });

    for (int idx : indices) {
        int i = idx;

        // 检查当前工件是否包含已使用的轮廓ID
        std::vector<int> contourIds = m_possibleWorkpieces[i].getContourIds();
        bool hasDuplicate = false;

        for (int id : contourIds) {
            if (usedContourIds.find(id) != usedContourIds.end()) {
                hasDuplicate = true;
                break;
            }
        }

        // 如果包含重复轮廓ID，跳过该工件
        if (hasDuplicate) {
            continue;
        }

        // 提前剪枝：检查当前工件与已选工件的线段是否相交
        bool hasIntersection = false;
        for (int selectedIdx : current) {
            const WorkpieceBoundingBox& currentWp = m_possibleWorkpieces[i];
            const WorkpieceBoundingBox& selectedWp = m_possibleWorkpieces[selectedIdx];

            if (doWorkpiecesIntersect(currentWp, selectedWp)) {
                hasIntersection = true;
                break;
            }
        }

        if (hasIntersection) {
            continue;
        }

        // 添加当前工件到组合中
        current.push_back(i);
        for (int id : contourIds) {
            usedContourIds.insert(id);
        }

        // 递归生成剩余组合
        generateOptimizedCombinations(i + 1, k, current, usedContourIds, result);

        // 回溯：移除当前工件
        current.pop_back();
        for (int id : contourIds) {
            usedContourIds.erase(id);
        }

        // 如果已经找到足够多的组合，可以提前终止
        if (!result.empty() && result.size() >= 10) { // 限制最大组合数
            break;
        }
    }
}
