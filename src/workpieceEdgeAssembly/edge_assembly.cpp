#include "edge_assembly.h"
#include <iostream>

EdgeAssembly::EdgeAssembly() {}


void EdgeAssembly::makeTestExample(JointSeam jointSeam) {
    // std::vector<ContourCurve> contourCurves = jointSeam.getContourCurves();
    // std::vector<cv::Point2f> contourCurve0 = contourCurves[0].getSortedSubpixelContours();
    // std::vector<cv::Point2f> contourCurve1 = contourCurves[1].getSortedSubpixelContours();

    // cv::Point2f offsetPoint = cv::Point2f(300.0, 500.0);
    // std::vector<cv::Point2f> offsetContourCurve0;
    // for (auto& point : contourCurve0) {
    //     offsetContourCurve0.push_back(cv::Point2f(point.x + offsetPoint.x, point.y + offsetPoint.y));
    // }
    // std::vector<cv::Point2f> offsetContourCurve1;
    // for (auto& point : contourCurve1) {
    //     offsetContourCurve1.push_back(cv::Point2f(point.x + offsetPoint.x, point.y + offsetPoint.y));
    // }

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
        for (int j = i + 1; j < unpairedContours.size(); ++j) {
            WorkpieceBoundingBox wp;
            wp.addContourBoundingBox(unpairedContours[i]);
            wp.addContourBoundingBox(unpairedContours[j]);

            // 检查组合是否合法：遍历所有未组合轮廓，判断是否能合法加入
            bool isLegalCombination = true;
            for (const auto& candidate : unpairedContours) {
                if (!wp.isLegal(candidate)) {
                    isLegalCombination = false;
                    break;
                }
            }

            if (isLegalCombination) {
                m_resultWorkpieces.push_back(wp);
            }
        }
    }

    // 生成3轮廓组合
    for (int i = 0; i < unpairedContours.size(); ++i) {
        for (int j = i + 1; j < unpairedContours.size(); ++j) {
            for (int k = j + 1; k < unpairedContours.size(); ++k) {
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
                    m_resultWorkpieces.push_back(wp);
                }
            }
        }
    }
}

// 辅助函数：生成组合
void generateCombinations(int n, int k, int start, std::vector<int>& current,
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

// 检查组合是否合法：所有工件不能包含重复的轮廓id
bool isCombinationValid(const std::vector<WorkpieceBoundingBox>& workpieces,
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

    return true;
}

// 根据门环工件个数，组合工件，保证轮廓不重合
void EdgeAssembly::validateCombinations(int n) {
    if (m_resultWorkpieces.empty() || n <= 0) {
        return;
    }

    // 如果n大于工件数量，则无法选择n个工件
    if (n > m_resultWorkpieces.size()) {
        return;
    }

    // 生成所有可能的组合
    std::vector<std::vector<int>> allCombinations;
    std::vector<int> current;
    generateCombinations(static_cast<int>(m_resultWorkpieces.size()), n, 0, current, allCombinations);

    // 存储合法的组合
    std::vector<std::vector<int>> validCombinations;

    // 检查每个组合是否合法
    for (const auto& combination : allCombinations) {
        if (isCombinationValid(m_resultWorkpieces, combination)) {
            validCombinations.push_back(combination);
        }
    }

    // 输出结果（这里可以根据需要存储或处理合法组合）
    std::cout << "找到 " << validCombinations.size() << " 个合法的 " << n << " 工件组合：" << std::endl;

    for (size_t i = 0; i < validCombinations.size(); ++i) {
        std::cout << "组合 " << i + 1 << ": [";
        for (size_t j = 0; j < validCombinations[i].size(); ++j) {
            std::cout << validCombinations[i][j];
            if (j < validCombinations[i].size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;

        // 输出每个组合中工件的轮廓id
        std::cout << "  轮廓id: ";
        std::set<int> allIds;
        for (int index : validCombinations[i]) {
            std::vector<int> ids = m_resultWorkpieces[index].getContourIds();
            for (int id : ids) {
                allIds.insert(id);
            }
        }
        for (int id : allIds) {
            std::cout << id << " ";
        }
        std::cout << std::endl;
    }
}


void EdgeAssembly::outputResult() {
    // ... existing code ...
}
