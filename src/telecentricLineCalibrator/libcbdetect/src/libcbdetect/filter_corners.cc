
#include <math.h>
#include <stdio.h>

#include <algorithm>
#include <unordered_map>
#include <vector>

#include <opencv2/opencv.hpp>

#include "libcbdetect/config.h"
#include "libcbdetect/filter_corners.h"
#include "libcbdetect/find_corners.h"
#include "libcbdetect/find_modes_meanshift.h"
#include "libcbdetect/weight_mask.h"

namespace cbdetect {

void filter_corners(const cv::Mat& img, const cv::Mat& img_angle, const cv::Mat& img_weight,
                    Corner& corners, const Params& params) {
  // 不同类型的角点参数设置
  int n_cicle, n_bin, crossing_thr, need_crossing, need_mode;
  if(params.corner_type == SaddlePoint) {
    n_cicle = n_bin = 32;//圆周上采样的点数\方向直方图的bin数
    crossing_thr    = 3; //最小零交叉间距
    need_crossing   = 4;//需要的零交叉数（用于检测亮-暗-亮结构）
    need_mode       = 2;//所需角度方向峰值数量（反映方向复杂性）
  } else if(params.corner_type == MonkeySaddlePoint) {
    n_cicle       = 48;
    n_bin         = 32;
    crossing_thr  = 3;
    need_crossing = 6;
    need_mode     = 3;
  }
  // 获取图像尺寸
  int width = img.cols, height = img.rows;
   // 初始化输出角点容器
  std::vector<cv::Point2d> corners_out_p;
  std::vector<int> corners_out_r;
  std::vector<int> choose(corners.p.size(), 0);// 初始化选择标记数组
  std::vector<double> cos_v(n_cicle), sin_v(n_cicle);
  for(int i = 0; i < n_cicle; ++i) {
    cos_v[i] = std::cos(i * 2.0 * M_PI / (n_cicle - 1));
    sin_v[i] = std::sin(i * 2.0 * M_PI / (n_cicle - 1));
  }
  // 获取权重掩码
  auto mask = weight_mask(params.radius);
 // 并行处理每个角点
  cv::parallel_for_(cv::Range(0, corners.p.size()), [&](const cv::Range& range) -> void {
    for(int i = range.start; i < range.end; ++i) {
      int num_crossings = 0, num_modes = 0;
       // 获取当前角点中心坐标和半径
      int center_u = std::round(corners.p[i].x);
      int center_v = std::round(corners.p[i].y);
      int r        = corners.r[i];
       // 检查角点是否在图像边界内
      if(center_u - r < 0 || center_u + r >= width - 1 || center_v - r < 0 || center_v + r >= height - 1) {
        continue;
      }

      // 提取圆周采样点的像素值
      std::vector<double> c(n_cicle);
      for(int j = 0; j < n_cicle; ++j) {
        int circle_u = static_cast<int>(std::round(center_u + 0.75 * r * cos_v[j]));
        int circle_v = static_cast<int>(std::round(center_v + 0.75 * r * sin_v[j]));
        circle_u     = std::min(std::max(circle_u, 0), width - 1);
        circle_v     = std::min(std::max(circle_v, 0), height - 1);
        c[j]         = img.at<double>(circle_v, circle_u);
      }
      // 归一化圆周采样点像素值
      auto minmax  = std::minmax_element(c.begin(), c.end());
      double min_c = *minmax.first, max_c = *minmax.second;
      for(int j = 0; j < n_cicle; ++j) {
        c[j] = c[j] - min_c - (max_c - min_c) / 2;
      }

      // 计算零交叉数量
      int fisrt_cross_index = 0;
      for(int j = 0; j < n_cicle; ++j) {
        if((c[j] > 0) ^ (c[(j + 1) % n_cicle] > 0)) {
          fisrt_cross_index = (j + 1) % n_cicle;
          break;
        }
      }
      for(int j = fisrt_cross_index, count = 1; j < n_cicle + fisrt_cross_index; ++j, ++count) {
        if((c[j % n_cicle] > 0) ^ (c[(j + 1) % n_cicle] > 0)) {
          if(count >= crossing_thr) {
            ++num_crossings;
          }
          count = 1;
        }
      }

      // 提取权重子图像
      int top_left_u         = std::max(center_u - r, 0);
      int top_left_v         = std::max(center_v - r, 0);
      int bottom_right_u     = std::min(center_u + r, width - 1);
      int bottom_right_v     = std::min(center_v + r, height - 1);
      cv::Mat img_weight_sub = cv::Mat::zeros(2 * r + 1, 2 * r + 1, CV_64F);
      img_weight.rowRange(top_left_v, bottom_right_v + 1).colRange(top_left_u, bottom_right_u + 1).copyTo(img_weight_sub(cv::Range(top_left_v - center_v + r, bottom_right_v - center_v + r + 1), cv::Range(top_left_u - center_u + r, bottom_right_u - center_u + r + 1)));
      img_weight_sub    = img_weight_sub.mul(mask[r]);
      // 过滤低权重值
      double tmp_maxval = 0;
      cv::minMaxLoc(img_weight_sub, NULL, &tmp_maxval);
      img_weight_sub.forEach<double>([&tmp_maxval](double& val, const int* pos) -> void {
        val = val < 0.5 * tmp_maxval ? 0 : val;
      });

      // 创建角度直方图
      std::vector<double> angle_hist(n_bin, 0);
      for(int j2 = top_left_v; j2 <= bottom_right_v; ++j2) {
        for(int i2 = top_left_u; i2 <= bottom_right_u; ++i2) {
          int bin = static_cast<int>(std::floor(img_angle.at<double>(j2, i2) / (M_PI / n_bin))) % n_bin;
          angle_hist[bin] += img_weight_sub.at<double>(j2 - center_v + r, i2 - center_u + r);
        }
      }
      // 查找角度直方图的峰值
      auto modes = find_modes_meanshift(angle_hist, 1.5);
      for(const auto& j : modes) {
        if(2 * j.second > modes[0].second) {
          ++num_modes;
        }
      }

      // 根据条件选择角点
      if(num_crossings == need_crossing && num_modes >= need_mode) {
        choose[i] = 1;
      }
    }
  });

  for(int i = 0; i < corners.p.size(); ++i) {
    if(choose[i] == 1) {
      corners_out_p.emplace_back(cv::Point2d(corners.p[i].x, corners.p[i].y));
      corners_out_r.emplace_back(corners.r[i]);
    }
  }
  corners.p = std::move(corners_out_p);
  corners.r = std::move(corners_out_r);
}

} // namespace cbdetect
