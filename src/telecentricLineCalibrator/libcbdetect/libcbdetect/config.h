

#pragma once
#ifndef LIBCBDETECT_CONFIG_H
#define LIBCBDETECT_CONFIG_H

#include <opencv2/opencv.hpp>
#include <vector>

#if CV_VERSION_MAJOR == 3 && CV_VERSION_MINOR <= 2
#include <functional>
namespace cv {
class ParallelLoopBodyLambdaWrapper : public ParallelLoopBody {
private:
    std::function<void(const Range&)> m_functor;

public:
    ParallelLoopBodyLambdaWrapper(std::function<void(const Range&)> functor) : m_functor(functor) {}

    virtual void operator()(const cv::Range& range) const { m_functor(range); }
};

inline void parallel_for_(const Range& range, std::function<void(const Range&)> functor, double nstripes = -1.) {
    parallel_for_(range, ParallelLoopBodyLambdaWrapper(functor), nstripes);
}
}  // namespace cv
#endif

#ifdef _MSC_VER
#define M_PI 3.14159265358979323846   /* pi */
#define M_PI_2 1.57079632679489661923 /* pi/2 */
#define M_PI_4 0.78539816339744830962 /* pi/4 */
#define M_1_PI 0.31830988618379067154 /* 1/pi */
#define M_2_PI 0.63661977236758134308 /* 2/pi */
#endif

#ifndef LIBCBDETECT_DLL_DECL
#if IS_A_DLL && defined(_MSC_VER)
#define LIBCBDETECT_DLL_DECL __declspec(dllexport)
#else
#define LIBCBDETECT_DLL_DECL
#endif
#endif

namespace cbdetect {

enum DetectMethod {
    // 已删除
    TemplateMatchFast = 0,
    // 2012年论文复现
    // Automatic Camera and Range Sensor Calibration using a single Shot
    TemplateMatchSlow,
    // 2017年论文复现
    // Deltille Grids for Geometric Camera Calibration
    HessianResponse,

    // 2018年论文：Accurate Detection and Localization of Checkerboard Corners for Calibration
    LocalizedRadonTransform
};

enum CornerType { SaddlePoint = 0, MonkeySaddlePoint };

typedef struct Params {
    bool show_processing;
    bool show_debug_image;
    bool show_grow_processing;
    bool norm;
    bool polynomial_fit;
    int norm_half_kernel_size;
    int polynomial_fit_half_kernel_size;
    double init_loc_thr;
    double score_thr;
    bool strict_grow;
    bool overlay;
    bool occlusion;
    DetectMethod detect_method;
    CornerType corner_type;
    std::vector<int> radius;
    bool use_known_board_size;
    int board_cols;
    int board_rows;

    Params()
        : show_processing(true),
          show_debug_image(false),
          show_grow_processing(false),
          norm(true),
          polynomial_fit(true),
          norm_half_kernel_size(31),
          polynomial_fit_half_kernel_size(4),
          init_loc_thr(0.025),
          score_thr(0.01),
          strict_grow(true),
          overlay(false),
          occlusion(true),
          detect_method(HessianResponse),
          corner_type(SaddlePoint),
          radius({4, 8, 12}) {}
} Params;

typedef struct Corner {
    std::vector<cv::Point2d> p;
    std::vector<int> r;
    std::vector<cv::Point2d> v1;
    std::vector<cv::Point2d> v2;
    std::vector<cv::Point2d> v3;
    std::vector<double> score;
} Corner;

typedef struct Board {
    std::vector<std::vector<int>> idx;
    std::vector<std::vector<std::vector<double>>> energy;
    int num;

    Board() : num(0) {}
} Board;

}  // namespace cbdetect

#endif  // LIBCBDETECT_CONFIG_H
