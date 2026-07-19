#include "platform_pose_io.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <plog/Log.h>

namespace {

    // @TODO:改成采用原始的cereal库加载

// 自 from 起查找 "key" : [a, b, c] 形式的三维向量。
// 容忍 key 与 '[' 之间的任意空白。
Eigen::Vector3d extractVec(const std::string& json, const std::string& key, size_t from) {
    size_t keyPos = json.find("\"" + key + "\"", from);
    if (keyPos == std::string::npos) return Eigen::Vector3d::Zero();
    size_t lb = json.find('[', keyPos);
    if (lb == std::string::npos) return Eigen::Vector3d::Zero();
    size_t rb = json.find(']', lb);
    if (rb == std::string::npos) return Eigen::Vector3d::Zero();
    double v0 = 0.0, v1 = 0.0, v2 = 0.0;
    int parsed = sscanf(json.substr(lb + 1, rb - lb - 1).c_str(), "%lf,%lf,%lf", &v0, &v1, &v2);
    if (parsed != 3) {
        PLOG_WARNING << "extractVec: failed to parse 3 doubles for key \"" << key
                     << "\" (got " << parsed << ")";
        return Eigen::Vector3d::Zero();
    }
    return Eigen::Vector3d(v0, v1, v2);
}

}  // namespace

bool loadPlatformAxes(const std::string& path, std::vector<PlatformAxis>& out) {
    out.clear();

    std::ifstream is(path);
    if (!is.is_open()) {
        PLOG_WARNING << "Cannot open platform pose JSON: " << path;
        return false;
    }
    std::string json((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());

    size_t platformsKey = json.find("\"platforms\"");
    if (platformsKey == std::string::npos) {
        PLOG_WARNING << "Cannot find \"platforms\" key in " << path;
        return false;
    }
    size_t platformsArr = json.find('[', platformsKey);
    if (platformsArr == std::string::npos) return false;
    size_t pos = platformsArr + 1;

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
        PLOG_WARNING << "No platforms parsed from " << path;
        return false;
    }
    return true;
}
