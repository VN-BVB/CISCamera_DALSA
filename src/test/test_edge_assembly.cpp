#include "test_edge_assembly.h"

TestEdgeAssembly::TestEdgeAssembly(){}

std::vector<cv::Point2f> TestEdgeAssembly::contourLeft(const cv::Point2f& offset)
{
    // 定义轮廓的原始点集（按顺序排列的坐标）
    std::vector<cv::Point2f> originalPoints = {
        cv::Point2f(0, 0),  // 左上角
        cv::Point2f(50, 0),
        cv::Point2f(100, 0),  // 右上角
        cv::Point2f(100, 100),
        cv::Point2f(100, 200),  // 右下角
        cv::Point2f(50, 200),
        cv::Point2f(0, 200)   // 左下角
    };

    // 应用偏移量到所有点
    std::vector<cv::Point2f> contourPoints;
    for (const auto& point : originalPoints) {
        contourPoints.push_back(cv::Point2f(point.x + offset.x, point.y + offset.y));
    }

    return contourPoints;
}

std::vector<cv::Point2f> TestEdgeAssembly::contourRight(const cv::Point2f& offset)
{
    // 定义轮廓的原始点集（按顺序排列的坐标）
    std::vector<cv::Point2f> originalPoints = {
        cv::Point2f(100, 0),  // 右上角
        cv::Point2f(50, 0),
        cv::Point2f(0, 0),  // 左上角
        cv::Point2f(0, 100),
        cv::Point2f(0, 200),  // 左下角
        cv::Point2f(50, 200),
        cv::Point2f(100, 200)   // 右下角
    };

    // 应用偏移量到所有点
    std::vector<cv::Point2f> contourPoints;
    for (const auto& point : originalPoints) {
        contourPoints.push_back(cv::Point2f(point.x + offset.x, point.y + offset.y));
    }

    return contourPoints;
}

std::vector<cv::Point2f> TestEdgeAssembly::contourUp(const cv::Point2f& offset)
{
    // 定义轮廓的原始点集（按顺序排列的坐标）
    std::vector<cv::Point2f> originalPoints = {
        cv::Point2f(0, 0),  // 左上角
        cv::Point2f(0, 50),
        cv::Point2f(0, 100),  // 左下角
        cv::Point2f(100, 100),
        cv::Point2f(200, 100),  // 右下角
        cv::Point2f(200, 50),
        cv::Point2f(200, 0)   // 右上角
    };

    // 应用偏移量到所有点
    std::vector<cv::Point2f> contourPoints;
    for (const auto& point : originalPoints) {
        contourPoints.push_back(cv::Point2f(point.x + offset.x, point.y + offset.y));
    }

    return contourPoints;
}

std::vector<cv::Point2f> TestEdgeAssembly::contourDown(const cv::Point2f& offset)
{
    // 定义轮廓的原始点集（按顺序排列的坐标）
    std::vector<cv::Point2f> originalPoints = {
        cv::Point2f(0, 100),  // 左下角
        cv::Point2f(0, 50),
        cv::Point2f(0, 0),  // 左上角
        cv::Point2f(100, 0),
        cv::Point2f(200, 0),  // 右上角
        cv::Point2f(200, 50),
        cv::Point2f(200, 100)   // 右下角
    };

    // 应用偏移量到所有点
    std::vector<cv::Point2f> contourPoints;
    for (const auto& point : originalPoints) {
        contourPoints.push_back(cv::Point2f(point.x + offset.x, point.y + offset.y));
    }

    return contourPoints;
}

ContourData TestEdgeAssembly::setContourData(cv::Point2f offset, int direction)
{
    // 左：0，右：1，上：2，下：3
    ContourData cd;
    switch (direction) {
    case 0:
    {
        std::vector<cv::Point2f> contourPoint = contourLeft(offset);
        cd.setSubpixelContour(contourPoint);
        break;
    }
    case 1:
    {
        std::vector<cv::Point2f> contourPoint = contourRight(offset);
        cd.setSubpixelContour(contourPoint);
        break;
    }
    case 2:
    {
        std::vector<cv::Point2f> contourPoint = contourUp(offset);
        cd.setSubpixelContour(contourPoint);
        break;
    }
    case 3:
    {
        std::vector<cv::Point2f> contourPoint = contourDown(offset);
        cd.setSubpixelContour(contourPoint);
        break;
    }
    default:
        break;
    }
    return cd;
}

void TestEdgeAssembly::generateSeams()
{
    std::vector<ContourData> cDatas;
    ContourData cd;
    cv::Point2f offsetPoint;
    // 拼缝1
    offsetPoint = cv::Point2f(500,10);
    cd = setContourData(offsetPoint,0);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(610,10);
    cd = setContourData(offsetPoint,1);
    cDatas.push_back(cd);
    // 拼缝2
    offsetPoint = cv::Point2f(1400,800);
    cd = setContourData(offsetPoint,2);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(1400,910);
    cd = setContourData(offsetPoint,3);
    cDatas.push_back(cd);
    // 拼缝3
    offsetPoint = cv::Point2f(1000,1100);
    cd = setContourData(offsetPoint,0);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(1110,1100);
    cd = setContourData(offsetPoint,1);
    cDatas.push_back(cd);
    // 拼缝4
    offsetPoint = cv::Point2f(1400,1400);
    cd = setContourData(offsetPoint,2);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(1400,1510);
    cd = setContourData(offsetPoint,3);
    cDatas.push_back(cd);
    // 拼缝5
    offsetPoint = cv::Point2f(900,2400);
    cd = setContourData(offsetPoint,0);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(1010,2400);
    cd = setContourData(offsetPoint,1);
    cDatas.push_back(cd);
    // 拼缝6
    offsetPoint = cv::Point2f(100,2000);
    cd = setContourData(offsetPoint,2);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(100,2110);
    cd = setContourData(offsetPoint,3);
    cDatas.push_back(cd);
    // 拼缝7
    offsetPoint = cv::Point2f(100,1200);
    cd = setContourData(offsetPoint,2);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(100,1310);
    cd = setContourData(offsetPoint,3);
    cDatas.push_back(cd);
    // 拼缝8
    offsetPoint = cv::Point2f(300,900);
    cd = setContourData(offsetPoint,0);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(410,900);
    cd = setContourData(offsetPoint,1);
    cDatas.push_back(cd);
    // 拼缝9
    offsetPoint = cv::Point2f(100,600);
    cd = setContourData(offsetPoint,2);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(100,710);
    cd = setContourData(offsetPoint,3);
    cDatas.push_back(cd);
    // 拼缝10
    offsetPoint = cv::Point2f(100,300);
    cd = setContourData(offsetPoint,2);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(100,410);
    cd = setContourData(offsetPoint,3);
    cDatas.push_back(cd);

    m_cDatas = cDatas;
}

void TestEdgeAssembly::run()
{
    generateSeams();
    std::vector<std::shared_ptr<ContourBoundingBox>> cbbs;
    
    int i = 0;
    for (auto& cData : m_cDatas)
    {
        std::shared_ptr<ContourBoundingBox> cbb = std::make_shared<ContourBoundingBox>();
        cbb->initContourData(i++, cData);
        cbbs.push_back(cbb);
    }

    EdgeAssembly edgeAssembly = EdgeAssembly{cbbs};
    edgeAssembly.generateWorkpiece();
    // edgeAssembly.validateCombinations(9);
    return ;
}













