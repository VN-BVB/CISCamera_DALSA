#include "edge_assembly.h"
#include <iostream>
#include <QDebug>

EdgeAssembly::EdgeAssembly(const std::vector<std::shared_ptr<ContourBoundingBox>> cbbs)
{
    m_cbbs = cbbs;
}


void EdgeAssembly::makeTestExample(JointSeam jointSeam) {


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

// 检查组合是否合法：所有工件不能包含重复的轮廓id，且包含所有轮廓
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

    return true;
}

// 根据门环工件个数，组合工件，保证轮廓不重合
void EdgeAssembly::validateCombinations(int n) {
    if (m_possibleWorkpieces.empty() || n <= 0) {
        return;
    }

    // 如果n大于工件数量，则无法选择n个工件
    if (n > m_possibleWorkpieces.size()) {
        return;
    }

    // 生成所有可能的组合
    std::vector<std::vector<int>> allCombinations;
    std::vector<int> current;
    generateCombinations(static_cast<int>(m_possibleWorkpieces.size()), n, 0, current, allCombinations);

    // 存储合法的组合
    std::vector<std::vector<int>> validCombinations;

    // 检查每个组合是否合法
    for (const auto& combination : allCombinations) {
        if (isCombinationValid(m_possibleWorkpieces, combination)) {
            validCombinations.push_back(combination);
        }
    }
    m_validCombinations = validCombinations;

    // 输出结果（这里可以根据需要存储或处理合法组合）
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
}


void EdgeAssembly::outputResult() {
    // ... existing code ...
}
