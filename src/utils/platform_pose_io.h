#ifndef PLATFORM_POSE_IO_H
#define PLATFORM_POSE_IO_H

#include <string>
#include <vector>
#include <Eigen/Core>

// 对位平台坐标系：X/Y 为单位方向向量（世界系），T 为原点（世界系）。
// 直接对应 platform_pose.json 中每个 platform 对象的三个数组。
struct PlatformAxis {
    int id = -1;
    Eigen::Vector3d X = Eigen::Vector3d::Zero();
    Eigen::Vector3d Y = Eigen::Vector3d::Zero();
    Eigen::Vector3d T = Eigen::Vector3d::Zero();
};

// 读取 platform_pose.json，填充 out。
// 容忍 key 与数组之间的空白（实际 JSON 文件里写作 "X": [\n  0.04, ...\n]）。
// 成功返回 true；文件打不开、缺 "platforms" key 或解析到 0 条平台均返回 false。
bool loadPlatformAxes(const std::string& path, std::vector<PlatformAxis>& out);

#endif  // PLATFORM_POSE_IO_H
