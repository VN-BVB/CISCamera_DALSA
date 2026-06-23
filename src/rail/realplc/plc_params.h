#pragma once

#include "cereal/archives/json.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/array.hpp"

namespace plc
{

// 外部轴参数
struct axisParams
{
    double minMotionRange = 0;    // 外部轴负极限位置
    double maxMotionRange = 1500; // 外部轴正极限位置

    double speedMax = 1000; // 最大速度
    double accMax = 3000;   // 最大加速度
    double jerkMax = 10000; // 最大加加速度

    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(CEREAL_NVP(minMotionRange),
                CEREAL_NVP(maxMotionRange),
                CEREAL_NVP(speedMax),
                CEREAL_NVP(accMax),
                CEREAL_NVP(jerkMax));
    }
};

    // PLC硬件参数
    struct plcParams
    {
        std::string ip = "192.168.6.6"; // 远程连接地址
        int port = 502;                 // 远程连接端口

        int32_t pulseCntMax; // 编码器的最大计数值
        int pulseDiv;        // 编码器的脉冲分频值

        axisParams conveyor1; // 实验室输送线
        axisParams conveyor2; // 现场输送线
        axisParams exAxis1;   // 外部轴1
        axisParams exAxis2;   // 外部轴2

        template <class Archive>
        void serialize(Archive &archive)
        {
            archive(CEREAL_NVP(ip),
                    CEREAL_NVP(port),
                    CEREAL_NVP(pulseCntMax),
                    CEREAL_NVP(pulseDiv),
                    CEREAL_NVP(conveyor1),
                    CEREAL_NVP(conveyor2),
                    CEREAL_NVP(exAxis1),
                    CEREAL_NVP(exAxis2));
        }
    };

}
