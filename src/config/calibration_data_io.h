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
#include <sstream>
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
// =======================
//     平台位姿保存类
// =======================
class PlatformPoseData {
public:
    std::vector<Eigen::Vector3d> allRotVecs;    // 所有平台的旋转向量
    std::vector<Eigen::Vector3d> allTransVecs;  // 所有平台的平移向量

    // cereal 支持的序列化函数
    template <class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(allRotVecs), CEREAL_NVP(allTransVecs));
    }

    // 保存到 JSON 文件
    bool save(const std::string& path) {
        try {
            std::ofstream os(path);
            cereal::JSONOutputArchive archive(os);
            archive(cereal::make_nvp("PlatformPoseData", *this));
            return true;
        } catch (...) {
            return false;
        }
    }

    // 保存为世界坐标系（Rodrigues + 平移），跳过无效和最后一个（世界外参）
    bool saveCompact(const std::string& path, const Eigen::Vector3d& worldRvecBack, const Eigen::Vector3d& worldTvecBack) {
        try {
            cv::Mat r_wc(3, 1, CV_64F), R_wc(3, 3, CV_64F);
            for (int i = 0; i < 3; ++i) r_wc.at<double>(i) = worldRvecBack(i);
            cv::Rodrigues(r_wc, R_wc);
            cv::Mat R_cw = R_wc.t();
            cv::Mat t_wc(3, 1, CV_64F);
            for (int i = 0; i < 3; ++i) t_wc.at<double>(i) = worldTvecBack(i);

            std::ofstream os(path);
            os << std::setprecision(12);
            os << "{\"PlatformPoseData\":{\"platforms\":[\n";
            bool first = true;
            for (size_t i = 0; i + 1 < allRotVecs.size(); ++i) {  // 跳过最后一个(世界外参)
                if (allTransVecs[i].norm() < 1e-6) continue;       // 跳过未标定

                cv::Mat r_pc(3, 1, CV_64F), R_pc(3, 3, CV_64F);
                for (int j = 0; j < 3; ++j) r_pc.at<double>(j) = allRotVecs[i](j);
                cv::Rodrigues(r_pc, R_pc);
                cv::Mat t_pc(3, 1, CV_64F);
                for (int j = 0; j < 3; ++j) t_pc.at<double>(j) = allTransVecs[i](j);

                // 平台→相机 → 平台→世界
                cv::Mat R_pw = R_cw * R_pc;
                cv::Mat t_pw = R_cw * (t_pc - t_wc);
                t_pw.at<double>(2) = 0;  // CIS 无深度，强制 z=0

                if (!first) os << ",\n";
                first = false;
                os << "  {\"id\":" << i;
                // X 方向 (R_pw 第一列)
                os << ",\"X\":[" << R_pw.at<double>(0,0) << "," << R_pw.at<double>(1,0) << "," << R_pw.at<double>(2,0) << "]";
                // Y 方向 (R_pw 第二列)
                os << ",\"Y\":[" << R_pw.at<double>(0,1) << "," << R_pw.at<double>(1,1) << "," << R_pw.at<double>(2,1) << "]";
                os << ",\"T\":[" << t_pw.at<double>(0) << "," << t_pw.at<double>(1) << "," << t_pw.at<double>(2) << "]}";
            }
            // 写入世界→相机外参（orignCor），加载时用于转回相机坐标
            os << "\n],\"worldPose\":{";
            os << "\"R\":[" << worldRvecBack(0) << "," << worldRvecBack(1) << "," << worldRvecBack(2) << "]";
            os << ",\"T\":[" << worldTvecBack(0) << "," << worldTvecBack(1) << "," << worldTvecBack(2) << "]";
            os << "}}}\n";
            return true;
        } catch (...) { return false; }
    }

    // 从新格式加载：世界坐标 → 相机坐标，存回 allRotVecs/allTransVecs
    bool loadCompact(const std::string& path) {
        try {
            std::ifstream is(path);
            std::string json((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
            allRotVecs.clear();
            allTransVecs.clear();

            // 1. 先读 worldPose（世界→相机外参）
            auto extractVec = [&](const std::string& key, size_t from) -> Eigen::Vector3d {
                size_t p = json.find("\"" + key + "\":[", from);
                if (p == std::string::npos) return Eigen::Vector3d::Zero();
                p = json.find('[', p) + 1;
                size_t q = json.find(']', p);
                double v0, v1, v2;
                sscanf(json.substr(p, q - p).c_str(), "%lf,%lf,%lf", &v0, &v1, &v2);
                return Eigen::Vector3d(v0, v1, v2);
            };
            size_t wp = json.find("\"worldPose\":{");
            Eigen::Vector3d wRvec = extractVec("R", wp);
            Eigen::Vector3d wTvec = extractVec("T", wp);

            cv::Mat r_wc(3, 1, CV_64F), R_wc(3, 3, CV_64F);
            for (int i = 0; i < 3; ++i) r_wc.at<double>(i) = wRvec(i);
            cv::Rodrigues(r_wc, R_wc);
            cv::Mat t_wc(3, 1, CV_64F);
            for (int i = 0; i < 3; ++i) t_wc.at<double>(i) = wTvec(i);

            // 2. 读平台列表，转回相机坐标（按 id 放到正确位置）
            size_t pos = json.find("\"platforms\":[");
            pos = json.find('[', pos) + 1;
            size_t maxId = 0;
            std::vector<std::pair<size_t, std::pair<Eigen::Vector3d, Eigen::Vector3d>>> entries;
            while (true) {
                size_t start = json.find('{', pos);
                if (start == std::string::npos || start >= json.find(']', pos)) break;
                size_t end = json.find('}', start) + 1;
                std::string entry = json.substr(start, end - start);

                // 解析 id
                size_t pid = 0;
                {
                    size_t p = entry.find("\"id\":");
                    if (p != std::string::npos) pid = atoi(entry.c_str() + p + 5);
                }

                Eigen::Vector3d X_pw = Eigen::Vector3d::Zero();
                Eigen::Vector3d Y_pw = Eigen::Vector3d::Zero();
                Eigen::Vector3d t_pw = Eigen::Vector3d::Zero();
                {
                    size_t px = entry.find("\"X\":[");
                    if (px != std::string::npos) {
                        px = entry.find('[', px) + 1;
                        size_t qx = entry.find(']', px);
                        sscanf(entry.substr(px, qx - px).c_str(), "%lf,%lf,%lf", &X_pw(0), &X_pw(1), &X_pw(2));
                    }
                    size_t py = entry.find("\"Y\":[");
                    if (py != std::string::npos) {
                        py = entry.find('[', py) + 1;
                        size_t qy = entry.find(']', py);
                        sscanf(entry.substr(py, qy - py).c_str(), "%lf,%lf,%lf", &Y_pw(0), &Y_pw(1), &Y_pw(2));
                    }
                    size_t p = entry.find("\"T\":[");
                    if (p != std::string::npos) {
                        p = entry.find('[', p) + 1;
                        size_t q = entry.find(']', p);
                        sscanf(entry.substr(p, q - p).c_str(), "%lf,%lf,%lf", &t_pw(0), &t_pw(1), &t_pw(2));
                    }
                }

                // 世界→相机: 从 X/Y 拼旋转矩阵
                Eigen::Vector3d Z_pw = X_pw.cross(Y_pw);
                if (Z_pw.norm() > 1e-12) Z_pw.normalize();
                cv::Mat R_pw_cv(3, 3, CV_64F);
                for (int r = 0; r < 3; ++r) {
                    R_pw_cv.at<double>(r, 0) = X_pw(r);
                    R_pw_cv.at<double>(r, 1) = Y_pw(r);
                    R_pw_cv.at<double>(r, 2) = Z_pw(r);
                }
                cv::Mat t_pw_cv(3, 1, CV_64F);
                for (int j = 0; j < 3; ++j) t_pw_cv.at<double>(j) = t_pw(j);

                cv::Mat R_pc = R_wc * R_pw_cv;
                cv::Mat t_pc = R_wc * t_pw_cv + t_wc;
                cv::Mat r_pc;
                cv::Rodrigues(R_pc, r_pc);

                if (pid > maxId) maxId = pid;
                entries.push_back({pid,
                    {Eigen::Vector3d(r_pc.at<double>(0), r_pc.at<double>(1), r_pc.at<double>(2)),
                     Eigen::Vector3d(t_pc.at<double>(0), t_pc.at<double>(1), t_pc.at<double>(2))}});

                pos = end + 1;
            }
            // 按 id 填充到正确位置
            allRotVecs.resize(maxId + 1, Eigen::Vector3d(0, 0, 0));
            allTransVecs.resize(maxId + 1, Eigen::Vector3d(0, 0, 0));
            for (auto& e : entries) {
                allRotVecs[e.first] = e.second.first;
                allTransVecs[e.first] = e.second.second;
            }
            // 最后补上 worldPose（兼容 .back() 用法）
            allRotVecs.push_back(wRvec);
            allTransVecs.push_back(wTvec);
            return true;
        } catch (...) { return false; }
    }

    // 从 JSON 文件加载
    bool load(const std::string& path) {
        try {
            std::ifstream is(path);
            cereal::JSONInputArchive archive(is);
            archive(cereal::make_nvp("PlatformPoseData", *this));
            return true;
        } catch (...) {
            return false;
        }
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
    std::cout << "读取 " << path << " 成功，共 " << pts.size() << " 个点\n";
    return true;
}

#endif  // CALIBRATION_DATA_IO_H
