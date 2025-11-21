#ifndef CALIBRATION_DATA_IO_H
#define CALIBRATION_DATA_IO_H
#include <Eigen/Dense>
#include <cereal/archives/json.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cmath>
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

#include "plog/Log.h"
struct Pose {
    Eigen::Matrix3d R;  // 旋转矩阵
    Eigen::Vector3d t;  // 平移向量
    double reprojErr;   // 重投影误差
};
struct PoseVec {
    Eigen::Vector3d R;  // 旋转矩阵
    Eigen::Vector3d t;  // 平移向量
    double reprojErr;   // 重投影误差
};
namespace cereal {
// =====================================================
// Eigen Matrix JSON 序列化
// =====================================================
template <class Archive, class _Scalar, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
inline void save(Archive& ar, const Eigen::Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols>& m) {
    int rows = m.rows();
    int cols = m.cols();
    std::vector<_Scalar> vals(rows * cols);
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) vals[r * cols + c] = m(r, c);

    ar(CEREAL_NVP(rows), CEREAL_NVP(cols), cereal::make_nvp("val", vals));
}

template <class Archive, class _Scalar, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
inline void load(Archive& ar, Eigen::Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols>& m) {
    int rows, cols;
    std::vector<_Scalar> vals;
    ar(CEREAL_NVP(rows), CEREAL_NVP(cols), cereal::make_nvp("val", vals));
    m.resize(rows, cols);

    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) m(r, c) = vals[r * cols + c];
}
// Eigen::Vector3d JSON 序列化
template <class Archive>
void save(Archive& ar, const Eigen::Vector3d& v) {
    ar(v(0), v(1), v(2));
}

template <class Archive>
void load(Archive& ar, Eigen::Vector3d& v) {
    ar(v(0), v(1), v(2));
}
}  // namespace cereal

// =====================================================
// CalibrationData 类
// =====================================================
class CalibrationData {
public:
    double m = 0.0;
    double dx = 0.0;
    double dy = 0.0;
    double u0 = 0.0;
    double v0 = 0.0;

    Eigen::Matrix3d K{};
    Eigen::Matrix<double, 1, 5> coff_dis{};
    std::vector<Eigen::Vector3d> v_rot;
    std::vector<Eigen::Vector3d> v_trans;

    template <class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(m), CEREAL_NVP(dx), CEREAL_NVP(dy), CEREAL_NVP(u0), CEREAL_NVP(v0), CEREAL_NVP(K), CEREAL_NVP(coff_dis),
           CEREAL_NVP(v_rot), CEREAL_NVP(v_trans));
    }

    bool save(const std::string& path) const {
        std::ofstream os(path);
        if (!os.is_open()) {
            PLOGE << "[Error] Cannot open file for writing: " << path;
            return false;
        }
        cereal::JSONOutputArchive archive(os);
        archive(*this);
        PLOGD << "[OK] Calibration data saved: " << path;
        return true;
    }

    bool load(const std::string& path) {
        std::ifstream is(path);
        if (!is.is_open()) {
            PLOGE << "[Error] Cannot open file for reading: " << path;
            return false;
        }
        cereal::JSONInputArchive archive(is);
        archive(*this);
        PLOGD << "[OK] Calibration data loaded: " << path;
        return true;
    }
};

#endif  // CALIBRATION_DATA_IO_H
