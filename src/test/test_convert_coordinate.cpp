#include <Eigen/Core>
#include <plog/Log.h>

#include "test_convert_coordinate.h"
#include "src/telecentricLineCalibrator/libcbdetect/lib_cb_detecor.h"
#include "src/ui/CISCamera_imageGrab/cameraImage_processor.h"
#include "src/config/calibration_data_io.h"

TestConvertCoordinate::TestConvertCoordinate() {}

void TestConvertCoordinate::pixel2World()
{
    std::vector<Eigen::Vector2d> pix_pts;
    pix_pts.push_back(Eigen::Vector2d(100.0, 150.0));
    // pix_pts.push_back(Eigen::Vector2d(200.0, 250.0));
    pix_pts.push_back(Eigen::Vector2d(300.0, 350.0));
    // bool k = readPointsFromTxt("D:/Qt_Project/CISCamera_DALSA/data/PaltfromCalibrate/orignCor/txt/Splice_20251108_160806374.txt", pix_pts);

    std::shared_ptr<CameraImageProcessor> imageProcessor;
    imageProcessor = std::make_shared<CameraImageProcessor>();
    imageProcessor->initCameraCalibrator();
    std::vector<Eigen::Vector2d> worldPoints = imageProcessor->convertToWorld(pix_pts);
    // QMetaObject::invokeMethod(imageProcessor.get(), [=]() { imageProcessor->convertToWorld(pix_pts); }, Qt::QueuedConnection);
    PLOG_INFO << "convert done";
}
