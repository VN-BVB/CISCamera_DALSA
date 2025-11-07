#include "test_edge_assembly.h"
#include <cmath>

TestEdgeAssembly::TestEdgeAssembly(){}

std::vector<cv::Point2f> TestEdgeAssembly::contourLeft(const cv::Point2f& offset, double rotationAngle)
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

    // 计算轮廓的最小外接矩形中心点
    float minX = originalPoints[0].x, maxX = originalPoints[0].x;
    float minY = originalPoints[0].y, maxY = originalPoints[0].y;

    for (const auto& point : originalPoints) {
        if (point.x < minX) minX = point.x;
        if (point.x > maxX) maxX = point.x;
        if (point.y < minY) minY = point.y;
        if (point.y > maxY) maxY = point.y;
    }

    cv::Point2f center((minX + maxX) / 2.0f, (minY + maxY) / 2.0f);

    // 将角度转换为弧度
    double angleRad = rotationAngle * CV_PI / 180.0;
    double cosAngle = std::cos(angleRad);
    double sinAngle = std::sin(angleRad);

    // 应用旋转和偏移到所有点
    std::vector<cv::Point2f> contourPoints;
    for (const auto &point : originalPoints)
    {
        // 将点平移到以中心点为原点的坐标系
        float translatedX = point.x - center.x;
        float translatedY = point.y - center.y;

        // 在中心点坐标系中旋转点
        double rotatedX = translatedX * cosAngle - translatedY * sinAngle;
        double rotatedY = translatedX * sinAngle + translatedY * cosAngle;

        // 将点平移回原坐标系，并应用偏移量
        contourPoints.push_back(cv::Point2f(
            static_cast<float>(rotatedX + center.x + offset.x),
            static_cast<float>(rotatedY + center.y + offset.y)
            ));
    }
    return contourPoints;
}

std::vector<cv::Point2f> TestEdgeAssembly::contourRight(const cv::Point2f& offset, double rotationAngle)
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

    // 计算轮廓的最小外接矩形中心点
    float minX = originalPoints[0].x, maxX = originalPoints[0].x;
    float minY = originalPoints[0].y, maxY = originalPoints[0].y;

    for (const auto& point : originalPoints) {
        if (point.x < minX) minX = point.x;
        if (point.x > maxX) maxX = point.x;
        if (point.y < minY) minY = point.y;
        if (point.y > maxY) maxY = point.y;
    }

    cv::Point2f center((minX + maxX) / 2.0f, (minY + maxY) / 2.0f);

    // 将角度转换为弧度
    double angleRad = rotationAngle * CV_PI / 180.0;
    double cosAngle = std::cos(angleRad);
    double sinAngle = std::sin(angleRad);

    // 应用旋转和偏移到所有点
    std::vector<cv::Point2f> contourPoints;
    for (const auto &point : originalPoints)
    {
        // 将点平移到以中心点为原点的坐标系
        float translatedX = point.x - center.x;
        float translatedY = point.y - center.y;

        // 在中心点坐标系中旋转点
        double rotatedX = translatedX * cosAngle - translatedY * sinAngle;
        double rotatedY = translatedX * sinAngle + translatedY * cosAngle;

        // 将点平移回原坐标系，并应用偏移量
        contourPoints.push_back(cv::Point2f(
            static_cast<float>(rotatedX + center.x + offset.x),
            static_cast<float>(rotatedY + center.y + offset.y)
            ));
    }

    return contourPoints;
}

std::vector<cv::Point2f> TestEdgeAssembly::contourUp(const cv::Point2f& offset, double rotationAngle)
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
    // 计算轮廓的最小外接矩形中心点
    float minX = originalPoints[0].x, maxX = originalPoints[0].x;
    float minY = originalPoints[0].y, maxY = originalPoints[0].y;

    for (const auto& point : originalPoints) {
        if (point.x < minX) minX = point.x;
        if (point.x > maxX) maxX = point.x;
        if (point.y < minY) minY = point.y;
        if (point.y > maxY) maxY = point.y;
    }

    cv::Point2f center((minX + maxX) / 2.0f, (minY + maxY) / 2.0f);

    // 将角度转换为弧度
    double angleRad = rotationAngle * CV_PI / 180.0;
    double cosAngle = std::cos(angleRad);
    double sinAngle = std::sin(angleRad);

    // 应用旋转和偏移到所有点
    std::vector<cv::Point2f> contourPoints;
    for (const auto &point : originalPoints)
    {
        // 将点平移到以中心点为原点的坐标系
        float translatedX = point.x - center.x;
        float translatedY = point.y - center.y;

        // 在中心点坐标系中旋转点
        double rotatedX = translatedX * cosAngle - translatedY * sinAngle;
        double rotatedY = translatedX * sinAngle + translatedY * cosAngle;

        // 将点平移回原坐标系，并应用偏移量
        contourPoints.push_back(cv::Point2f(
            static_cast<float>(rotatedX + center.x + offset.x),
            static_cast<float>(rotatedY + center.y + offset.y)
            ));
    }
    return contourPoints;
}

