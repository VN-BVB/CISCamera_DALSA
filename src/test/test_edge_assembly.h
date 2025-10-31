#ifndef TEST_EDGE_ASSEMBLY_H
#define TEST_EDGE_ASSEMBLY_H

#include "src/workpieceEdgeAssembly/edge_assembly.h"

class TestEdgeAssembly
{
public:
    TestEdgeAssembly();
    std::vector<cv::Point2f> contourLeft(const cv::Point2f& offset = cv::Point2f(0, 0));
    std::vector<cv::Point2f> contourRight(const cv::Point2f& offset = cv::Point2f(0, 0));
    std::vector<cv::Point2f> contourUp(const cv::Point2f& offset = cv::Point2f(0, 0));
    std::vector<cv::Point2f> contourDown(const cv::Point2f& offset = cv::Point2f(0, 0));
    ContourData setContourData(cv::Point2f offset, int direction);
    void generateNineSeams();
    void generateFiveSeams();
    void run();

    std::vector<ContourData> m_cDatas;


};

#endif // TEST_EDGE_ASSEMBLY_H
