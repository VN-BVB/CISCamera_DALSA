#pragma once

#include <qmetatype.h>

#include "cereal/archives/json.hpp"
#include "cereal/types/array.hpp"
#include "cereal/types/vector.hpp"
#include "plc_params.h"

// PLC 控制参数（json 序列化用）
struct plcCtrlParams {
    std::string name = u8"PLC模块";
    plc::plcParams plc;

    template<class Archive>
    void serialize(Archive &archive) {
        archive(CEREAL_NVP(name), CEREAL_NVP(plc));
    }
};

// 单轴状态反馈
struct axisFdbk {
    int    enable    = 0;
    double pos       = 0.0;
    double vel       = 0.0;
    int    moving    = 0;
    int    homed     = 0;
    int    alarm     = 0;
};

// PLC 状态反馈（1 地轨 + 7 平台 = 8 单元）
struct plcFdbkParams {
    int connectStatus = 0;
    int runStatus     = 0;

    axisFdbk rail;                      // 地轨
    struct {
        axisFdbk x, y, r;              // 每个平台 3 轴
    } plt[7];
};

Q_DECLARE_METATYPE(plcFdbkParams)
