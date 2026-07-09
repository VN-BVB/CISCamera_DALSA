#include <QApplication>
#include <QDebug>
// clang-format off
#include <winsock2.h>
#include <windows.h>
// clang-format on
#include <windows.h>

#include <fstream>

#include "src/config/config_manager.h"
#include "src/crashHandler/CrashHandler.h"
#include "src/test/test_cad_view.h"'
#include "src/test/test_convert_coordinate.h"
#include "src/test/test_dxf_writer.h"
#include "src/test/test_frmVisionDisplay.h"
#include "src/test/test_multiRoi.h"
#include "src/test/test_pixel2world.h"
#include "src/test/test_platform_pose_dxf.h"
#include "src/test/test_tiny_spline.h"
#include "src/ui/CISCamera_imageGrab/cis_camera_image.h"
#include "src/ui/jointView/joint_view.h"
#include "src/ui/mainwindow/main_window.h"
#include "src/ui/measurementMonitor/qss_loader.h"
#include "src/utils/plog_utils.h"

int main(int argc, char *argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    QApplication a(argc, argv);
    a.setStyleSheet(qss_loader::load(":/mm/measurement_monitor.qss"));
    CrashHandler::Init(L"data/debug");  // 初始化Mini转储
    PlogUtils::initPlog();              // 初始化日志类

    // 加载配置文件
    if (!ConfigManager::getInstance().loadAllConfigs()) {
        PLOG_ERROR << "Failed to load configuration files";
    }

    Eigen::Vector2d pix1(225.7406, 555.4853);
    std::vector<Eigen::Vector2d> pix2{Eigen::Vector2d(8779, 5557)};
    CameraImageProcessor test;
    test.initCameraCalibrator();
    PLOG_INFO << "===========" << test.convertToWorld(pix2)[0];

    const Eigen::Vector2d origPix = pix2[0];
    const Eigen::Vector2d worldPt = test.convertToWorld(pix2)[0];
    Eigen::Vector2d reb = test.applyWorldOffsetToPixel(worldPt);

    PLOG_INFO << std::fixed << std::setprecision(2) << "虚拟移动验证: 处理前像素(" << origPix.x() << ", " << origPix.y() << ") → 处理后像素("
              << reb.x() << ", " << reb.y() << ")";

    test.debugProjectionDistancesFromFixedPixels();

    CISWidget w;
    w.show();

    // TestConvertCoordinate t;
    // t.pixel2World();

    // JointView w;
    // w.show();

    // std::shared_ptr<CameraImageProcessor> imageProcessor;
    // imageProcessor = std::make_shared<CameraImageProcessor>();
    // imageProcessor->initCameraCalibrator();

    // TestPixel2World test;
    // test.testPixel2World();

    // TestPlatformPoseDxf tp;
    // tp.runDirect();

    // TestDxfWriter td;
    // td.run();
    // JointView w;
    // w.show();

    // JointView w;
    // w.show();

    // MainWindow w;
    // w.show();

    return a.exec();
}
// void useDemo() {
//     // 放类成员
//     std::shared_ptr<CISWidget> test{nullptr};
//     // 放构造
//     test = std::make_shared<CISWidget>();
//     // 给图像坐标
//     std::vector<Eigen::Vector2d> pix_pts;
//     std::vector<std::vector<std::vector<cv::Point2d>>> calcOriCoordinateSystem;
//     LibCBDetector libcbDetector;
//     libcbDetector.processImagesInDirectoryFilePath("./data/PaltfromCalibrate/orignCor/img", calcOriCoordinateSystem);
//     for (auto& p : calcOriCoordinateSystem[0][0]) {
//         pix_pts.emplace_back(p.x, p.y);
//     }
//     // 调用
//     QMetaObject::invokeMethod(test.get(), [=]() { test->convertToWorldDemo(pix_pts); }, Qt::QueuedConnection);
// }
