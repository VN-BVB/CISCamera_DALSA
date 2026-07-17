#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QObject>
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

// 手动读取 8-bit 灰度 BMP，绕过 OpenCV 的 CV_IO_MAX_IMAGE_PIXELS 限制
static cv::Mat readLargeBMP(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) return cv::Mat();

    // BITMAPFILEHEADER (14 bytes)
    uint16_t bfType;
    uint32_t bfOffBits;
    file.read(reinterpret_cast<char*>(&bfType), 2);
    if (bfType != 0x4D42) return cv::Mat();  // 不是 'BM'
    file.seekg(8, std::ios::cur);            // 跳过 fileSize(4) + reserved(4)
    file.read(reinterpret_cast<char*>(&bfOffBits), 4);

    // BITMAPINFOHEADER (40 bytes)
    uint32_t biWidth, biHeight;
    uint16_t biBitCount;
    uint32_t biCompression;
    file.seekg(4, std::ios::cur);  // 跳过 headerSize(4)
    file.read(reinterpret_cast<char*>(&biWidth), 4);
    file.read(reinterpret_cast<char*>(&biHeight), 4);
    file.seekg(2, std::ios::cur);  // 跳过 planes(2)
    file.read(reinterpret_cast<char*>(&biBitCount), 2);
    file.read(reinterpret_cast<char*>(&biCompression), 4);

    // 只支持 8-bit 未压缩灰度图
    if (biBitCount != 8 || biCompression != 0) return cv::Mat();

    // BMP 每行补齐到 4 字节边界
    int stride = ((biWidth + 3) / 4) * 4;
    std::vector<uint8_t> rawData(stride * biHeight);
    file.seekg(bfOffBits, std::ios::beg);
    file.read(reinterpret_cast<char*>(rawData.data()), stride * biHeight);

    // BMP 是 bottom-up 存储，需要翻转
    cv::Mat img(biHeight, biWidth, CV_8UC1);
    for (uint32_t r = 0; r < biHeight; r++) {
        memcpy(img.ptr(r), rawData.data() + (biHeight - 1 - r) * stride, biWidth);
    }
    return img;
}
