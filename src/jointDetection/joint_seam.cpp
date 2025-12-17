#include <iostream>
#include "joint_seam.h"
#include "src/utils/image_tools.h"
#include "src/jointDetection/edgeDetection/canny_zernike_detector.h"
#include "contourProcess/contour_processor.h"
#include "src/utils/scoped_timer.h"
#include "src/utils/ThreadPool.h"

JointSeam::JointSeam(const cv::Mat &image, const cv::Point2f position)
    : m_image(image),
    m_position(position)
{}

void JointSeam::run() {
    // 拼缝两侧亚像素轮廓检测
    std::unique_ptr<AbstractContourDetector> s1;
    std::vector<std::vector<cv::Point2f>> contours;
    {
        std::unique_ptr<ContourDetectorContext> c = std::make_unique<ContourDetectorContext>();
        s1 = std::make_unique<CannyZernikeDetector>();
        c->setDetector(std::move(s1));
        contours = c->detectContours(m_image);
    }
    // 添加m_position偏移量,映射到整体图像坐标
    for (auto& contour : contours) {
        for (auto& point : contour) {
            point.x += m_position.x;
            point.y += m_position.y;
        }
    }

    // 轮廓信息处理（使用线程池并行处理）
    {
        ThreadPool pool(2);
        std::vector<std::future<std::tuple<bool, ContourData, std::vector<cv::Vec4f>, std::vector<cv::Point2f>>>> results;
        std::mutex resultMutex;

        // 将每条轮廓处理任务提交到线程池
        for (const auto& contour : contours) {
            results.emplace_back(pool.enqueue([contour]() -> std::tuple<bool, ContourData, std::vector<cv::Vec4f>, std::vector<cv::Point2f>> {
                ContourProcessor processor;
                if (processor.processContour(contour)) {
                    return {true, processor.getResult(), processor.getTangentLines(), processor.getIntersections()};
                }
                return {false, ContourData(), {}, {}};
            }));
        }

        // 收集所有任务的结果
        for (auto& future : results) {
            auto result = future.get();
            if (std::get<0>(result)) {
                std::lock_guard<std::mutex> lock(resultMutex);
                m_contourDatas.push_back(std::get<1>(result));
                const auto& tangentLines = std::get<2>(result);
                const auto& endPoints = std::get<3>(result);
                m_lines.insert(m_lines.end(), tangentLines.begin(), tangentLines.end());
                m_endPoints.insert(m_endPoints.end(), endPoints.begin(), endPoints.end());
            }
        }
    }
}


