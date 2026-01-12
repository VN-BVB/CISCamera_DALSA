#ifndef PY_TELECENTRIC_OPTIMIZER_H
#define PY_TELECENTRIC_OPTIMIZER_H
#include <pybind11/embed.h>
#include <pybind11/iostream.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include "src/config/calibration_data_io.h"
#include "src/telecentricLineCalibrator/telecentric_line_calibrator.h"

class TelecentricPYOptimizer {
public:
    TelecentricPYOptimizer();

    bool invokeTelecentricCalibration();

    bool optTelecentricExtrinsicParameters(const Eigen::Matrix3d& K, const Eigen::Matrix<double, 1, 5>& coff_dis,
                                           const std::vector<Eigen::Vector2d>& imgPts,
                                           const std::vector<Eigen::Vector2d>& worldPts, Eigen::Vector3d& v_rot,
                                           Eigen::Vector3d& v_trans, double& err);
    bool refinePlatformExtrinsicsLM(const Eigen::Matrix3d& K, const Eigen::Matrix<double, 1, 5>& D,
                                    const std::vector<std::vector<Eigen::Vector2d>>& pts, const Eigen::Vector3d& rvec_init,
                                    const Eigen::Vector3d& tvec_init, double dx, double dy, double ang_deg,
                                    Eigen::Vector3d& rvec_opt, Eigen::Vector3d& tvec_opt, double& final_rms);

private:
    static bool pythonInitialized;
};

#endif  // PY_TELECENTRIC_OPTIMIZER_H
