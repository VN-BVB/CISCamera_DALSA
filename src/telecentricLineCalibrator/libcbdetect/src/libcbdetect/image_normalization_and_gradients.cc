#include <math.h>

#include <opencv2/core/hal/hal.hpp>
#include <opencv2/opencv.hpp>

#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/config.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/image_normalization_and_gradients.h"

namespace cbdetect {

void box_filter(const cv::Mat& img, cv::Mat& blur_img, int kernel_size_x, int kernel_size_y) {
  // 如果y方向核大小小于0，则设为与x方向相同
  if(kernel_size_y < 0) {
    kernel_size_y = kernel_size_x;
  }
  // 创建与输入图像大小相同的输出图像，类型为64位浮点
  blur_img.create(img.size(), CV_64F);
  
  // 初始化缓冲区：buf用于存储列累积值，count_buf用于存储每列的像素计数
  std::vector<double> buf(img.cols, 0);
  std::vector<int> count_buf(img.cols, 0);
  int count = 0;

  // 初始化阶段：累加前kernel_size_y行的像素值
  for(int j = 0; j < std::min(kernel_size_y, img.rows - 1); ++j) {
    for(int i = 0; i < img.cols; ++i) {
      buf[i] += img.at<double>(j, i);
      ++count_buf[i];
    }
  }

  // 滑动窗口处理每一行
  for(int j = 0; j < img.rows; ++j) {
    // 如果当前行超出窗口上边界，减去窗口外的像素值
    if(j > kernel_size_y) {
      for(int i = 0; i < img.cols; ++i) {
        buf[i] -= img.at<double>(j - kernel_size_y - 1, i);
        --count_buf[i];
      }
    }
    // 如果当前行在窗口下边界内，加上新进入窗口的像素值
    if(j + kernel_size_y < img.rows) {
      for(int i = 0; i < img.cols; ++i) {
        buf[i] += img.at<double>(j + kernel_size_y, i);
        ++count_buf[i];
      }
    }

    // 处理当前行的每一列
    blur_img.at<double>(j, 0) = 0;
    count                     = 0;
    for(int i = 0; i <= std::min(kernel_size_x, img.cols - 1); ++i) {
      blur_img.at<double>(j, 0) += buf[i];
      count += count_buf[i];
    }
    // 滑动窗口处理当前行的其他列
    for(int i = 1; i < img.cols; ++i) {
      blur_img.at<double>(j, i) = blur_img.at<double>(j, i - 1);
      blur_img.at<double>(j, i - 1) /= count;
      // 如果超出窗口左边界，减去窗口外的像素值
      if(i > kernel_size_x) {
        blur_img.at<double>(j, i) -= buf[i - kernel_size_x - 1];
        count -= count_buf[i - kernel_size_x - 1];
      }
      // 如果当前列在窗口右边界内，加上新进入窗口的像素值
      if(i + kernel_size_x < img.cols) {
        blur_img.at<double>(j, i) += buf[i + kernel_size_x];
        count += count_buf[i + kernel_size_x];
      }
    }
    // 处理最后一列的平均值
    blur_img.at<double>(j, img.cols - 1) /= count;
  }
}

void image_normalization_and_gradients(cv::Mat& img, cv::Mat& img_du, cv::Mat& img_dv,
                                       cv::Mat& img_angle, cv::Mat& img_weight, const Params& params) {
  // 图像归一化处理
  if(params.norm) {
    cv::Mat blur_img;
    // 使用box_filter进行图像模糊处理
    box_filter(img, blur_img, params.norm_half_kernel_size);
    // 原始图像减去模糊图像，增强边缘
    img = img - blur_img;
    // 对图像进行对比度增强和范围限制
    img = 2.5 * (cv::max(cv::min(img + 0.2, 0.4), 0));
  }

  // 定义Sobel算子用于计算图像梯度
#if CV_VERSION_MAJOR == 4
  cv::Mat_<double> du({3, 3}, {1, 0, -1, 2, 0, -2, 1, 0, -1});
  cv::Mat_<double> dv({3, 3}, {1, 2, 1, 0, 0, 0, -1, -2, -1});
  // cv::Mat_<double> du({3, 3}, {-1, 0, 1, -1, 0, 1, -1, 0, 1});
  // cv::Mat_<double> dv({3, 3}, {-1, -1, -1, 0, 0, 0, 1, 1, 1});
#else
  double du_array[9] = {1, 0, -1, 2, 0, -2, 1, 0, -1};
  double dv_array[9] = {1, 2, 1, 0, 0, 0, -1, -2, -1};
  cv::Mat du(3, 3, CV_64F, du_array);
  cv::Mat dv(3, 3, CV_64F, dv_array);
#endif

  // 计算图像在x和y方向的导数
  cv::filter2D(img, img_du, -1, du, cv::Point(-1, -1), 0, cv::BORDER_REFLECT);
  cv::filter2D(img, img_dv, -1, dv, cv::Point(-1, -1), 0, cv::BORDER_REFLECT);
  
  // 创建用于存储角度和权重的矩阵
  img_angle.create(img.size(), img.type());
  img_weight.create(img.size(), img.type());
  
  // 确保矩阵数据是连续的
  if(!img_du.isContinuous()) {
    cv::Mat tmp = img_du.clone();
    std::swap(tmp, img_du);
  }
  if(!img_dv.isContinuous()) {
    cv::Mat tmp = img_dv.clone();
    std::swap(tmp, img_dv);
  }
  if(!img_angle.isContinuous()) {
    cv::Mat tmp = img_angle.clone();
    std::swap(tmp, img_angle);
  }
  if(!img_weight.isContinuous()) {
    cv::Mat tmp = img_weight.clone();
    std::swap(tmp, img_weight);
  }
  
  // 计算梯度方向角度
  cv::hal::fastAtan64f((const double*)img_dv.data, (const double*)img_du.data,
                       (double*)img_angle.data, img.rows * img.cols, false);
  // 将角度限制在0到π之间
  img_angle.forEach<double>([](double& pixel, const int* pos) -> void {
    pixel = pixel >= M_PI ? pixel - M_PI : pixel;
  });
  
  // 计算梯度幅值作为权重
  img_weight.forEach<double>([&img_du, &img_dv](double& pixel, const int* pos) -> void {
    int u = pos[1];
    int v = pos[0];
    pixel = std::sqrt(
        img_du.at<double>(v, u) * img_du.at<double>(v, u) + img_dv.at<double>(v, u) * img_dv.at<double>(v, u));
  });

  // 对输入图像进行归一化，使其值在0到1之间
  double img_min = 0, img_max = 1;
  cv::minMaxLoc(img, &img_min, &img_max);
  img = (img - img_min) / (img_max - img_min);
}

} // namespace cbdetect
