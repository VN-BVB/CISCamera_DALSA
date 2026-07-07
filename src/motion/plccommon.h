#pragma once

#include <qmetatype.h>

#include "cereal/archives/json.hpp"
#include "cereal/types/array.hpp"
#include "cereal/types/vector.hpp"
#include "plc_params.h"

// 用于配置PLC参数
struct plcCtrlParams {
    std::string name = u8"PLC模块";  // 模块名称

    // PLC硬件参数
    plc::plcParams plc;  // PLC硬件参数

    template <class Archive>
    void serialize(Archive &archive) {
        archive(CEREAL_NVP(name), CEREAL_NVP(realExAxis2Enabled),
                CEREAL_NVP(sprayGun1Enabled), CEREAL_NVP(sprayGun2Enabled), CEREAL_NVP(pulseDis), CEREAL_NVP(capturePosition),
                CEREAL_NVP(captureOnDiff), CEREAL_NVP(captureOffDiff), CEREAL_NVP(mscNumMax), CEREAL_NVP(mscPos1), CEREAL_NVP(mscPos1Diff),
                CEREAL_NVP(mscPos2), CEREAL_NVP(mscPos2Diff));
    }
};

// 用于PLC状态反馈
struct plcFdbkParams {
    int connectStatus;                 // PLC连接状态
    int runStatus;                     // PLC运行状态
    int encoderCnt;                    // 输送链编码器计数值
    int captureTrigger;                // 采图区域光幕信号
    int sprayTrigger;                  // 喷涂区域光幕信号
    int captureStatus;                 // PLC采图状态
    double captureDis;                 // 触发光幕1后的移动距离
    double captureLength;              // 工件长度
    int workpieceNum;                  // 最后扫描的工件ID
    std::vector<double> workpiecePos;  // 每个工件的输送链位置mm
    int conveyorEnale;                 // 输送链使能状态
    double conveyorVel;                // 输送链速度mm/s
    int exAxis1Enable;                 // 外部轴1使能状态
    double exAxis1Pos;                 // 外部轴1实际位置mm
    double exAxis1Vel;                 // 外部轴1实际速度mm/s
    int exAxis2Enable;                 // 外部轴2使能状态
    double exAxis2Pos;                 // 外部轴2实际位置mm
    double exAxis2Vel;                 // 外部轴2实际速度mm/s
};
Q_DECLARE_METATYPE(plcFdbkParams)
