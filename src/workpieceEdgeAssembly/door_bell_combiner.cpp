#include <QString>
#include <plog/Log.h>

#include "door_bell_combiner.h"

//===========================WorkpieceInfo===========================
/**
 * @brief WorkpieceInfo 构造函数
 * @param idx 工件索引
 * @param wp 工件边界框
 */
DoorBellCombiner::WorkpieceInfo::WorkpieceInfo(int idx, const WorkpieceBoundingBox& wp) {
    index = idx;
    contourMask = 0;
    contourCount = 0;

    std::vector<int> ids = wp.getContourIds();
    contourCount = static_cast<int>(ids.size());
    for (int id : ids) {
        if (id < 64) {  // 假设最多64个轮廓
            contourMask |=
                (1ULL << id);  // 将对应位设置为1，表示该轮廓存在。便于后续使用位与操作 & 快速判断两个工件是否有重复轮廓
            // 1ULL << id：将数字1左移id位，创建一个只有第id位为1的位掩码
            // contourMask |= ...：使用位或操作将对应位设置为1
        }
    }

    // 计算边界框面积作为启发式信息
    cv::RotatedRect rect = wp.getouterBoundingBox();
    boundingBoxArea = rect.size.width * rect.size.height;
}

// ===========================DoorBellCombiner===========================
DoorBellCombiner::DoorBellCombiner(const std::vector<WorkpieceBoundingBox>& possibleWorkpieces)
    : m_possibleWorkpieces(possibleWorkpieces)
{}

/**
 * @brief 生成并验证有效的门环工件组合
 * @param n 需要选择的工件数量
 *
 * @details
 * 该函数根据指定的工件数量n，生成并验证所有可能的工件组合：
 * 1. 生成所有包含n个工件的组合
 * 2. 验证每个组合是否满足以下条件：
 *    - 包含所有轮廓（不遗漏任何轮廓）
 *    - 轮廓不重复（每个轮廓只被一个工件包含）
 *    - 工件之间不相交（避免空间冲突）
 */
void DoorBellCombiner::generateValidCombinations(int n, const std::vector<std::shared_ptr<ContourBoundingBox>>& cbbs) {
    if (m_possibleWorkpieces.empty() || n <= 0) {
        return;
    }

    if (n > m_possibleWorkpieces.size()) {
        return;
    }

    std::vector<std::vector<int>> validCombinations;

    // 根据轮廓数量选择优化策略
    if (cbbs.size() <= 64) {  // 如果轮廓数不超过64，使用位运算优化
        // PLOG_INFO << "使用动态规划+位运算优化算法";
        generateOptimizedCombinationsDP(n, validCombinations, cbbs);
    } else {
        // 回退到原来的算法（但使用更多剪枝）
        PLOG_INFO << "轮廓数量超过64，使用原始算法（带剪枝）";
        std::vector<int> current;
        std::set<int> usedContourIds;
        generateOptimizedCombinations(0, n, current, usedContourIds, validCombinations, cbbs);
    }

    m_validCombinations = validCombinations;

    // 输出结果
    PLOG_INFO << "找到 " << validCombinations.size() << " 个合法的 " << n << " 工件组合：" ;

    for (size_t i = 0; i < validCombinations.size(); ++i) {
        QString str1 = "组合 " + QString::number(i + 1) + ": [";
        for (size_t j = 0; j < validCombinations[i].size(); ++j) {
            str1 += QString::number(validCombinations[i][j]);
            if (j < validCombinations[i].size() - 1) {
                str1 += ", ";
            }
        }
        str1 += "]";
        PLOG_INFO << str1.toStdString();

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
        PLOG_INFO << str2.toStdString();
    }
}

/**
 * @brief 计算最有可能的工件组合
 * @return 无返回值
 */
void DoorBellCombiner::calculateMostLikelyCombination() {
    // TODO: 后续用 sourceImageId 等规则重新设计组合筛选
    static const std::vector<int> kFixedCombination = {3, 16, 11, 30, 28, 48, 51};

    for (int idx : kFixedCombination) {
        if (idx < 0 || idx >= static_cast<int>(m_possibleWorkpieces.size())) {
            PLOG_WARNING << "硬编码组合下标 " << idx << " 超出 m_possibleWorkpieces 范围 (size="
                         << m_possibleWorkpieces.size() << ")，跳过";
            m_mostLikelyCombination.clear();
            return;
        }
    }

    m_mostLikelyCombination = kFixedCombination;

    PLOG_INFO << "最有可能的工件组合（临时硬编码）：";
    PLOG_INFO << "组合索引：" << m_mostLikelyCombination;

    // 输出组合中每个工件的轮廓ID
    QString contourIdsStr = "ID：";
    std::set<int> allIds;
    for (int index : kFixedCombination) {
        std::vector<int> ids = m_possibleWorkpieces[index].getContourIds();
        for (int id : ids) {
            allIds.insert(id);
        }
    }
    for (int id : allIds) {
        contourIdsStr += QString::number(id) + ",";
    }
    PLOG_INFO << contourIdsStr.toStdString();
}

