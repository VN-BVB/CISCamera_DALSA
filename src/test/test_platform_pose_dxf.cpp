#include "test_platform_pose_dxf.h"

#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include <Eigen/Dense>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/core/core.hpp>
#include <plog/Log.h>

#include "dl_dxf.h"
#include "src/config/calibration_data_io.h"

namespace {

// 轴线长度（mm）。T 数值大致在 0~500mm 范围，30mm 长足以可见又不会拥挤。
constexpr double kAxisLength = 30.0;
// 原点十字的半边长（mm）。
constexpr double kCrossHalf = 3.0;

struct PlatformAxis {
    int id = -1;
    Eigen::Vector3d X = Eigen::Vector3d::Zero();
    Eigen::Vector3d Y = Eigen::Vector3d::Zero();
    Eigen::Vector3d T = Eigen::Vector3d::Zero();
};

// 自 from 起查找 "key" : [a, b, c] 形式的三维向量。
// 容忍 key 与 '[' 之间的任意空白（实际 JSON 文件里写作 "X": [\n  0.04, ...\n]）。
Eigen::Vector3d extractVec(const std::string& json, const std::string& key, size_t from) {
    size_t keyPos = json.find("\"" + key + "\"", from);
    if (keyPos == std::string::npos) return Eigen::Vector3d::Zero();
    size_t lb = json.find('[', keyPos);
    if (lb == std::string::npos) return Eigen::Vector3d::Zero();
    size_t rb = json.find(']', lb);
    if (rb == std::string::npos) return Eigen::Vector3d::Zero();
    double v0 = 0.0, v1 = 0.0, v2 = 0.0;
    sscanf(json.substr(lb + 1, rb - lb - 1).c_str(), "%lf,%lf,%lf", &v0, &v1, &v2);
    return Eigen::Vector3d(v0, v1, v2);
}

// 直接读取 JSON 中存储的世界坐标 X/Y/T，不做 worldPose 的世界→相机变换。
bool parsePlatformPoseJson(const std::string& path, std::vector<PlatformAxis>& out) {
    std::ifstream is(path);
    if (!is.is_open()) {
        PLOG_ERROR << "Cannot open platform pose JSON: " << path;
        return false;
    }
    std::string json((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());

    size_t platformsKey = json.find("\"platforms\"");
    if (platformsKey == std::string::npos) {
        PLOG_ERROR << "Cannot find \"platforms\" key in " << path;
        return false;
    }
    size_t pos = json.find('[', platformsKey) + 1;

    while (true) {
        size_t start = json.find('{', pos);
        if (start == std::string::npos) break;
        size_t nextClose = json.find(']', pos);
        if (nextClose == std::string::npos || start >= nextClose) break;
        size_t end = json.find('}', start);
        if (end == std::string::npos) break;
        end += 1;

        PlatformAxis pa;
        size_t pidPos = json.find("\"id\"", start);
        if (pidPos != std::string::npos && pidPos < end) {
            size_t colon = json.find(':', pidPos);
            if (colon != std::string::npos && colon < end) {
                pa.id = atoi(json.c_str() + colon + 1);
            }
        }
        pa.X = extractVec(json, "X", start);
        pa.Y = extractVec(json, "Y", start);
        pa.T = extractVec(json, "T", start);

        out.push_back(pa);
        pos = end + 1;
    }

    if (out.empty()) {
        PLOG_ERROR << "No platforms parsed from " << path;
        return false;
    }
    return true;
}

void writeLine(DL_Dxf& dxf, DL_WriterA* dw, const DL_Attributes& attr,
               const Eigen::Vector3d& a, const Eigen::Vector3d& b) {
    DL_LineData lineData(a.x(), a.y(), 0.0, b.x(), b.y(), 0.0);
    dxf.writeLine(*dw, lineData, attr);
}

// 把解析出的平台数据写成 DXF：每个平台画 X 轴（红）、Y 轴（绿）和原点十字（黄）。
void writePlatformDxf(const std::string& outputPath, const std::vector<PlatformAxis>& platforms) {
    DL_Dxf dxf;
    DL_WriterA* dw = dxf.out(outputPath.c_str(), DL_Codes::AC1015);
    if (!dw || dw->openFailed()) {
        PLOG_ERROR << "Cannot create DXF: " << outputPath;
        return;
    }

    // ---- Header ----
    dxf.writeHeader(*dw);
    dw->sectionEnd();

    // ---- Tables ----
    dw->sectionTables();
    dxf.writeVPort(*dw);

    dw->tableLinetypes(1);
    dxf.writeLinetype(*dw, DL_LinetypeData("CONTINUOUS", "Continuous", 0, 0, 0.0));
    dxf.writeLinetype(*dw, DL_LinetypeData("BYLAYER", "", 0, 0, 0.0));
    dxf.writeLinetype(*dw, DL_LinetypeData("BYBLOCK", "", 0, 0, 0.0));
    dw->tableEnd();

    // 必须包含 "0" 系统层（AutoCAD 的 _DXFIN 严格要求，缺失会导致导入时报错并跳过
    // 所有实体），再加 X 轴红、Y 轴绿、原点黄。color24 = 0x00RRGGBB。
    dw->tableLayers(4);
    dxf.writeLayer(*dw, DL_LayerData("0", 0),
                   DL_Attributes("", 1, 0x00ffffff, 15, "CONTINUOUS"));
    dxf.writeLayer(*dw, DL_LayerData("PlatformX", 0),
                   DL_Attributes("", 1, 0x00ff0000, 15, "CONTINUOUS"));
    dxf.writeLayer(*dw, DL_LayerData("PlatformY", 0),
                   DL_Attributes("", 1, 0x0000ff00, 15, "CONTINUOUS"));
    dxf.writeLayer(*dw, DL_LayerData("PlatformOrigin", 0),
                   DL_Attributes("", 1, 0x00ffff00, 15, "CONTINUOUS"));
    dw->tableEnd();

    dw->tableStyle(1);
    DL_StyleData style("Standard", 0, 0.0, 1.0, 0.0, 0, 2.5, "txt", "");
    style.bold = false;
    style.italic = false;
    dxf.writeStyle(*dw, style);
    dw->tableEnd();

    dxf.writeView(*dw);
    dxf.writeUcs(*dw);

    dw->tableAppid(1);
    dxf.writeAppid(*dw, "ACAD");
    dw->tableEnd();

    dxf.writeDimStyle(*dw, 2.5, 0.625, 0.625, 0.625, 2.5);

    dxf.writeBlockRecord(*dw);
    dw->tableEnd();

    dw->sectionEnd();

    // ---- Blocks ----
    dw->sectionBlocks();
    dxf.writeBlock(*dw, DL_BlockData("*Model_Space", 0, 0.0, 0.0, 0.0));
    dxf.writeEndBlock(*dw, "*Model_Space");
    dxf.writeBlock(*dw, DL_BlockData("*Paper_Space", 0, 0.0, 0.0, 0.0));
    dxf.writeEndBlock(*dw, "*Paper_Space");
    dxf.writeBlock(*dw, DL_BlockData("*Paper_Space0", 0, 0.0, 0.0, 0.0));
    dxf.writeEndBlock(*dw, "*Paper_Space0");
    dw->sectionEnd();

    // ---- Entities ----
    dw->sectionEntities();

    DL_Attributes attrX("PlatformX", 256, -1, -1, "BYLAYER");
    DL_Attributes attrY("PlatformY", 256, -1, -1, "BYLAYER");
    DL_Attributes attrO("PlatformOrigin", 256, -1, -1, "BYLAYER");

    for (const auto& p : platforms) {
        const Eigen::Vector3d& T = p.T;
        const Eigen::Vector3d& X = p.X;
        const Eigen::Vector3d& Y = p.Y;

        writeLine(dxf, dw, attrX, T, T + kAxisLength * X);
        writeLine(dxf, dw, attrY, T, T + kAxisLength * Y);
        writeLine(dxf, dw, attrO, T - kCrossHalf * X, T + kCrossHalf * X);
        writeLine(dxf, dw, attrO, T - kCrossHalf * Y, T + kCrossHalf * Y);
    }

    dw->sectionEnd();

    // ---- Objects ----
    dxf.writeObjects(*dw, "MY_OBJECTS");
    dxf.writeObjectsEnd(*dw);

    dw->dxfEOF();
    dw->close();
    delete dw;

    PLOG_INFO << outputPath << " written (platforms=" << platforms.size() << ")";
}

}  // namespace

TestPlatformPoseDxf::TestPlatformPoseDxf() {}

void TestPlatformPoseDxf::run() {
    try {
        // 走 PlatformPoseData::loadCompact：JSON 里的世界坐标系 X/Y/T 会被转成
        // 相机坐标系下的旋转向量 + 平移向量。
        PlatformPoseData poseData;
        if (!poseData.loadCompact("./data/calibration_config/platform_pose.json")) {
            PLOG_ERROR << "PlatformPoseData::loadCompact failed for platform_pose.json";
            return;
        }

        PLOG_INFO << "[loadCompact] raw entries: allRotVecs=" << poseData.allRotVecs.size()
                  << ", allTransVecs=" << poseData.allTransVecs.size();
        for (size_t i = 0; i < poseData.allRotVecs.size(); ++i) {
            const auto& rv = poseData.allRotVecs[i];
            const auto& tv = poseData.allTransVecs[i];
            PLOG_INFO << "  raw[" << i << "]  rvec=(" << rv.x() << ", " << rv.y() << ", " << rv.z()
                      << ")  tvec=(" << tv.x() << ", " << tv.y() << ", " << tv.z()
                      << ")  |tvec|=" << tv.norm();
        }

        // 把每个平台的旋转向量用 Rodrigues 转回旋转矩阵，取前两列作为相机坐标系下的
        // X、Y 轴方向。allRotVecs 末尾追加了 worldPose，循环时跳过；也跳过未标定的项。
        std::vector<PlatformAxis> platforms;
        for (size_t i = 0; i + 1 < poseData.allRotVecs.size(); ++i) {
            if (poseData.allTransVecs[i].norm() < 1e-6) continue;

            cv::Mat rvec(3, 1, CV_64F);
            for (int j = 0; j < 3; ++j) rvec.at<double>(j) = poseData.allRotVecs[i](j);
            cv::Mat R;
            cv::Rodrigues(rvec, R);

            PlatformAxis pa;
            pa.id = static_cast<int>(i);
            pa.X = Eigen::Vector3d(R.at<double>(0, 0), R.at<double>(1, 0), R.at<double>(2, 0));
            pa.Y = Eigen::Vector3d(R.at<double>(0, 1), R.at<double>(1, 1), R.at<double>(2, 1));
            pa.T = poseData.allTransVecs[i];
            platforms.push_back(pa);
        }

        PLOG_INFO << "[loadCompact] " << platforms.size() << " platforms (camera frame)";
        for (const auto& p : platforms) {
            PLOG_INFO << "  platform[" << p.id << "]  X=(" << p.X.x() << ", " << p.X.y() << ", " << p.X.z()
                      << ")  Y=(" << p.Y.x() << ", " << p.Y.y() << ", " << p.Y.z()
                      << ")  T=(" << p.T.x() << ", " << p.T.y() << ", " << p.T.z() << ")";
        }

        writePlatformDxf("./data/calibration_config/platform_pose.dxf", platforms);
    } catch (const std::exception& e) {
        PLOG_ERROR << "TestPlatformPoseDxf::run failed: " << e.what();
    }
}

void TestPlatformPoseDxf::runDirect() {
    try {
        std::vector<PlatformAxis> platforms;
        if (!parsePlatformPoseJson("./data/calibration_config/platform_pose.json", platforms)) {
            return;
        }

        PLOG_INFO << "[direct] " << platforms.size() << " platforms (world frame)";
        for (const auto& p : platforms) {
            PLOG_INFO << "  platform[" << p.id << "]  X=(" << p.X.x() << ", " << p.X.y() << ", " << p.X.z()
                      << ")  Y=(" << p.Y.x() << ", " << p.Y.y() << ", " << p.Y.z()
                      << ")  T=(" << p.T.x() << ", " << p.T.y() << ", " << p.T.z() << ")";
        }

        writePlatformDxf("./data/calibration_config/platform_pose_world.dxf", platforms);
    } catch (const std::exception& e) {
        PLOG_ERROR << "TestPlatformPoseDxf::runDirect failed: " << e.what();
    }
}
