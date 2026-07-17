#include <iostream>
#include <limits>
#include "joint_seam.h"
#include "src/utils/image_tools.h"
#include "src/jointDetection/edgeDetection/canny_zernike_detector.h"
#include "contourProcess/contour_processor.h"
#include "src/utils/scoped_timer.h"
#include "src/utils/ThreadPool.h"

JointSeam::JointSeam(const cv::Mat &image, const cv::Point2f position, const int id)
    : m_image(image),
    m_position(position),
    m_id(id)
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
        std::vector<std::future<std::tuple<bool, ContourData, std::vector<cv::Vec4f>, std::vector<ContourIntersection>>>> results;
        std::mutex resultMutex;

        // 将每条轮廓处理任务提交到线程池
        for (size_t contourIndex = 0; contourIndex < contours.size(); ++contourIndex) {
            const auto& contour = contours[contourIndex];
            int contourId = m_id * 2 + contourIndex;
            results.emplace_back(pool.enqueue([contour, contourId]()
                                              -> std::tuple<bool, ContourData,
                                                            std::vector<cv::Vec4f>,
                                                            std::vector<ContourIntersection>> {
                ContourProcessor processor;
                if (processor.processContour(contour, contourId)) {
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
                const auto& contourIntersections = std::get<3>(result);
                m_lines.insert(m_lines.end(), tangentLines.begin(), tangentLines.end());

                // 从ContourIntersection中提取信息并创建SeamEndpoint对象
                for (const auto& intersection : contourIntersections) {
                    SeamEndpoint seamEndpoint;
                    seamEndpoint.id = intersection.id;                    // 端点ID使用交点ID
                    seamEndpoint.coordinates = intersection.coordinates;  // 端点坐标
                    seamEndpoint.contourId = intersection.contourId;      // 交点所属轮廓ID

                    m_endPoints.push_back(seamEndpoint);
                }
            }
        }
    }

    // 计算端点之间的对应关系
    calculateEndpointCorrespondences();
}

void JointSeam::calculateEndpointCorrespondences() {
    if (m_endPoints.size() < 2) return;

    // 创建标记数组，用于记录端点是否已配对
    std::vector<bool> isPaired(m_endPoints.size(), false);

    // 依次遍历端点，寻找最近的未配对端点
    for (size_t i = 0; i < m_endPoints.size(); ++i) {
        if (isPaired[i]) continue;

        const auto& endpoint1 = m_endPoints[i];
        int nearestIndex = -1;
        float minDistance = std::numeric_limits<float>::max();

        // 寻找最近的未配对端点（属于不同轮廓）
        for (size_t j = i + 1; j < m_endPoints.size(); ++j) {
            if (isPaired[j]) continue;
            const auto& endpoint2 = m_endPoints[j];
            // 只配对不同轮廓的端点
            if (endpoint1.contourId != endpoint2.contourId) {
                float dx = endpoint1.coordinates.x - endpoint2.coordinates.x;
                float dy = endpoint1.coordinates.y - endpoint2.coordinates.y;
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance < minDistance) {
                    minDistance = distance;
                    nearestIndex = static_cast<int>(j);
                }
            }
        }

        if (nearestIndex != -1) {
            m_endPoints[i].correspondingIntersectionId = m_endPoints[nearestIndex].id;
            m_endPoints[nearestIndex].correspondingIntersectionId = m_endPoints[i].id;
            isPaired[i] = true;
            isPaired[nearestIndex] = true;
        }
    }
}