std::vector<cv::Point2f> TestEdgeAssembly::contourDown(const cv::Point2f& offset, double rotationAngle)
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
    // 计算轮廓的最小外接矩形中心点
    float minX = originalPoints[0].x, maxX = originalPoints[0].x;
    float minY = originalPoints[0].y, maxY = originalPoints[0].y;

    for (const auto& point : originalPoints) {
        if (point.x < minX) minX = point.x;
        if (point.x > maxX) maxX = point.x;
        if (point.y < minY) minY = point.y;
        if (point.y > maxY) maxY = point.y;
    }

    cv::Point2f center((minX + maxX) / 2.0f, (minY + maxY) / 2.0f);

    // 将角度转换为弧度
    double angleRad = rotationAngle * CV_PI / 180.0;
    double cosAngle = std::cos(angleRad);
    double sinAngle = std::sin(angleRad);

    // 应用旋转和偏移到所有点
    std::vector<cv::Point2f> contourPoints;
    for (const auto &point : originalPoints)
    {
        // 将点平移到以中心点为原点的坐标系
        float translatedX = point.x - center.x;
        float translatedY = point.y - center.y;

        // 在中心点坐标系中旋转点
        double rotatedX = translatedX * cosAngle - translatedY * sinAngle;
        double rotatedY = translatedX * sinAngle + translatedY * cosAngle;

        // 将点平移回原坐标系，并应用偏移量
        contourPoints.push_back(cv::Point2f(
            static_cast<float>(rotatedX + center.x + offset.x),
            static_cast<float>(rotatedY + center.y + offset.y)
            ));
    }
    return contourPoints;
}

ContourData TestEdgeAssembly::setContourData(cv::Point2f offset, int direction, double rotationAngle)
{
    // 左：0，右：1，上：2，下：3
    ContourData cd;
    switch (direction) {
    case 0:
    {
        std::vector<cv::Point2f> contourPoint = contourLeft(offset, rotationAngle);
        cd.setSubpixelContour(contourPoint);
        break;
    }
    case 1:
    {
        std::vector<cv::Point2f> contourPoint = contourRight(offset, rotationAngle);
        cd.setSubpixelContour(contourPoint);
        break;
    }
    case 2:
    {
        std::vector<cv::Point2f> contourPoint = contourUp(offset, rotationAngle);
        cd.setSubpixelContour(contourPoint);
        break;
    }
    case 3:
    {
        std::vector<cv::Point2f> contourPoint = contourDown(offset, rotationAngle);
        cd.setSubpixelContour(contourPoint);
        break;
    }
    default:
        break;
    }
    return cd;
}

void TestEdgeAssembly::generateNineSeams()
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

void TestEdgeAssembly::generateFiveSeams()
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
    offsetPoint = cv::Point2f(1400,600);
    cd = setContourData(offsetPoint,2);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(1400,710);
    cd = setContourData(offsetPoint,3);
    cDatas.push_back(cd);
    // 拼缝3
    offsetPoint = cv::Point2f(300,1000);
    cd = setContourData(offsetPoint,0);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(410,1000);
    cd = setContourData(offsetPoint,1);
    cDatas.push_back(cd);
    // 拼缝4
    offsetPoint = cv::Point2f(100,600);
    cd = setContourData(offsetPoint,2);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(100,710);
    cd = setContourData(offsetPoint,3);
    cDatas.push_back(cd);
    // 拼缝5
    offsetPoint = cv::Point2f(100,300);
    cd = setContourData(offsetPoint,2);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(100,410);
    cd = setContourData(offsetPoint,3);
    cDatas.push_back(cd);

    m_cDatas = cDatas;
}

