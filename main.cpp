#include <QApplication>
#include <plog/Init.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Log.h>

// clang-format off
#include <winsock2.h>
#include <windows.h>
// clang-format on
#include "src/crashHandler/CrashHandler.h"
#include "src/utils/plog_utils.h"
#include "src/ui/CISCamera_imageGrab/cis_camera_image.h"
#include "src/test/test_tiny_spline.h"
#include "src/ui/jointView/joint_view.h"
#include "src/test/test_frmVisionDisplay.h"
#include "src/test/test_multiRoi.h"
#include "src/test/test_cad_view.h"'
#include "src/test/test_dxf_writer.h"
#include "src/test/test_convert_coordinate.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    CrashHandler::Init(L"data/debug");  // 初始化Mini转储
    PlogUtils::initPlog();              // 初始化日志类
    // CISWidget w;
    // w.show();

    // TestConvertCoordinate t;
    // t.pixel2World();

    JointView w;
    w.show();

    return a.exec();
}
void useDemo() {
    // 放类成员
    std::shared_ptr<CISWidget> test{nullptr};
    // 放构造
    test = std::make_shared<CISWidget>();
    // 给图像坐标
    std::vector<Eigen::Vector2d> pix_pts;
    std::vector<std::vector<std::vector<cv::Point2d>>> calcOriCoordinateSystem;
    LibCBDetector libcbDetector;
    libcbDetector.processImagesInDirectoryFilePath("./data/PaltfromCalibrate/orignCor/img", calcOriCoordinateSystem);
    for (auto& p : calcOriCoordinateSystem[0][0]) {
        pix_pts.emplace_back(p.x, p.y);
    }
    // 调用
    QMetaObject::invokeMethod(test.get(), [=]() { test->convertToWorldDemo(pix_pts); }, Qt::QueuedConnection);
}
