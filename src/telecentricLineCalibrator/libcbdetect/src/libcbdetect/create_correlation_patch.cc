#include <math.h>

#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/create_correlation_patch.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/config.h"

namespace cbdetect {

void create_correlation_patch(std::vector<cv::Mat>& template_kernel, double angle_1, double angle_2, int radius) {
  // 计算模板的宽度和高度
  int width  = radius * 2 + 1;
  int height = radius * 2 + 1;

  // 初始化模板矩阵
  template_kernel[0] = cv::Mat::zeros(height, width, CV_64F);
  template_kernel[1] = cv::Mat::zeros(height, width, CV_64F);
  template_kernel[2] = cv::Mat::zeros(height, width, CV_64F);
  template_kernel[3] = cv::Mat::zeros(height, width, CV_64F);

  // 计算模板中心点
  int mu = radius + 1;
  int mv = radius + 1;

  // 根据输入角度计算法向量
  double n1[2]{-std::sin(angle_1), std::cos(angle_1)};
  double n2[2]{-std::sin(angle_2), std::cos(angle_2)};

  // 遍历模板中的每个像素
  for(int u = 0; u < width; ++u) {
    for(int v = 0; v < height; ++v) {
      // 计算当前像素相对于中心点的向量
      int vec[2]{u + 1 - mu, v + 1 - mv};
      double dist = std::sqrt(vec[0] * vec[0] + vec[1] * vec[1]);

      // 计算当前像素在法向量上的投影
      double s1 = vec[0] * n1[0] + vec[1] * n1[1];
      double s2 = vec[0] * n2[0] + vec[1] * n2[1];

      // 如果像素在半径范围内，根据投影值设置模板值
      //注意此处坐标系
      if(dist <= radius) {
        if(s1 <= -0.1 && s2 <= -0.1) {
          template_kernel[0].at<double>(v, u) = 1;
        } else if(s1 >= 0.1 && s2 >= 0.1) {
          template_kernel[1].at<double>(v, u) = 1;
        } else if(s1 <= -0.1 && s2 >= 0.1) {
          template_kernel[2].at<double>(v, u) = 1;
        } else if(s1 >= 0.1 && s2 <= -0.1) {
          template_kernel[3].at<double>(v, u) = 1;
        }
      }
    }
  }

  // 对每个模板进行归一化
  double sum = cv::sum(template_kernel[0])[0];
  if(sum > 1e-5) {
    template_kernel[0] /= sum;
  }
  sum = cv::sum(template_kernel[1])[0];
  if(sum > 1e-5) {
    template_kernel[1] /= sum;
  }
  sum = cv::sum(template_kernel[2])[0];
  if(sum > 1e-5) {
    template_kernel[2] /= sum;
  }
  sum = cv::sum(template_kernel[3])[0];
  if(sum > 1e-5) {
    template_kernel[3] /= sum;
  }
}

void create_correlation_patch(std::vector<cv::Mat>& template_kernel,
                              double angle_1, double angle_2, double angle_3, int radius) {
  // 计算模板的宽度和高度
  int width  = radius * 2 + 1;
  int height = radius * 2 + 1;

  // 初始化6个模板矩阵
  template_kernel[0] = cv::Mat::zeros(height, width, CV_64F);
  template_kernel[1] = cv::Mat::zeros(height, width, CV_64F);
  template_kernel[2] = cv::Mat::zeros(height, width, CV_64F);
  template_kernel[3] = cv::Mat::zeros(height, width, CV_64F);
  template_kernel[4] = cv::Mat::zeros(height, width, CV_64F);
  template_kernel[5] = cv::Mat::zeros(height, width, CV_64F);

  // 计算模板中心点
  int mu = radius + 1;
  int mv = radius + 1;

  // 根据输入角度计算法向量
  double n1[2]{-std::sin(angle_1), std::cos(angle_1)};
  double n2[2]{-std::sin(angle_2), std::cos(angle_2)};
  double n3[3]{-std::sin(angle_3), std::cos(angle_3)};

  // 遍历模板中的每个像素
  for(int u = 0; u < width; ++u) {
    for(int v = 0; v < height; ++v) {
      // 计算当前像素相对于中心点的向量
      int vec[2]{u + 1 - mu, v + 1 - mv};
      double dist = std::sqrt(vec[0] * vec[0] + vec[1] * vec[1]);

      // 计算当前像素在三个法向量上的投影
      double s1 = vec[0] * n1[0] + vec[1] * n1[1];
      double s2 = vec[0] * n2[0] + vec[1] * n2[1];
      double s3 = vec[0] * n3[0] + vec[1] * n3[1];

      // 如果像素在半径范围内，根据投影值设置模板值
      if(dist <= radius) {
        if(s1 >= -0.1 && s2 <= -0.1) {
          template_kernel[0].at<double>(v, u) = 1;
        } else if(s1 >= 0.1 && s3 >= 0.1) {
          template_kernel[1].at<double>(v, u) = 1;
        } else if(s2 <= -0.1 && s3 >= 0.1) {
          template_kernel[2].at<double>(v, u) = 1;
        } else if(s1 <= 0.1 && s2 >= -0.1) {
          template_kernel[3].at<double>(v, u) = 1;
        } else if(s1 <= 0.1 && s3 <= -0.1) {
          template_kernel[4].at<double>(v, u) = 1;
        } else if(s2 >= 0.1 && s3 <= -0.1) {
          template_kernel[5].at<double>(v, u) = 1;
        }
      }
    }
  }

  // 对每个模板进行归一化
  double sum = cv::sum(template_kernel[0])[0];
  if(sum > 1e-5) {
    template_kernel[0] /= sum;
  }
  sum = cv::sum(template_kernel[1])[0];
  if(sum > 1e-5) {
    template_kernel[1] /= sum;
  }
  sum = cv::sum(template_kernel[2])[0];
  if(sum > 1e-5) {
    template_kernel[2] /= sum;
  }
  sum = cv::sum(template_kernel[3])[0];
  if(sum > 1e-5) {
    template_kernel[3] /= sum;
  }
  sum = cv::sum(template_kernel[4])[0];
  if(sum > 1e-5) {
    template_kernel[4] /= sum;
  }
  sum = cv::sum(template_kernel[5])[0];
  if(sum > 1e-5) {
    template_kernel[5] /= sum;
  }
}

} // namespace cbdetect