void TestEdgeAssembly::generateTiltedNineSeams()
{
    std::vector<ContourData> cDatas;
    ContourData cd;
    cv::Point2f offsetPoint;
    // 拼缝1
    offsetPoint = cv::Point2f(500,10);
    cd = setContourData(offsetPoint,0,10);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(610,10);
    cd = setContourData(offsetPoint,1,10);
    cDatas.push_back(cd);
    // 拼缝2
    offsetPoint = cv::Point2f(1400,800);
    cd = setContourData(offsetPoint,2,16);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(1400,910);
    cd = setContourData(offsetPoint,3,16);
    cDatas.push_back(cd);
    // 拼缝3
    offsetPoint = cv::Point2f(1000,1100);
    cd = setContourData(offsetPoint,0,20);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(1110,1100);
    cd = setContourData(offsetPoint,1,20);
    cDatas.push_back(cd);
    // 拼缝4
    offsetPoint = cv::Point2f(1400,1400);
    cd = setContourData(offsetPoint,2,-10);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(1400,1510);
    cd = setContourData(offsetPoint,3,-10);
    cDatas.push_back(cd);
    // 拼缝5
    offsetPoint = cv::Point2f(900,2400);
    cd = setContourData(offsetPoint,0,-13);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(1010,2400);
    cd = setContourData(offsetPoint,1,-13);
    cDatas.push_back(cd);
    // 拼缝6
    offsetPoint = cv::Point2f(100,2000);
    cd = setContourData(offsetPoint,2,-30);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(100,2110);
    cd = setContourData(offsetPoint,3,-30);
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
    cd = setContourData(offsetPoint,0,25);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(410,900);
    cd = setContourData(offsetPoint,1,25);
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
    cd = setContourData(offsetPoint,2,15);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(100,410);
    cd = setContourData(offsetPoint,3,15);
    cDatas.push_back(cd);

    m_cDatas = cDatas;
}

void TestEdgeAssembly::generateTiltedFiveSeams()
{
    std::vector<ContourData> cDatas;
    ContourData cd;
    cv::Point2f offsetPoint;
    // 拼缝1
    offsetPoint = cv::Point2f(500,10);
    cd = setContourData(offsetPoint, 0, -10);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(610,10);
    cd = setContourData(offsetPoint, 1, -10);
    cDatas.push_back(cd);
    // 拼缝2
    offsetPoint = cv::Point2f(1400,600);
    cd = setContourData(offsetPoint,2, 20);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(1400,710);
    cd = setContourData(offsetPoint,3, 20);
    cDatas.push_back(cd);
    // 拼缝3
    offsetPoint = cv::Point2f(300,1000);
    cd = setContourData(offsetPoint,0, 30);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(430,1000);
    cd = setContourData(offsetPoint,1, 30);
    cDatas.push_back(cd);
    // 拼缝4
    offsetPoint = cv::Point2f(100,600);
    cd = setContourData(offsetPoint,2, 35);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(100,750);
    cd = setContourData(offsetPoint,3, 35);
    cDatas.push_back(cd);
    // 拼缝5
    offsetPoint = cv::Point2f(100,300);
    cd = setContourData(offsetPoint,2, 13);
    cDatas.push_back(cd);
    offsetPoint = cv::Point2f(100,410);
    cd = setContourData(offsetPoint,3, 13);
    cDatas.push_back(cd);

    m_cDatas = cDatas;
}

void TestEdgeAssembly::run()
{
    // generateNineSeams();
    generateTiltedNineSeams();
    // generateTiltedFiveSeams();
    // generateFiveSeams();
    std::vector<std::shared_ptr<ContourBoundingBox>> cbbs;
    
    int i = 0;
    for (auto& cData : m_cDatas)
    {
        std::shared_ptr<ContourBoundingBox> cbb = std::make_shared<ContourBoundingBox>();
        cbb->initContourData(i++, cData);
        cbbs.push_back(cbb);
    }


    EdgeAssembly edgeAssembly = EdgeAssembly{cbbs};
    edgeAssembly.run(9);
    // for (auto& workpiece : edgeAssembly.getpossibleWorkpieces())
    // {
    //     m_workpieceRotateRect.push_back(workpiece.getouterBoundingBox());
    // }
    auto combinations = edgeAssembly.getValidCombinations();
    auto combination = combinations[0];
    combination = edgeAssembly.getMostLikelyCombination();
    auto workpieces = edgeAssembly.getpossibleWorkpieces();
    for (auto& workpiecID :  combination)
    {
        m_workpieceRotateRect.push_back((workpieces[workpiecID].getouterBoundingBox()));
    }
}













