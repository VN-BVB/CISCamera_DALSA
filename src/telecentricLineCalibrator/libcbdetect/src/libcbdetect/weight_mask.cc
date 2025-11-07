


#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/config.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/weight_mask.h"

namespace cbdetect {
//环形权重
std::unordered_map<int, cv::Mat> weight_mask(const std::vector<int>& radius) {
  // 创建一个映射表，键为半径，值为对应的权重矩阵
  std::unordered_map<int, cv::Mat> mask;

    // 遍历所有给定的半径值
  for(const auto& r : radius) {
    // 为当前半径创建一个大小为(2r+1)x(2r+1)的矩阵，类型为64位浮点数
    mask[r]      = cv::Mat::zeros(r * 2 + 1, r * 2 + 1, CV_64F);
    cv::Mat& mat = mask[r];


    for(int v = 0; v < r * 2 + 1; ++v) {
      for(int u = 0; u < r * 2 + 1; ++u) {
        double dist          = std::sqrt((u - r) * (u - r) + (v - r) * (v - r)) / r;
        dist                 = std::min(std::max(dist, 0.7), 1.3);
        // 这样距离中心点越近，权重越大；距离中心点越远，权重越小
        mat.at<double>(v, u) = (1.3 - dist) / 0.6;
      }
    }
  }
  return mask;
}

} // namespace cbdetect
