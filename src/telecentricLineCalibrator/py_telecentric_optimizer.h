#ifndef PY_TELECENTRIC_OPTIMIZER_H
#define PY_TELECENTRIC_OPTIMIZER_H
#include <pybind11/embed.h>

#include <iostream>

#include "plog/Log.h"
class TelecentricPYOptimizer {
public:
    TelecentricPYOptimizer() = default;
    bool invokeTelecentricCalibration() {
        PLOGD << "正在进行非线性优化";
        _putenv("PYTHONHOME=D:\\anaconda\\envs\\Telecentric-Calibration");
        _putenv(
            "PYTHONPATH=D:\\anaconda\\envs\\Telecentric-Calibration\\Lib;"
            "D:\\anaconda\\envs\\Telecentric-Calibration\\Lib\\site-packages;"
            ".\\src\\telecentricLineCalibrator\\python\\Telecentric-Calibration-main");

        try {
            pybind11::scoped_interpreter guard{};

            pybind11::exec(R"(
            import sys
            print('Python from:', sys.executable)
            print('sys.path =', sys.path)
        )");

            std::string scriptPath =
                R"(D:\Code\CISCamera_DALSA\src\telecentricLineCalibrator\python\Telecentric-Calibration-main\load.py)";

            pybind11::eval_file(scriptPath);
            return true;

        } catch (pybind11::error_already_set &e) {
            std::cerr << "❌ Python 执行错误:\n" << std::endl;
            return false;
        }
    }
};

#endif  // PY_TELECENTRIC_OPTIMIZER_H
