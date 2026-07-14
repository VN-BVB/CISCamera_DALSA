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
#include <sstream>
#include <vector>

#include "plog/Log.h"
#include "src/utils/plog_utils.h"
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
        ar(CEREAL_NVP(m), CEREAL_NVP(dx), CEREAL_NVP(dy), CEREAL_NVP(u0), CEREAL_NVP(v0), CEREAL_NVP(K), CEREAL_NVP(coff_dis), CEREAL_NVP(v_rot),
           CEREAL_NVP(v_trans));
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
// =======================
//   旧 JSON 格式辅助结构（cereal 匹配 worldPose + platforms[]）
// =======================
struct PlatformPoseWorldPose {
    Eigen::Vector3d R, T;
    template <class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(R), CEREAL_NVP(T));
    }
};
struct PlatformPosePlatform {
    int id;
    Eigen::Vector3d X, Y, T;
    template <class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(id), CEREAL_NVP(X), CEREAL_NVP(Y), CEREAL_NVP(T));
    }
};
struct PlatformPoseFile {
    PlatformPoseWorldPose worldPose;
    std::vector<PlatformPosePlatform> platforms;
    template <class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(worldPose), CEREAL_NVP(platforms));
    }
};

// =======================
//     平台位姿保存类
// =======================
class PlatformPoseData {
public:
    std::vector<Eigen::Vector3d> allRotVecs;    // 所有平台的旋转向量
    std::vector<Eigen::Vector3d> allTransVecs;  // 所有平台的平移向量

    bool save(const std::string& path, const Eigen::Vector3d& worldRvecBack = Eigen::Vector3d::Zero(),
              const Eigen::Vector3d& worldTvecBack = Eigen::Vector3d::Zero()) {
        auto wR = worldRvecBack, wT = worldTvecBack;
        if (wR.norm() < 1e-12 && wT.norm() < 1e-12 && !allRotVecs.empty()) {
            wR = allRotVecs.back();
            wT = allTransVecs.back();
        }
        if (!allRotVecs.empty()) {
            allRotVecs.back() = wR;
            allTransVecs.back() = wT;
        }

        PlatformPoseFile file;
        file.worldPose = {wR, wT};
        for (size_t i = 0; i + 1 < allRotVecs.size(); ++i) {
            cv::Mat rv(3, 1, CV_64F);
            rv.at<double>(0) = allRotVecs[i](0);
            rv.at<double>(1) = allRotVecs[i](1);
            rv.at<double>(2) = allRotVecs[i](2);
            cv::Mat Rmat;
            cv::Rodrigues(rv, Rmat);
            file.platforms.push_back({int(i), Eigen::Vector3d(Rmat.at<double>(0, 0), Rmat.at<double>(1, 0), Rmat.at<double>(2, 0)),
                                      Eigen::Vector3d(Rmat.at<double>(0, 1), Rmat.at<double>(1, 1), Rmat.at<double>(2, 1)), allTransVecs[i]});
        }

        std::ofstream os(path);
        if (!os.is_open()) {
            PLOGE << "[Error] Cannot open file for writing: " << path;
            return false;
        }
        {
            cereal::JSONOutputArchive archive(os);
            archive(file);
        }
        PLOGD << "[OK] Calibration data saved: " << path;
        return true;
    }

    bool load(const std::string& path) {
        std::ifstream is(path);
        if (!is.is_open()) {
            PLOGE << "[Error] Cannot open file for reading: " << path;
            return false;
        }
        PlatformPoseFile file;
        {
            cereal::JSONInputArchive archive(is);
            archive(file);
        }

        allRotVecs.clear();
        allTransVecs.clear();
        for (auto& p : file.platforms) {
            cv::Mat Rmat(3, 3, CV_64F);
            Rmat.at<double>(0, 0) = p.X(0);
            Rmat.at<double>(0, 1) = p.Y(0);
            Rmat.at<double>(1, 0) = p.X(1);
            Rmat.at<double>(1, 1) = p.Y(1);
            Rmat.at<double>(2, 0) = p.X(2);
            Rmat.at<double>(2, 1) = p.Y(2);
            Eigen::Vector3d Z = p.X.cross(p.Y);
            Rmat.at<double>(0, 2) = Z(0);
            Rmat.at<double>(1, 2) = Z(1);
            Rmat.at<double>(2, 2) = Z(2);
            cv::Mat rvec;
            cv::Rodrigues(Rmat, rvec);
            allRotVecs.push_back(Eigen::Vector3d(rvec.at<double>(0), rvec.at<double>(1), rvec.at<double>(2)));
            allTransVecs.push_back(p.T);
        }
        allRotVecs.push_back(file.worldPose.R);
        allTransVecs.push_back(file.worldPose.T);

        PLOGD << "[OK] Calibration data loaded: " << path;
        return true;
    }
};
inline bool readPointsFromTxt(const std::string& path, std::vector<Eigen::Vector2d>& pts) {
    std::ifstream fin(path);
    if (!fin.is_open()) {
        std::cerr << "无法打开文件: " << path << std::endl;
        return false;
    }

    std::string line;
    pts.clear();
    // 跳过第一行标题
    std::getline(fin, line);

    double idx, x, y;
    while (fin >> idx >> x >> y) {
        pts.emplace_back(x, y);
    }

    if (pts.empty()) {
        std::cerr << "文件 " << path << " 无有效点。" << std::endl;
        return false;
    }
    PLOG_INFO << "读取 " << path << " 成功，共 " << pts.size() << " 个点\n";
    return true;
}

#endif  // CALIBRATION_DATA_IO_H
