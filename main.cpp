#include <QApplication>
// clang-format off
#include <winsock2.h>
#include <windows.h>
// clang-format on
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
#include "src/utils/plog_utils.h"
#include "src/ui/measurementMonitor/measurement_monitor.h"
#include "src/ui/measurementMonitor/qss_loader.h"
#include "src/ui/mainwindow/main_window.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setStyleSheet(qss_loader::load(":/mm/measurement_monitor.qss"));
    CrashHandler::Init(L"data/debug");  // 初始化Mini转储
    PlogUtils::initPlog();              // 初始化日志类

    // 加载配置文件
    if (!ConfigManager::getInstance().loadAllConfigs()) {
        PLOG_ERROR << "Failed to load configuration files";
    }

    // CISWidget w;
    // w.show();

    // TestConvertCoordinate t;
    // t.pixel2World();

    // JointView w;
    // w.show();

    MainWindow w;
    w.show();

    // MeasurementMonitor m;
    // m.show();

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
