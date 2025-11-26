#ifndef PY_TELECENTRIC_OPTIMIZER_H
#define PY_TELECENTRIC_OPTIMIZER_H
#include <pybind11/embed.h>
#include <pybind11/numpy.h>

#include "src/config/calibration_data_io.h"
#include "src/telecentricLineCalibrator/telecentric_line_calibrator.h"
class TelecentricPYOptimizer {
public:
    TelecentricPYOptimizer() = default;
    // TelecentricPYOptimizer();

    bool invokeTelecentricCalibration();
    bool optTelecentricExtrinsicParameters(const Eigen::Matrix3d& K, const Eigen::Matrix<double, 1, 5>& coff_dis,
                                           const std::vector<Eigen::Vector2d> imgPts,
                                           const std::vector<Eigen::Vector2d>& worldPts, Eigen::Vector3d& v_rot,
                                           Eigen::Vector3d& v_trans, double& err);
};

#endif  // PY_TELECENTRIC_OPTIMIZER_H
