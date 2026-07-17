#pragma once

#include "cereal/archives/json.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/array.hpp"

namespace plc {

// 单轴运动参数
struct axisMotionParams {
    double pos   = 0.0;       // 目标位置
    double vel   = 100.0;     // 速度
    double acc   = 1000.0;    // 加速度
    double dec   = 1000.0;    // 减速度
    double jerk  = 10000.0;   // 加加速度

    template<class Archive>
    void serialize(Archive &archive) {
        archive(CEREAL_NVP(pos), CEREAL_NVP(vel), CEREAL_NVP(acc),
                CEREAL_NVP(dec), CEREAL_NVP(jerk));
    }
};

// 对位平台参数
struct pltParams {
    double speedBack  = -1;     // 回参考点速度
    double speedMoveR = 10;     // 相对运动速度
    double posMoveR   = 10;     // 相对运动位置

    template<class Archive>
    void serialize(Archive &archive) {
        archive(CEREAL_NVP(speedBack),
                CEREAL_NVP(speedMoveR),
                CEREAL_NVP(posMoveR));
    }
};

// PLC 硬件参数
struct plcParams {
    std::string ip = "192.168.6.6";
    int port       = 502;
    int32_t pulseCntMax = 0;
    int pulseDiv        = 0;

    axisMotionParams rail;   // 地轨运动参数
    pltParams plt0, plt1, plt2, plt3, plt4, plt5, plt6;

    template<class Archive>
    void serialize(Archive &archive) {
        archive(CEREAL_NVP(ip), CEREAL_NVP(port),
                CEREAL_NVP(pulseCntMax), CEREAL_NVP(pulseDiv),
                CEREAL_NVP(rail),
                CEREAL_NVP(plt0), CEREAL_NVP(plt1), CEREAL_NVP(plt2),
                CEREAL_NVP(plt3), CEREAL_NVP(plt4), CEREAL_NVP(plt5),
                CEREAL_NVP(plt6));
    }
};

}  // namespace plc
