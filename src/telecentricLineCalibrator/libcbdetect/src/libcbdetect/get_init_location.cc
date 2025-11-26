#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/get_init_location.h"

#include <opencv2/opencv.hpp>
#include <vector>

#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/config.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/create_correlation_patch.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/get_image_patch.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/image_normalization_and_gradients.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/non_maximum_suppression.h"

namespace cbdetect {

// 自己看 https://github.com/facebookincubator/deltille
void hessian_response(const cv::Mat& img_in, cv::Mat& img_out) {
    const int rows = img_in.rows;
    const int cols = img_in.cols;
    const int stride = cols;

    // allocate output
    img_out = cv::Mat::zeros(rows, cols, CV_64F);

    cv::parallel_for_(cv::Range(1, rows - 1), [&img_in, &img_out, &stride, &cols](const cv::Range& range) -> void {
        // setup input and output pointer to be centered at 1,0 and 1,1 resp.
        auto* in = img_in.ptr<double>(range.start);
        auto* out = img_out.ptr<double>(range.start) + 1;

        for (int i = range.start; i < range.end; ++i) {
            double v11, v12, v21, v22, v31, v32;
            /* fill in shift registers at the beginning of the row */
            v11 = in[-stride];
            v12 = in[1 - stride];
            v21 = in[0];
            v22 = in[1];
            v31 = in[+stride];
            v32 = in[1 + stride];
            /* move input pointer to (1,2) of the 3x3 square */
            in += 2;
            for (int c = 1; c < cols - 1; ++c) {
                /* fetch remaining values (last column) */
                const double v13 = in[-stride];
                const double v23 = *in;
                const double v33 = in[+stride];

                // compute 3x3 Hessian values from symmetric differences.
                double Lxx = (v21 - 2 * v22 + v23);
                double Lyy = (v12 - 2 * v22 + v32);
                double Lxy = (v13 - v11 + v31 - v33) / 4.;

                /* normalize and write out */
                *out = Lxx * Lyy - Lxy * Lxy;

                /* move window */
                v11 = v12;
                v12 = v13;
                v21 = v22;
                v22 = v23;
                v31 = v32;
                v32 = v33;

                /* move input/output pointers */
                in++;
                out++;
            }
            out += 2;
        }
    });
}

void rotate_image(const cv::Mat& img_in, double angle, cv::Mat& img_out, cv::Size out_size = cv::Size()) {
    if (std::abs(angle) < 1e-3) {
        img_out = img_in.clone();
        return;
    }

    // cal new width and height
    double in_center_u = (img_in.cols - 1) / 2.;
    double in_center_v = (img_in.rows - 1) / 2.;
    if (out_size.width <= 0 || out_size.height <= 0) {
        cv::Point2i tl(std::round(-in_center_u * std::cos(angle) - in_center_v * std::sin(angle)),
                       std::round(in_center_u * std::sin(angle) - in_center_v * std::cos(angle)));
        cv::Point2i tr(std::round(in_center_u * std::cos(angle) - in_center_v * std::sin(angle)),
                       std::round(-in_center_u * std::sin(angle) - in_center_v * std::cos(angle)));
        cv::Point2i bl(std::round(-in_center_u * std::cos(angle) + in_center_v * std::sin(angle)),
                       std::round(in_center_u * std::sin(angle) + in_center_v * std::cos(angle)));
        cv::Point2i br(std::round(in_center_u * std::cos(angle) + in_center_v * std::sin(angle)),
                       std::round(-in_center_u * std::sin(angle) + in_center_v * std::cos(angle)));
        if (std::min(tl.x, br.x) > std::min(tr.x, bl.x)) {
            out_size = cv::Size(std::abs(tr.x - bl.x) + 1, std::abs(tl.y - br.y) + 1);
        } else {
            out_size = cv::Size(std::abs(tl.x - br.x) + 1, std::abs(tr.y - bl.y) + 1);
        }
    }
    double out_center_u = (out_size.width - 1) / 2.;
    double out_center_v = (out_size.height - 1) / 2.;

    //  // rotate image
    //  img_out.create(height, width, CV_64F);
    //  double new_center_u = (width - 1) / 2.0;
    //  double new_center_v = (height - 1) / 2.0;
    //  for (int j = 0; j < img_out.rows; ++j) {
    //    for (int i = 0; i < img_out.cols; ++i) {
    //      double u = (i - new_center_u) * std::cos(angle) - (j - new_center_v) * sin(angle) + center_u;
    //      double v = (i - new_center_u) * std::sin(angle) + (j - new_center_v) * cos(angle) + center_v;
    //      if (u < 0 || u >= img_in.cols - 1 || v < 0 || v >= img_in.rows - 1) {
    //        img_out.at<double>(j, i) = 0;
    //        continue;
    //      }
    //
    //      int iu = u;
    //      int iv = v;
    //      double du = u - iu;
    //      double dv = v - iv;
    //      double a00 = 1 - du - dv + du * dv;
    //      double a01 = du - du * dv;
    //      double a10 = dv - du * dv;
    //      double a11 = du * dv;
    //      img_out.at<double>(j, i) = img_in.at<double>(v, u) * a00 + img_in.at<double>(v, u + 1) * a01 +
    //          img_in.at<double>(v + 1, u) * a10 + img_in.at<double>(v + 1, u + 1) * a11;
    //    }
    //  }

    // rotate image
    double shift_u = out_center_u - in_center_u * std::cos(angle) - in_center_v * std::sin(angle);
    double shift_v = out_center_v + in_center_u * std::sin(angle) - in_center_v * std::cos(angle);
    cv::Mat rot =
        (cv::Mat_<double>(2, 3) << std::cos(angle), std::sin(angle), shift_u, -std::sin(angle), std::cos(angle), shift_v);
    cv::warpAffine(img_in, img_out, rot, out_size);
}

// paper: Accurate Detection and Localization of Checkerboard Corners for Calibration
void localized_radon_transform(const cv::Mat& img_in, cv::Mat& img_out) {
    std::vector<double> angles = {0, M_PI / 4};
    std::vector<cv::Mat> rb_imgs(4);
    for (int i = 0; i < 2; ++i) {
        cv::Mat r_img, u_img, v_img;
        rotate_image(img_in, -angles[i], r_img);
        cv::blur(r_img, u_img, cv::Size(11, 3));
        cv::blur(r_img, v_img, cv::Size(3, 11));
        rotate_image(u_img, angles[i], rb_imgs[2 * i], img_in.size());
        rotate_image(v_img, angles[i], rb_imgs[2 * i + 1], img_in.size());
    }
    cv::Mat max_img_1 = cv::max(rb_imgs[0], rb_imgs[1]);
    cv::Mat max_img_2 = cv::max(rb_imgs[2], rb_imgs[3]);
    cv::Mat min_img_1 = cv::min(rb_imgs[0], rb_imgs[1]);
    cv::Mat min_img_2 = cv::min(rb_imgs[2], rb_imgs[3]);
    img_out = cv::max(max_img_1, max_img_2) - cv::min(min_img_1, min_img_2);
    img_out.forEach<double>([](double& pixel, const int* position) -> void { pixel = pixel * pixel; });
}

void get_init_location(const cv::Mat& img, const cv::Mat& img_du, const cv::Mat& img_dv, Corner& corners, const Params& params) {
    // 根据角点类型选择检测方法
    DetectMethod detect_method = params.corner_type == MonkeySaddlePoint ? HessianResponse : params.detect_method;

    // 根据选择的检测方法进行处理
    switch (detect_method) {
        case TemplateMatchFast:
        case TemplateMatchSlow: {
            // 定义模板匹配的角度参数
            std::vector<double> tprops;
            if (detect_method == TemplateMatchFast) {
                tprops = {0, M_PI_2, M_PI_4, -M_PI_4};
            } else {
                tprops = {0,         M_PI_2,   M_PI_4,    -M_PI_4,       0,        M_PI_4,        0,
                          -M_PI_4,   M_PI_4,   M_PI_2,    -M_PI_4,       M_PI_2,   -3 * M_PI / 8, 3 * M_PI / 8,
                          -M_PI / 8, M_PI / 8, -M_PI / 8, -3 * M_PI / 8, M_PI / 8, 3 * M_PI / 8};
            }

            // 对每个半径进行模板匹配
            for (const auto& r : params.radius) {
                cv::Mat img_corners = cv::Mat::zeros(img.size(), CV_64F);
                cv::Mat img_corners_a1, img_corners_a2, img_corners_b1, img_corners_b2, img_corners_mu, img_corners_a,
                    img_corners_b, img_corners_s1, img_corners_s2;

                // 对每个模板进行匹配
                for (int i = 0; i < tprops.size(); i += 2) {
                    std::vector<cv::Mat> template_kernel(4);  // a1, a2, b1, b2
                    create_correlation_patch(template_kernel, tprops[i], tprops[i + 1], r);

                    // 使用模板对图像进行滤波
                    cv::filter2D(img, img_corners_a1, -1, template_kernel[0], cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
                    cv::filter2D(img, img_corners_a2, -1, template_kernel[1], cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
                    cv::filter2D(img, img_corners_b1, -1, template_kernel[2], cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
                    cv::filter2D(img, img_corners_b2, -1, template_kernel[3], cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);

                    // 计算均值
                    img_corners_mu = (img_corners_a1 + img_corners_a2 + img_corners_b1 + img_corners_b2) / 4;

                    // 情况1：a为白色，b为黑色
                    img_corners_a = cv::min(img_corners_a1, img_corners_a2) - img_corners_mu;
                    img_corners_b = img_corners_mu - cv::max(img_corners_b1, img_corners_b2);
                    img_corners_s1 = cv::min(img_corners_a, img_corners_b);
                    // 情况2：b为白色，a为黑色
                    img_corners_a = img_corners_mu - cv::max(img_corners_a1, img_corners_a2);
                    img_corners_b = cv::min(img_corners_b1, img_corners_b2) - img_corners_mu;
                    img_corners_s2 = cv::min(img_corners_a, img_corners_b);

                    // 合并两种情况的结果
                    img_corners = cv::max(img_corners, cv::max(img_corners_s1, img_corners_s2));
                }
                // 非极大值抑制
                non_maximum_suppression(img_corners, 2, params.init_loc_thr, r, corners);
            }
            break;
        }
        case HessianResponse: {
            // 使用高斯模糊预处理图像
            cv::Mat gauss_img;
            cv::GaussianBlur(img, gauss_img, cv::Size(7, 7), 1.5, 1.5);
            // 计算Hessian响应
            cv::Mat hessian_img;
            hessian_response(gauss_img, hessian_img);
            // 计算阈值并进行非极大值抑制
            double mn = 0, mx = 0;
            cv::minMaxIdx(hessian_img, &mn, &mx, NULL, NULL);
            hessian_img = cv::abs(hessian_img);
            double thr = std::abs(mn * params.init_loc_thr);
            for (const auto& r : params.radius) {
                non_maximum_suppression(hessian_img, r, thr, r, corners);
            }
            break;
        }
        case LocalizedRadonTransform: {
            // 计算局部Radon变换响应
            cv::Mat response_img;
            localized_radon_transform(img, response_img);
            // 非极大值抑制
            for (const auto& r : params.radius) {
                non_maximum_suppression(response_img, r, params.init_loc_thr / 10.0, r, corners);
            }
            break;
        }
        default:
            break;
    }

    // 角点位置精炼
    int width = img.cols, height = img.rows;
    cv::parallel_for_(cv::Range(0, corners.p.size()), [&](const cv::Range& range) -> void {
        for (int i = range.start; i < range.end; ++i) {
            double u = corners.p[i].x;
            double v = corners.p[i].y;
            int r = corners.r[i];

            // 初始化矩阵G和b
            cv::Mat G = cv::Mat::zeros(2, 2, CV_64F);
            cv::Mat b = cv::Mat::zeros(2, 1, CV_64F);

            // 获取子图像块
            cv::Mat img_du_sub, img_dv_sub;
            if (u - r < 0 || u + r >= width - 1 || v - r < 0 || v + r >= height - 1) {
                break;
            }
            get_image_patch(img_du, u, v, r, img_du_sub);
            get_image_patch(img_dv, u, v, r, img_dv_sub);

            // 计算梯度信息
            for (int j2 = 0; j2 < 2 * r + 1; ++j2) {
                for (int i2 = 0; i2 < 2 * r + 1; ++i2) {
                    double o_du = img_du_sub.at<double>(j2, i2);
                    double o_dv = img_dv_sub.at<double>(j2, i2);
                    double o_norm = std::sqrt(o_du * o_du + o_dv * o_dv);
                    if (o_norm < 0.1) {
                        continue;
                    }

                    // 忽略中心像素
                    if (i2 == r && j2 == r) {
                        continue;
                    }
                    // 更新矩阵G和b
                    G.at<double>(0, 0) += o_du * o_du;
                    G.at<double>(0, 1) += o_du * o_dv;
                    G.at<double>(1, 0) += o_du * o_dv;
                    G.at<double>(1, 1) += o_dv * o_dv;
                    b.at<double>(0, 0) += o_du * o_du * (i2 - r + u) + o_du * o_dv * (j2 - r + v);
                    b.at<double>(1, 0) += o_du * o_dv * (i2 - r + u) + o_dv * o_dv * (j2 - r + v);
                }
            }

            // 计算新的角点位置
            cv::Mat new_pos = G.inv() * b;
            if (std::abs(new_pos.at<double>(0, 0) - corners.p[i].x) + std::abs(new_pos.at<double>(1, 0) - corners.p[i].y) <
                corners.r[i] * 2) {
                corners.p[i].x = new_pos.at<double>(0, 0);
                corners.p[i].y = new_pos.at<double>(1, 0);
            }
        }
    });
}

}  // namespace cbdetect
