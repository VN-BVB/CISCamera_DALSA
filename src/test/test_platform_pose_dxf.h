#ifndef TEST_PLATFORM_POSE_DXF_H
#define TEST_PLATFORM_POSE_DXF_H

// 测试：将 data/calibration_config/platform_pose.json 中所有对位平台坐标系
// （每组为两个轴方向向量 X/Y 和原点 T）画到同一个 DXF 文件中。

class TestPlatformPoseDxf {
public:
    TestPlatformPoseDxf();
    // 走 PlatformPoseData::loadCompact 路径（世界→相机坐标变换后），输出相机坐标系下的平台 triad。
    void run();
    // 直接解析 JSON 中的 X/Y/T（世界坐标系），不做坐标变换。
    void runDirect();
};

#endif  // TEST_PLATFORM_POSE_DXF_H
