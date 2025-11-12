#include <math.h>

#include <opencv2/opencv.hpp>

#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/refine_corners.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/config.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/find_corners.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/find_modes_meanshift.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/get_image_patch.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/weight_mask.h"

namespace cbdetect {

std::vector<std::vector<double>> edge_orientations(cv::Mat& img_angle, cv::Mat& img_weight) {
  // number of bins (histogram parameter)
  int n = 32;

  // convert angles from normals to directions
  img_angle.forEach<double>([](double& val, const int* pos) -> void {
    val += M_PI / 2;
    val = val >= M_PI ? val - M_PI : val;
  });

  // create histogram
  std::vector<double> angle_hist(n, 0);
  for(int i = 0; i < img_angle.cols; ++i) {
    for(int j = 0; j < img_angle.rows; ++j) {
      int bin = static_cast<int>(std::floor(img_angle.at<double>(j, i) / (M_PI / n)));
      angle_hist[bin] += img_weight.at<double>(j, i);
    }
  }

  // find modes of smoothed histogram
  auto modes = find_modes_meanshift(angle_hist, 1.5);

  // if only one or no mode => return invalid corner
  if(modes.size() <= 1) {
    return std::vector<std::vector<double>>();
  }

  // compute orientation at modes
  // extract 2 strongest modes and sort by angle
  double angle_1 = modes[0].first * M_PI / n + M_PI / n / 2;
  double angle_2 = modes[1].first * M_PI / n + M_PI / n / 2;
  if(angle_1 > angle_2) {
    std::swap(angle_1, angle_2);
  }

  // compute angle between modes
  double delta_angle = std::min(angle_2 - angle_1, angle_1 + M_PI - angle_2);

  // if angle too small => return invalid corner
  if(delta_angle <= 0.3) {
    return std::vector<std::vector<double>>();
  }

  // set statistics: orientations
  std::vector<std::vector<double>> v(2, std::vector<double>(2));
  v[0][0] = std::cos(angle_1);
  v[0][1] = std::sin(angle_1);
  v[1][0] = std::cos(angle_2);
  v[1][1] = std::sin(angle_2);
  return v;
}

std::vector<std::vector<double>> edge_3_orientations(cv::Mat& img_angle, cv::Mat& img_weight) {
  // number of bins (histogram parameter)
  int n = 32;

  // convert angles from normals to directions
  img_angle.forEach<double>([](double& val, const int* pos) -> void {
    val += M_PI / 2;
    val = val >= M_PI ? val - M_PI : val;
  });

  // create histogram
  std::vector<double> angle_hist(n, 0);
  for(int i = 0; i < img_angle.cols; ++i) {
    for(int j = 0; j < img_angle.rows; ++j) {
      int bin = static_cast<int>(std::floor(img_angle.at<double>(j, i) / (M_PI / n)));
      angle_hist[bin] += img_weight.at<double>(j, i);
    }
  }

  // find modes of smoothed histogram
  auto modes = find_modes_meanshift(angle_hist, 1.5);

  // if only one or no mode => return invalid corner
  if(modes.size() <= 2) {
    return std::vector<std::vector<double>>();
  }

  // compute orientation at modes
  // extract 2 strongest modes and sort by angle
  double angle_1 = modes[0].first * M_PI / n + M_PI / n / 2;
  double angle_2 = modes[1].first * M_PI / n + M_PI / n / 2;
  double angle_3 = modes[2].first * M_PI / n + M_PI / n / 2;
  if(angle_1 > angle_2) {
    std::swap(angle_1, angle_2);
  }
  if(angle_1 > angle_3) {
    std::swap(angle_1, angle_3);
  }
  if(angle_2 > angle_3) {
    std::swap(angle_2, angle_3);
  }

  // compute angle between modes
  double delta_angle_1 = std::min(angle_2 - angle_1, angle_1 + M_PI - angle_2);
  double delta_angle_2 = std::min(angle_3 - angle_2, angle_2 + M_PI - angle_3);
  double delta_angle_3 = std::min(angle_3 - angle_1, angle_1 + M_PI - angle_3);

  // if angle too small => return invalid corner
  if(delta_angle_1 <= 0.2 || delta_angle_2 <= 0.2 || delta_angle_3 <= 0.2) {
    return std::vector<std::vector<double>>();
  }

  // set statistics: orientations
  std::vector<std::vector<double>> v(3, std::vector<double>(2));
  v[0][0] = std::cos(angle_1);
  v[0][1] = std::sin(angle_1);
  v[1][0] = std::cos(angle_2);
  v[1][1] = std::sin(angle_2);
  v[2][0] = std::cos(angle_3);
  v[2][1] = std::sin(angle_3);
  return v;
}

void refine_corners(const cv::Mat& img_du, const cv::Mat& img_dv, const cv::Mat& img_angle, const cv::Mat& img_weight,
                    Corner& corners, const Params& params) {
  // 最大迭代次数和精度
  int max_iteration     = 10;
  double eps            = 0.1;
  bool is_monkey_saddle = params.corner_type == MonkeySaddlePoint;

  // 获取图像尺寸
  int width = img_du.cols, height = img_du.rows;
  // 初始化输出角点容器
  std::vector<cv::Point2d> corners_out_p, corners_out_v1, corners_out_v2, corners_out_v3;
  std::vector<int> corners_out_r;
  // 初始化选择标记数组
  std::vector<int> choose(corners.p.size(), 0);
  // 初始化角点方向向量
  corners.v1.resize(corners.p.size());
  corners.v2.resize(corners.p.size());
  if(is_monkey_saddle) {
    corners.v3.resize(corners.p.size());
  }
  // 获取权重掩码
  auto mask = weight_mask(params.radius);

  // 并行处理每个角点
  cv::parallel_for_(cv::Range(0, corners.p.size()), [&](const cv::Range& range) -> void {
    for(int i = range.start; i < range.end; ++i) {
      // 提取当前角点位置
      int ui        = std::round(corners.p[i].x);
      int vi        = std::round(corners.p[i].y);
      double u_init = corners.p[i].x;
      double v_init = corners.p[i].y;
      int r         = corners.r[i];

      // 估计边缘方向（如果太靠近边界则跳过）
      if(ui - r < 0 || ui + r >= width - 1 || vi - r < 0 || vi + r >= height - 1) {
        continue;
      }
      // 提取图像子区域
      cv::Mat img_angle_sub, img_weight_sub;
      get_image_patch(img_angle, ui, vi, r, img_angle_sub);
      get_image_patch(img_weight, ui, vi, r, img_weight_sub);
      img_weight_sub = img_weight_sub.mul(mask[r]);
      // 获取边缘方向
      auto v         = is_monkey_saddle ? edge_3_orientations(img_angle_sub, img_weight_sub) : edge_orientations(img_angle_sub, img_weight_sub);

      // 如果边缘方向无效则跳过
      if(v.empty()) {
        continue;
      }

      // 角点方向优化
      cv::Mat A1 = cv::Mat::zeros(2, 2, CV_64F);
      cv::Mat A2 = cv::Mat::zeros(2, 2, CV_64F);
      cv::Mat A3 = cv::Mat::zeros(2, 2, CV_64F);
      for(int j2 = vi - r; j2 <= vi + r; ++j2) {
        for(int i2 = ui - r; i2 <= ui + r; ++i2) {
          // 像素方向向量
          double o_du   = img_du.at<double>(j2, i2);
          double o_dv   = img_dv.at<double>(j2, i2);
          double o_norm = std::sqrt(o_du * o_du + o_dv * o_dv);
          if(o_norm < 0.1) {
            continue;
          }
          double o_du_norm = o_du / o_norm;
          double o_dv_norm = o_dv / o_norm;

          // 优化方向1
          if(std::abs(o_du_norm * v[0][0] + o_dv_norm * v[0][1]) < 0.25) {
            A1.at<double>(0, 0) += o_du * o_du;
            A1.at<double>(0, 1) += o_du * o_dv;
            A1.at<double>(1, 0) += o_du * o_dv;
            A1.at<double>(1, 1) += o_dv * o_dv;
          }

          // 优化方向2
          if(std::abs(o_du_norm * v[1][0] + o_dv_norm * v[1][1]) < 0.25) {
            A2.at<double>(0, 0) += o_du * o_du;
            A2.at<double>(0, 1) += o_du * o_dv;
            A2.at<double>(1, 0) += o_du * o_dv;
            A2.at<double>(1, 1) += o_dv * o_dv;
          }

          // 优化方向3（仅用于MonkeySaddlePoint）
          if(is_monkey_saddle && std::abs(o_du_norm * v[2][0] + o_dv_norm * v[2][1]) < 0.25) {
            A3.at<double>(0, 0) += o_du * o_du;
            A3.at<double>(0, 1) += o_du * o_dv;
            A3.at<double>(1, 0) += o_du * o_dv;
            A3.at<double>(1, 1) += o_dv * o_dv;
          }
        }
      }

      // 设置新的角点方向
      cv::Mat eig_tmp1, eig_tmp2;
      cv::eigen(A1, eig_tmp1, eig_tmp2);
      v[0][0] = eig_tmp2.at<double>(1, 0);
      v[0][1] = eig_tmp2.at<double>(1, 1);
      cv::eigen(A2, eig_tmp1, eig_tmp2);
      v[1][0] = eig_tmp2.at<double>(1, 0);
      v[1][1] = eig_tmp2.at<double>(1, 1);
      if(is_monkey_saddle) {
        cv::eigen(A3, eig_tmp1, eig_tmp2);
        v[2][0] = eig_tmp2.at<double>(1, 0);
        v[2][1] = eig_tmp2.at<double>(1, 1);
      }

      // 对方向进行排序
      std::sort(v.begin(), v.end(), [](const auto& a1, const auto& a2) {
        return a1[0] * a2[1] - a1[1] * a2[0] > 0;
      });
      v[v.size() - 1][0] = -v[v.size() - 1][0];
      v[v.size() - 1][1] = -v[v.size() - 1][1];
      std::sort(v.begin(), v.end(), [](const auto& a1, const auto& a2) {
        return a1[0] * a2[1] - a1[1] * a2[0] > 0;
      });

      // 如果使用多项式拟合，则直接选择角点
      if(params.polynomial_fit) {
        choose[i]     = 1;
        corners.v1[i] = cv::Point2d(v[0][0], v[0][1]);
        corners.v2[i] = cv::Point2d(v[1][0], v[1][1]);
        if(is_monkey_saddle) {
          corners.v3[i] = cv::Point2d(v[2][0], v[2][1]);
        }
        continue;
      }

      // 角点位置优化
      double u_cur = u_init, v_cur = v_init, u_last = u_cur, v_last = v_cur;
      for(int num_it = 0; num_it < max_iteration; ++num_it) {
        cv::Mat G = cv::Mat::zeros(2, 2, CV_64F);
        cv::Mat b = cv::Mat::zeros(2, 1, CV_64F);

        // 获取子像素梯度
        cv::Mat img_du_sub, img_dv_sub;
        if(u_cur - r < 0 || u_cur + r >= width || v_cur - r < 0 || v_cur + r >= height) {
          break;
        }
        get_image_patch(img_du, u_cur, v_cur, r, img_du_sub);
        get_image_patch(img_dv, u_cur, v_cur, r, img_dv_sub);

        for(int j2 = 0; j2 < 2 * r + 1; ++j2) {
          for(int i2 = 0; i2 < 2 * r + 1; ++i2) {
            // 像素方向向量
            double o_du   = img_du_sub.at<double>(j2, i2);
            double o_dv   = img_dv_sub.at<double>(j2, i2);
            double o_norm = std::sqrt(o_du * o_du + o_dv * o_dv);
            if(o_norm < 0.1) {
              continue;
            }
            double o_du_norm = o_du / o_norm;
            double o_dv_norm = o_dv / o_norm;

            // 不考虑中心像素
            if(i2 == r && j2 == r) {
              continue;
            }

            // 鲁棒子像素角点估计
            // 计算像素的相对位置和到向量的距离
            double w_u = i2 - r - ((i2 - r) * v[0][0] + (j2 - r) * v[0][1]) * v[0][0];
            double v_u = j2 - r - ((i2 - r) * v[0][0] + (j2 - r) * v[0][1]) * v[0][1];
            double d1  = std::sqrt(w_u * w_u + v_u * v_u);
            w_u        = i2 - r - ((i2 - r) * v[1][0] + (j2 - r) * v[1][1]) * v[1][0];
            v_u        = j2 - r - ((i2 - r) * v[1][0] + (j2 - r) * v[1][1]) * v[1][1];
            double d2  = std::sqrt(w_u * w_u + v_u * v_u);

            // 如果像素对应于任一向量/方向
            if((d1 < 3 && std::abs(o_du_norm * v[0][0] + o_dv_norm * v[0][1]) < 0.25) ||
               (d2 < 3 && std::abs(o_du_norm * v[1][0] + o_dv_norm * v[1][1]) < 0.25)) {
              G.at<double>(0, 0) += o_du * o_du;
              G.at<double>(0, 1) += o_du * o_dv;
              G.at<double>(1, 0) += o_du * o_dv;
              G.at<double>(1, 1) += o_dv * o_dv;
              b.at<double>(0, 0) += o_du * o_du * (i2 - r + u_cur) + o_du * o_dv * (j2 - r + v_cur);
              b.at<double>(1, 0) += o_du * o_dv * (i2 - r + u_cur) + o_dv * o_dv * (j2 - r + v_cur);
            }

            // 如果是MonkeySaddlePoint，则处理第三个方向
            if(is_monkey_saddle) {
              w_u       = i2 - r - ((i2 - r) * v[2][0] + (j2 - r) * v[2][1]) * v[2][0];
              v_u       = j2 - r - ((i2 - r) * v[2][0] + (j2 - r) * v[2][1]) * v[2][1];
              double d3 = std::sqrt(w_u * w_u + v_u * v_u);
              if(d3 < 3 && std::abs(o_du_norm * v[2][0] + o_dv_norm * v[2][1]) < 0.25) {
                G.at<double>(0, 0) += o_du * o_du;
                G.at<double>(0, 1) += o_du * o_dv;
                G.at<double>(1, 0) += o_du * o_dv;
                G.at<double>(1, 1) += o_dv * o_dv;
                b.at<double>(0, 0) += o_du * o_du * (i2 - r + u_cur) + o_du * o_dv * (j2 - r + v_cur);
                b.at<double>(1, 0) += o_du * o_dv * (i2 - r + u_cur) + o_dv * o_dv * (j2 - r + v_cur);
              }
            }
          }
        }

        // 如果G满秩，则设置新的角点位置
        cv::Mat new_pos = G.inv() * b;
        u_last          = u_cur;
        v_last          = v_cur;
        u_cur           = new_pos.at<double>(0, 0);
        v_cur           = new_pos.at<double>(1, 0);
        double dist     = std::sqrt((u_cur - u_last) * (u_cur - u_last) + (v_cur - v_last) * (v_cur - v_last));
        if(dist >= 3) {
          u_cur = u_last;
          v_cur = v_last;
          break;
        }
        if(dist <= eps) {
          break;
        }
      }

      // 如果角点位置变化不大，则添加到角点列表
      if(std::sqrt((u_cur - u_init) * (u_cur - u_init) + (v_cur - v_init) * (v_cur - v_init)) < std::max(r / 2, 3)) {
        choose[i]     = 1;
        corners.p[i]  = cv::Point2d(u_cur, v_cur);
        corners.v1[i] = cv::Point2d(v[0][0], v[0][1]);
        corners.v2[i] = cv::Point2d(v[1][0], v[1][1]);
        if(is_monkey_saddle) {
          corners.v3[i] = cv::Point2d(v[2][0], v[2][1]);
        }
      }
    }
  });

  // 收集被选择的角点
  for(int i = 0; i < corners.p.size(); ++i) {
    if(choose[i] == 1) {
      corners_out_p.emplace_back(corners.p[i]);
      corners_out_r.emplace_back(corners.r[i]);
      corners_out_v1.emplace_back(corners.v1[i]);
      corners_out_v2.emplace_back(corners.v2[i]);
      if(is_monkey_saddle) {
        corners_out_v3.emplace_back(corners.v3[i]);
      }
    }
  }
  // 更新角点信息
  corners.p  = std::move(corners_out_p);
  corners.r  = std::move(corners_out_r);
  corners.v1 = std::move(corners_out_v1);
  corners.v2 = std::move(corners_out_v2);
  if(is_monkey_saddle) {
    corners.v3 = std::move(corners_out_v3);
  }
}

} // namespace cbdetect