std::map<int, std::vector<int>> DoorBellCombiner::outputResult() {
    // 键为工件序号，值为轮廓id数组
    std::map<int, std::vector<int>> resultDictionary;

    // 检查是否有最可能的组合
    if (m_mostLikelyCombination.empty()) {
        PLOG_INFO << "没有找到有效的工件组合";
        return resultDictionary;
    }

    // 遍历最可能组合中的每个工件
    for (size_t i = 0; i < m_mostLikelyCombination.size(); ++i) {
        int workpieceIndex = m_mostLikelyCombination[i];
        if (workpieceIndex < 0 || workpieceIndex >= m_possibleWorkpieces.size()) {
            continue;
        }
        std::vector<int> contourIds = m_possibleWorkpieces[workpieceIndex].getContourIds();
        resultDictionary[i] = contourIds;
    }

    return resultDictionary;
}

/**
 * @brief 获取工件集合中最大轮廓数
 * @param infos 工件信息集合
 * @param start 起始索引
 * @return 最大轮廓数
 */
int DoorBellCombiner::getMaxContoursPerWorkpiece(const std::vector<WorkpieceInfo>& infos, int start) const {
    int maxContours = 0;
    for (int i = start; i < infos.size(); ++i) {
        if (infos[i].contourCount > maxContours) {
            maxContours = infos[i].contourCount;
        }
    }
    return maxContours;
}

/**
 * @brief 检查工件是否与已选工件相交
 * @param selected 已选工件索引数组
 * @param newIndex 新工件索引
 * @return 是否相交
 */
bool DoorBellCombiner::hasIntersectionWithSelected(const std::vector<int>& selected, int newIndex) const {
    const WorkpieceBoundingBox& newWp = m_possibleWorkpieces[newIndex];

    for (int idx : selected) {
        const WorkpieceBoundingBox& existingWp = m_possibleWorkpieces[idx];
        if (doWorkpiecesIntersect(newWp, existingWp)) {
            return true;
        }
    }
    return false;
}

/**
 * @brief 计算轮廓集合掩码中设置的位数（即包含的轮廓数量）
 * @param mask 轮廓集合掩码，每个位代表一个轮廓ID（0-63）
 * @return 掩码中设置的位数，即该掩码代表的轮廓数量
 *
 * @details
 * 轮廓集合掩码(ContourSet)是一个64位无符号整数(uint64_t)，每个位对应一个轮廓ID：
 * - 位0对应轮廓ID 0
 * - 位1对应轮廓ID 1
 * - ...
 * - 位63对应轮廓ID 63
 *
 * 函数通过位运算统计掩码中值为1的位的数量，这个数量代表：
 * 1. 当前组合已经覆盖的轮廓数量
 * 2. 或者目标掩码中需要覆盖的总轮廓数量
 *
 * 例如：
 * - mask = 0b1010 (二进制) → 返回2（有2个位被设置）
 * - mask = 0b1111 (二进制) → 返回4（有4个位被设置）
 * - mask = 0 (二进制) → 返回0（没有位被设置）
 */
int DoorBellCombiner::countBits(ContourSet mask) const {
    int count = 0;
    while (mask) {
        count += (mask & 1);
        mask >>= 1;
    }
    return count;
}

/**
 * @brief 创建目标轮廓掩码，表示需要覆盖的所有轮廓
 * @return 目标轮廓掩码，每个设置的位代表一个需要覆盖的轮廓ID
 *
 * @details
 * 目标轮廓掩码是一个64位无符号整数，用于表示所有需要被工件组合覆盖的轮廓。
 * 函数遍历所有轮廓边界框(m_cbbs)，根据每个轮廓的ID设置对应的位：
 * - 轮廓ID 0 对应位0
 * - 轮廓ID 1 对应位1
 * - ...
 * - 轮廓ID 63 对应位63
 *
 * 例如，如果系统中有轮廓ID为0、2、5的轮廓，则目标掩码为：
 * 0b00100101 (二进制) = 37 (十进制)
 *
 * 这个掩码用于后续的组合生成算法中，判断是否已经覆盖了所有需要的轮廓。
 */
DoorBellCombiner::ContourSet DoorBellCombiner::createTargetMask(const std::vector<std::shared_ptr<ContourBoundingBox>>& cbbs) const {
    ContourSet targetMask = 0;
    for (const auto& cbb : cbbs) {
        int id = cbb->getId();
        if (id < 64) {
            targetMask |= (1ULL << id);
        }
    }
    return targetMask;
}

