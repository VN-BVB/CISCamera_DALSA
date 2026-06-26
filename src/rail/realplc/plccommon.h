#pragma once

#include <qmetatype.h>

#include "cereal/archives/json.hpp"
#include "cereal/types/array.hpp"
#include "cereal/types/vector.hpp"
#include "plc_params.h"

// 用于配置PLC参数
struct plcCtrlParams {
    std::string name = u8"PLC模块";  // 模块名称

    // 运行开关
    bool realConveyEnabled = true;    // 选择启用实体输送线
    bool realExAxis1Enabled = true;   // 是否启用实体外部轴1
    bool realExAxis2Enabled = false;  // 是否启用实体外部轴2
    bool sprayGun1Enabled = true;     // 是否启用喷枪1
    bool sprayGun2Enabled = true;     // 是否启用喷枪2

    // PLC硬件参数
    plc::plcParams plc;  // PLC硬件参数

    // 编码器参数(标定)
    double pulseDis;  // 编码器每个脉冲输送链的移动距离
    // 采图工位参数(标定)
    double capturePosition;  // 采图开启位置
    double captureOnDiff;    // 采图开启距离偏差值
    double captureOffDiff;   // 采图关闭距离偏移值
    // 喷涂工位参数(标定)
    int mscNumMax = 10;  // 多工位控制的最大可处理的工件数
    double mscPos1;      // 多工位控制的工位1的位置
    double mscPos1Diff;  // 多工位控制的工位1的位置偏差值
    double mscPos2;      // 多工位控制的工位2的位置
    double mscPos2Diff;  // 多工位控制的工位2的位置偏差值

    template <class Archive>
    void serialize(Archive &archive) {
        archive(CEREAL_NVP(name), CEREAL_NVP(realConveyEnabled), CEREAL_NVP(realExAxis1Enabled), CEREAL_NVP(realExAxis2Enabled),
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
