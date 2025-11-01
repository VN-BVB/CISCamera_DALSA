#include <math.h>

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "libcbdetect/find_modes_meanshift.h"
#include "libcbdetect/config.h"

namespace cbdetect {

std::vector<std::pair<int, double>> find_modes_meanshift(const std::vector<double>& hist, double sigma) {
  // 初始化哈希表和模式容器
  std::unordered_map<int, double> hash_table;
  std::vector<std::pair<int, double>> modes;
  // 计算高斯核的半径
  int r = static_cast<int>(std::round(2 * sigma));
  // 初始化高斯权重
  std::vector<double> weight(2 * r + 1, 0);
  for(int i = 0; i < 2 * r + 1; ++i) {
    // 计算高斯权重
    weight[i] = std::exp(-0.5 * (i - r) * (i - r) / sigma / sigma) / std::sqrt(2 * M_PI) / sigma;
  }

  // 计算平滑后的直方图
  int n = hist.size();
  std::vector<double> hist_smoothed(n, 0);
  for(int i = 0; i < n; ++i) {
    for(int j = 0; j < 2 * r + 1; ++j) {
      // 使用高斯核进行平滑
      hist_smoothed[(i + r) % n] += hist[(i + j) % n] * weight[j];
    }
  }

  // 检查平滑后的直方图是否至少有一个非零值
  // （否则模式查找可能会无限循环）
  auto max_hist_val = std::max_element(hist_smoothed.begin(), hist_smoothed.end());
  if(*max_hist_val < 1e-6) {
    return modes;
  }

  // 模式查找
  std::vector<int> visited(n, 0);
  for(int i = 0; i < n; ++i) {
    int j = i;
    if(!visited[j]) {
      while(1) {
        visited[j] = 1;
        // 获取相邻的索引
        int j1 = (j + 1) % n, j2 = (j + n - 1) % n;
        double h0 = hist_smoothed[j];
        double h1 = hist_smoothed[j1];
        double h2 = hist_smoothed[j2];
        // 移动到相邻的更高值
        if(h1 >= h0 && h1 >= h2) {
          j = j1;
        } else if(h2 > h0 && h2 > h1) {
          j = j2;
        } else {
          break;
        }
      }
      // 将找到的模式存入哈希表
      hash_table[j] = hist_smoothed[j];
    }
  }
  // 将哈希表中的模式存入结果容器
  for(const auto& i : hash_table) {
    modes.emplace_back(i);
  }
  // 按值的大小降序排序
  std::sort(modes.begin(), modes.end(), [](const auto& i1, const auto& i2) -> bool {
    return i1.second > i2.second;
  });

  return modes;
}

} // namespace cbdetect