/**
 * @brief 使用多种剪枝策略和启发式搜索的组合生成算法
 * @param start 起始搜索索引
 * @param k 需要选择的工件数量
 * @param current 当前已选择的工件索引组合
 * @param usedContourIds 已使用的轮廓ID集合
 * @param result 存储所有有效组合的结果容器
 *
 * @details
 * 算法流程：
 * 1. 检查剪枝条件，提前终止无效分支
 * 2. 当组合大小达到k时，验证是否覆盖所有轮廓且工件不相交
 * 3. 对剩余工件进行启发式排序
 * 4. 递归搜索所有可能的组合，应用剪枝策略
 */
void DoorBellCombiner::generateOptimizedCombinations(int start,
                                                      int k,
                                                      std::vector<int>& current,
                                                      std::set<int>& usedContourIds,
                                                      std::vector<std::vector<int>>& result,
                                                      const std::vector<std::shared_ptr<ContourBoundingBox>>& cbbs) {
    // 提前剪枝1：如果剩余工件数量不足以完成组合，直接返回
    int remainingWorkpieces = static_cast<int>(m_possibleWorkpieces.size()) - start;
    if (remainingWorkpieces < k - static_cast<int>(current.size())) {
        return;
    }

    // 提前剪枝2：如果当前已使用的轮廓ID数量已经超过总轮廓数，直接返回
    if (usedContourIds.size() > cbbs.size()) {
        return;
    }

    // 提前剪枝3：如果剩余轮廓不足以覆盖所有轮廓，直接返回
    int remainingContours = static_cast<int>(cbbs.size()) - static_cast<int>(usedContourIds.size());
    int maxPossibleContoursFromRemaining = 0;
    for (int i = start; i < m_possibleWorkpieces.size(); ++i) {
        maxPossibleContoursFromRemaining += static_cast<int>(m_possibleWorkpieces[i].getContourIds().size());
    }
    if (maxPossibleContoursFromRemaining < remainingContours) {
        return;
    }

    if (current.size() == k) {
        // 检查是否包含了所有轮廓
        if (usedContourIds.size() == cbbs.size()) {
            // 检查线段相交情况
            if (checkWorkpieceIntersections(m_possibleWorkpieces, current)) {
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
        return m_possibleWorkpieces[a].getContourIds().size() > m_possibleWorkpieces[b].getContourIds().size();
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
        generateOptimizedCombinations(i + 1, k, current, usedContourIds, result, cbbs);

        // 回溯：移除当前工件
        current.pop_back();
        for (int id : contourIds) {
            usedContourIds.erase(id);
        }
    }
}

/**
 * @brief 动态规划+位运算优化的组合生成算法
 * @param k 需要选择的工件数量
 * @param result 存储所有有效组合的结果容器
 *
 * @details
 * 算法流程：
 * 1. 预处理工件信息，包括轮廓掩码和轮廓数量
 * 2. 按轮廓数量降序排序工件（启发式策略）
 * 3. 创建目标轮廓掩码，表示需要覆盖的所有轮廓
 * 4. 调用DFS算法进行组合搜索，使用位运算优化状态表示
 * 5. 应用剪枝策略减少搜索空间
 */
void DoorBellCombiner::generateOptimizedCombinationsDP(int k,
                                                        std::vector<std::vector<int>>& result,
                                                        const std::vector<std::shared_ptr<ContourBoundingBox>>& cbbs)
{
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
              [](const WorkpieceInfo& a, const WorkpieceInfo& b) { return a.contourCount > b.contourCount; });

    // 目标轮廓掩码（所有轮廓），表示最终需要覆盖的轮廓id
    ContourSet targetMask = createTargetMask(cbbs);

    // 使用DFS+剪枝
    std::vector<int> current;
    ContourSet usedMask = 0;
    dfsCombinations(workpieceInfos, 0, k, targetMask, usedMask, current, result);
}

/**
 * @brief DFS搜索组合（动态规划核心）
 * @param infos 工件信息列表，已按轮廓数量降序排序
 * @param start 起始搜索索引
 * @param k 需要选择的工件数量
 * @param targetMask 目标轮廓掩码，表示需要覆盖的所有轮廓
 * @param usedMask 已使用的轮廓掩码，表示当前组合已覆盖的轮廓
 * @param current 当前已选择的工件索引组合
 * @param result 存储所有有效组合的结果容器
 *
 * @details
 * 该函数是动态规划算法的核心DFS实现，使用位运算优化状态表示和剪枝：
 * 1. 位运算状态：使用64位掩码表示轮廓集合，提高集合操作效率
 * 2. 多种剪枝策略：
 *    - 剪枝1：当前组合已覆盖所有轮廓时，验证并保存结果
 *    - 剪枝2：达到k个工件但未覆盖所有轮廓时，终止分支
 *    - 剪枝3：基于剩余轮廓数量和最大可能覆盖进行剪枝
 *    - 剪枝4：剩余工件数量不足时终止
 *    - 剪枝5：跳过包含已使用轮廓的工件
 */
void DoorBellCombiner::dfsCombinations(const std::vector<WorkpieceInfo>& infos, int start, int k, ContourSet targetMask,
                                   ContourSet usedMask, std::vector<int>& current, std::vector<std::vector<int>>& result) {
    // 剪枝1：如果当前组合已经包含所有轮廓
    if ((usedMask & targetMask) == targetMask) {
        if (current.size() <= k) {
            // 检查线段相交
            if (checkWorkpieceIntersections(m_possibleWorkpieces, current)) {
                result.push_back(current);
            }
        }
        return;
    }

    // 剪枝2：如果已经达到k个工件但还没覆盖所有轮廓
    if (current.size() == k) {
        return;
    }

    // 剪枝3：计算剩余轮廓和最大可能覆盖
    int remainingContours = countBits(targetMask & ~usedMask);
    int maxContoursPerWp = getMaxContoursPerWorkpiece(infos, start);
    int remainingSlots = k - static_cast<int>(current.size());

    // 如果剩余轮廓数大于剩余槽位能覆盖的最大轮廓数，剪枝
    if (remainingContours > remainingSlots * maxContoursPerWp) {
        return;
    }

    // 剪枝4：如果剩余工件数量不足
    if (start >= infos.size()) {
        return;
    }

    for (int i = start; i < infos.size(); ++i) {
        const WorkpieceInfo& info = infos[i];

        // 剪枝5：如果当前工件包含已使用的轮廓
        if (info.contourMask & usedMask) {
            continue;
        }

        // 剪枝6：如果当前工件与已选工件相交
        if (hasIntersectionWithSelected(current, info.index)) {
            continue;
        }

        // 剪枝7：如果添加当前工件后，剩余轮廓无法被剩余工件覆盖
        ContourSet newUsedMask = usedMask | info.contourMask;
        int newRemainingContours = countBits(targetMask & ~newUsedMask);
        int newRemainingSlots = k - static_cast<int>(current.size()) - 1;

        if (newRemainingContours > 0 && newRemainingSlots == 0) {
            continue;
        }

        if (newRemainingContours > newRemainingSlots * getMaxContoursPerWorkpiece(infos, i + 1)) {
            continue;
        }

        current.push_back(info.index);
        dfsCombinations(infos, i + 1, k, targetMask, newUsedMask, current, result);
        current.pop_back();

        // 剪枝8：如果已经找到足够组合，提前终止
        if (!result.empty() && result.size() >= 10) {
            break;
        }
    }
}

/**
 * @brief 检查两个工件是否相交
 * @param wp1 第一个工件
 * @param wp2 第二个工件
 * @return 是否相交
 */
bool DoorBellCombiner::doWorkpiecesIntersect(const WorkpieceBoundingBox& wp1, const WorkpieceBoundingBox& wp2) const {
    // 获取两个工件的中心线段集合
    std::vector<std::pair<cv::Point2f, cv::Point2f>> segments1 = wp1.getCenterPointConnections();
    std::vector<std::pair<cv::Point2f, cv::Point2f>> segments2 = wp2.getCenterPointConnections();

    // 检查工件1的所有线段与工件2的所有线段是否有交点
    for (const auto& seg1 : segments1) {
        for (const auto& seg2 : segments2) {
            if (GeometryUtils::doSegmentsIntersect(seg1.first, seg1.second, seg2.first, seg2.second)) {
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief 检查组合是否合法：根据组合中是否有工件相交
 * @param workpieces 工件集合
 * @param combination 工件组合索引数组
 * @return 组合是否合法
 */
bool DoorBellCombiner::checkWorkpieceIntersections(const std::vector<WorkpieceBoundingBox>& workpieces,
                                               const std::vector<int>& combination) {
    // 遍历组合中所有两个工件的组合情况
    // 检查不同工件之间的中心线段是否有交点
    for (size_t i = 0; i < combination.size(); ++i) {
        for (size_t j = i + 1; j < combination.size(); ++j) {
            int index1 = combination[i];
            int index2 = combination[j];

            const WorkpieceBoundingBox& workpiece1 = workpieces[index1];
            const WorkpieceBoundingBox& workpiece2 = workpieces[index2];

            if (doWorkpiecesIntersect(workpiece1, workpiece2)) {
                return false;
            }
        }
    }

    return true;
}


/**
 * @brief 计算工件组合的总线段长度
 * @param combination 工件组合索引数组
 * @return 总线段长度
 */
float DoorBellCombiner::calculateCombinationTotalLength(const std::vector<int>& combination) const {
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
