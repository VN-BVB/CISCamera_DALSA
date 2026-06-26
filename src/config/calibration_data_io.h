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

    // 保存（纯序列化，无参数时用 .back() 作为 worldPose）
    bool save(const std::string& path,
              const Eigen::Vector3d& worldRvecBack = Eigen::Vector3d::Zero(),
              const Eigen::Vector3d& worldTvecBack = Eigen::Vector3d::Zero()) {
        auto wR = worldRvecBack, wT = worldTvecBack;
        if (wR.norm() < 1e-12 && wT.norm() < 1e-12 && !allRotVecs.empty()) {
            wR = allRotVecs.back(); wT = allTransVecs.back();
        }
        try {
            std::ofstream os(path);
            os << std::setprecision(12);
            os << "{\"PlatformPoseData\":{\"platforms\":[\n";
            bool first = true;
            for (size_t i = 0; i + 1 < allRotVecs.size(); ++i) {
                if (allTransVecs[i].norm() < 1e-6) continue;
                cv::Mat rv(3, 1, CV_64F), R(3, 3, CV_64F);
                for (int j = 0; j < 3; ++j) rv.at<double>(j) = allRotVecs[i](j);
                cv::Rodrigues(rv, R);
                if (!first) os << ",\n";
                first = false;
                os << "  {\"id\":" << i;
                os << ",\"X\":[" << R.at<double>(0,0) << "," << R.at<double>(1,0) << "," << R.at<double>(2,0) << "]";
                os << ",\"Y\":[" << R.at<double>(0,1) << "," << R.at<double>(1,1) << "," << R.at<double>(2,1) << "]";
                os << ",\"T\":[" << allTransVecs[i](0) << "," << allTransVecs[i](1) << "," << allTransVecs[i](2) << "]}";
            }
            os << "\n],\"worldPose\":{";
            os << "\"R\":[" << worldRvecBack(0) << "," << worldRvecBack(1) << "," << worldRvecBack(2) << "]";
            os << ",\"T\":[" << worldTvecBack(0) << "," << worldTvecBack(1) << "," << worldTvecBack(2) << "]";
            os << "}}}\n";
            return true;
        } catch (...) { return false; }
    }

    // 加载（纯反序列化，无坐标转换）
    bool load(const std::string& path) {
        try {
            std::ifstream is(path);
            std::string json((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
            allRotVecs.clear();
            allTransVecs.clear();

            // 读 worldPose
            auto extractVec = [&](const std::string& key, size_t from) -> Eigen::Vector3d {
                size_t p = json.find("\"" + key + "\":[", from);
                if (p == std::string::npos) return Eigen::Vector3d::Zero();
                p = json.find('[', p) + 1;
                size_t q = json.find(']', p);
                Eigen::Vector3d v;
                sscanf(json.substr(p, q - p).c_str(), "%lf,%lf,%lf", &v(0), &v(1), &v(2));
                return v;
            };
            size_t wp = json.find("\"worldPose\":{");
            Eigen::Vector3d wRvec = extractVec("R", wp);
            Eigen::Vector3d wTvec = extractVec("T", wp);

            // 读平台列表
            size_t pos = json.find("\"platforms\":[");
            if (pos == std::string::npos) { allRotVecs.push_back(wRvec); allTransVecs.push_back(wTvec); return true; }
            pos = json.find('[', pos) + 1;
            size_t maxId = 0;
            struct Entry { size_t id; Eigen::Vector3d rvec, tvec; };
            std::vector<Entry> entries;

            while (true) {
                size_t start = json.find('{', pos);
                if (start == std::string::npos || start >= json.find(']', pos)) break;
                size_t end = json.find('}', start) + 1;
                std::string entry = json.substr(start, end - start);

                size_t pid = 0; {size_t p = entry.find("\"id\":"); if(p!=std::string::npos) pid=atoi(entry.c_str()+p+5);}
                Eigen::Vector3d Xw = Eigen::Vector3d::Zero(), Yw = Eigen::Vector3d::Zero(), Tw = Eigen::Vector3d::Zero();
                {size_t p=entry.find("\"X\":[");if(p!=std::string::npos){p=entry.find('[',p)+1;size_t q=entry.find(']',p);sscanf(entry.substr(p,q-p).c_str(),"%lf,%lf,%lf",&Xw(0),&Xw(1),&Xw(2));}}
                {size_t p=entry.find("\"Y\":[");if(p!=std::string::npos){p=entry.find('[',p)+1;size_t q=entry.find(']',p);sscanf(entry.substr(p,q-p).c_str(),"%lf,%lf,%lf",&Yw(0),&Yw(1),&Yw(2));}}
                {size_t p=entry.find("\"T\":[");if(p!=std::string::npos){p=entry.find('[',p)+1;size_t q=entry.find(']',p);sscanf(entry.substr(p,q-p).c_str(),"%lf,%lf,%lf",&Tw(0),&Tw(1),&Tw(2));}}

                Eigen::Vector3d Zw = Xw.cross(Yw); if(Zw.norm()>1e-12) Zw.normalize();
                cv::Mat R(3,3,CV_64F);
                for(int r=0;r<3;++r){R.at<double>(r,0)=Xw(r);R.at<double>(r,1)=Yw(r);R.at<double>(r,2)=Zw(r);}
                cv::Mat rv; cv::Rodrigues(R, rv);

                if (pid > maxId) maxId = pid;
                entries.push_back({pid,
                    Eigen::Vector3d(rv.at<double>(0),rv.at<double>(1),rv.at<double>(2)), Tw});
                pos = end + 1;
            }
            allRotVecs.resize(maxId + 1, Eigen::Vector3d(0, 0, 0));
            allTransVecs.resize(maxId + 1, Eigen::Vector3d(0, 0, 0));
            for (auto& e : entries) { allRotVecs[e.id] = e.rvec; allTransVecs[e.id] = e.tvec; }
            allRotVecs.push_back(wRvec);
            allTransVecs.push_back(wTvec);
            return true;
        } catch (...) { return false; }
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
