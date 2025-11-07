
#pragma once
#ifndef LIBCBDETECT_FILTER_CORNERS_H
#define LIBCBDETECT_FILTER_CORNERS_H

#include <vector>

#include <opencv2/opencv.hpp>

#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/config.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/find_corners.h"

namespace cbdetect {

LIBCBDETECT_DLL_DECL void filter_corners(const cv::Mat& img, const cv::Mat& img_angle, const cv::Mat& img_weight,
                                         Corner& corner, const Params& params);

}

#endif //LIBCBDETECT_FILTER_CORNERS_H
