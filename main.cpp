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

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    CrashHandler::Init(L"data/debug");      // 初始化Mini转储
    PlogUtils::initPlog();                  // 初始化日志类

    // TinySplineqqq testSpline;
    // testSpline.runcv();

    // test_FrmVisionDisplay testDisplay;
    // testDisplay.show();

    JointView w;
    w.show();

    return a.exec();
}


