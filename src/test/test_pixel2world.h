#ifndef TEST_PIXEL2WORLD_H
#define TEST_PIXEL2WORLD_H

#include <opencv2/core/core.hpp>
#include <Eigen/Core>
#include <vector>

class TestPixel2World
{
public:
    TestPixel2World();
    void testPixel2World();
    void runBlob();
    void calculateNearestNeighborDistance(const std::vector<Eigen::Vector2d>& worldPoints);
    bool calculateImagePoints(cv::Mat imageInput, cv::Size boardSize,
                              std::vector<cv::Point2d>& imagePoints);
    void calculateTwoPointDistance(const std::vector<Eigen::Vector2d>& worldPoints);
    // 新增：计算旋转校正并可视化
    void rotateAndVisualize(const std::vector<Eigen::Vector2d>& worldPoints,
                            size_t idx0 = 0, size_t idx1 = 10);
    // 新增：排序后按每37个点分组着色显示
    void sortAndDisplayGrouped(const std::vector<Eigen::Vector2d>& rotatedPoints,
                               int groupSize = 37);
    // 新增：计算每组首尾点间距的统计分析
    void calculateGroupSpanDistance(const std::vector<Eigen::Vector2d>& sortedPoints,
                                    int groupSize = 37);

private:
    // 可视化旋转前后的点
    void displayRotatedPoints(const std::vector<Eigen::Vector2d>& originalPoints,
                              const std::vector<Eigen::Vector2d>& rotatedPoints,
                              size_t idx0, size_t idx1);
};

#endif // TEST_PIXEL2WORLD_H
