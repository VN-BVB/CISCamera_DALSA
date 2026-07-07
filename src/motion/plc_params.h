#pragma once

#include "cereal/archives/json.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/array.hpp"

namespace plc
{

// 对位平台参数
struct pltParams
{

    double speedBack = -1;  // 回参考点速度运动速度
    double speedMoveR = 10; // 回参考点相对运动速度
    double posMoveR = 10;  // 回参考点相对运动位置


    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(CEREAL_NVP(speedBack),
                CEREAL_NVP(speedMoveR),
                CEREAL_NVP(posMoveR));
    }
};

    // PLC硬件参数
    struct plcParams
    {
        std::string ip = "192.168.6.6"; // 远程连接地址
        int port = 502;                 // 远程连接端口

        int32_t pulseCntMax; // 编码器的最大计数值
        int pulseDiv;        // 编码器的脉冲分频值


        pltParams plt0;   // 对位平台0
        pltParams plt1;   // 对位平台1
        pltParams plt2;   // 对位平台2
        pltParams plt3;   // 对位平台3
        pltParams plt4;   // 对位平台4
        pltParams plt5;   // 对位平台5
        pltParams plt6;   // 对位平台6
        pltParams plt7;   // 对位平台7

        template <class Archive>
        void serialize(Archive &archive)
        {
            archive(CEREAL_NVP(ip),
                    CEREAL_NVP(port),
                    CEREAL_NVP(pulseCntMax),
                    CEREAL_NVP(pulseDiv),
                    CEREAL_NVP(plt0),
                    CEREAL_NVP(plt1),
                    CEREAL_NVP(plt2),
                    CEREAL_NVP(plt3),
                    CEREAL_NVP(plt4),
                    CEREAL_NVP(plt5),
                    CEREAL_NVP(plt6));
        }
    };

}
