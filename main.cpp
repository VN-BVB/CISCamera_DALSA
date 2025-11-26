#include <plog/Init.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Log.h>

#include <QApplication>
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
#include "src/test/test_cad_view.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    CrashHandler::Init(L"data/debug");  // 初始化Mini转储
    PlogUtils::initPlog();              // 初始化日志类
    // CISWidget w;
    // w.show();

    // test_multiRoi t;
    // t.show();

    TestCADView t;
    t.show();

    // JointView w;
    // w.show();

    return a.exec();
}
void useDemo() {
    // 放类成员
    std::shared_ptr<CISWidget> test{nullptr};
    // 放构造
    test = std::make_shared<CISWidget>();
    // 给图像坐标
    std::vector<Eigen::Vector2d> pix_pts;
    // 调用
    QMetaObject::invokeMethod(test.get(), [=]() { test->convertToWorldDemo(pix_pts); }, Qt::QueuedConnection);
}
