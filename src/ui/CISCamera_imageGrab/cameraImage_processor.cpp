#include "cameraImage_processor.h"

#include "src/telecentricLineCalibrator/libcbdetect/lib_cb_detecor.h"
#include "src/telecentricLineCalibrator/telecentric_line_calibrator.h"
#include "src/telecentricLineCalibrator/telecentricplatform_calib.h"
CameraImageProcessor::CameraImageProcessor(QObject* parent) : QObject(parent) {}

CameraImageProcessor::~CameraImageProcessor() {}
void CameraImageProcessor::initCameraCalibrator() {
    lodaCameraCalibrateParams();
    lodaCam2PlatCalibrateParams();
    libcbDetector = std::make_shared<LibCBDetector>();
    telecentricLineCalibrator = std::make_shared<TelecentricLineCalibrator>();
    telecentricPlatCalibrator =
        std::make_shared<TelecentricPlatformCalib>(K_, coff_dis_, allRotVecs_.back(), allTransVecs_.back());
    connect(this, &CameraImageProcessor::sendSignalToCalibrate, telecentricLineCalibrator.get(),
            &TelecentricLineCalibrator::calibrateCameraFromPointsDemo, Qt::QueuedConnection);
    connect(telecentricLineCalibrator.get(), &TelecentricLineCalibrator::sendSignalSuccessCalib, this,
            &CameraImageProcessor::lodaCameraCalibrateParams, Qt::QueuedConnection);
    worldPts.reserve(W_ * H_);
    for (int r = 0; r < H_; ++r)
        for (int c = 0; c < W_; ++c) worldPts.emplace_back(c * spacingMM_, r * spacingMM_);
}
void CameraImageProcessor::lodaCameraCalibrateParams() {
    v_rot_s.clear();
    v_trans_s.clear();
    CalibrationData calibParam;
    if (!calibParam.load("./data/calibration_config/optimized_calib_data.json")) {
        throw std::runtime_error("无法加载标定文件");
    }
    m_ = calibParam.m;
    K_ = calibParam.K;
    coff_dis_ = calibParam.coff_dis;
    v_rot_s = calibParam.v_rot;
    v_trans_s = calibParam.v_trans;
}
void CameraImageProcessor::lodaCam2PlatCalibrateParams() {
    allRotVecs_.clear();
    allTransVecs_.clear();
    PlatformPoseData cam2PlatParam;
    if (!cam2PlatParam.load("./data/calibration_config/platform_pose.json")) {
        throw std::runtime_error("无法加载标定文件");
    }
    allRotVecs_ = cam2PlatParam.allRotVecs;
    allTransVecs_ = cam2PlatParam.allTransVecs;
}
void CameraImageProcessor::setSpliceEnabled(bool enabled) {
    QMutexLocker locker(&mtx_);
    spliceEnabled_ = enabled;
}

void CameraImageProcessor::processSingle(std::shared_ptr<cv::Mat> image) {
    if (!image || image->empty()) {
        // emit error(QString(u8"单图处理：输入为空"));
        return;
    }
    {
        QMutexLocker locker(&mtx_);
        lastMaster_ = image;
        lastResult_ = image;  // 直通
    }
    emit text(QString(u8"单图处理完成，尺寸 %1x%2").arg(image->cols).arg(image->rows));
    emit imageReady(image);
}

bool CameraImageProcessor::prepareForConcat(const cv::Mat& m, const cv::Mat& s, cv::Mat& mOut, cv::Mat& sOut) {
    if (m.empty() || s.empty()) return false;

    // 统一通道数
    auto unifyChannels = [](const cv::Mat& src, int ch) -> cv::Mat {
        if (src.channels() == ch) return src;
        cv::Mat out;
        if (ch == 1) {
            if (src.channels() == 3)
                cv::cvtColor(src, out, cv::COLOR_BGR2GRAY);
            else if (src.channels() == 4)
                cv::cvtColor(src, out, cv::COLOR_BGRA2GRAY);
            else
                out = src.clone();
        } else if (ch == 3) {
            if (src.channels() == 1)
                cv::cvtColor(src, out, cv::COLOR_GRAY2BGR);
            else if (src.channels() == 4)
                cv::cvtColor(src, out, cv::COLOR_BGRA2BGR);
            else
                out = src.clone();
        } else if (ch == 4) {
            if (src.channels() == 1)
                cv::cvtColor(src, out, cv::COLOR_GRAY2BGRA);
            else if (src.channels() == 3)
                cv::cvtColor(src, out, cv::COLOR_BGR2BGRA);
            else
                out = src.clone();
        } else {
            out = src.clone();
        }
        return out;
    };

    int targetCh = std::max(m.channels(), s.channels());
    if (targetCh != 1 && targetCh != 3 && targetCh != 4) targetCh = 3;

    cv::Mat mC = unifyChannels(m, targetCh);
    cv::Mat sC = unifyChannels(s, targetCh);

    // 统一深度（避免 CV_8U / CV_16U / CV_32F 不同）
    int targetDepth = std::max(mC.depth(), sC.depth());  // 粗暴选更高精度
    if (mC.depth() != targetDepth) mC.convertTo(mC, CV_MAKETYPE(targetDepth, targetCh));
    if (sC.depth() != targetDepth) sC.convertTo(sC, CV_MAKETYPE(targetDepth, targetCh));

    // 行数对齐（取最小行数裁剪，避免插值带来误差；如需插值，可改为 resize）
    int rows = std::min(mC.rows, sC.rows);
    if (rows <= 0) return false;
    cv::Rect mRoi(0, 0, mC.cols, rows);
    cv::Rect sRoi(0, 0, sC.cols, rows);
    mOut = mC(mRoi).clone();
    sOut = sC(sRoi).clone();
    return true;
}

void CameraImageProcessor::processPair(std::shared_ptr<cv::Mat> master, std::shared_ptr<cv::Mat> slave, bool spliceEnabledFromCaller,
                                       bool useColumnCheck) {
    if (!master || master->empty()) {
        emit error(QString(u8"拼接：Master 为空"));
        return;
    }
    {
        QMutexLocker locker(&mtx_);
        lastMaster_ = master;
        lastSlave_ = slave;
    }

    bool doSplice = spliceEnabledFromCaller;
    {
        QMutexLocker locker(&mtx_);
        doSplice = doSplice && spliceEnabled_;
    }

    if (!doSplice || !slave || slave->empty()) {
        // 直通 master
        {
            QMutexLocker locker(&mtx_);
            lastResult_ = master;
        }
        emit text(QString(u8"未拼接（关闭或缺少Slave），回传Master。尺寸 %1x%2").arg(master->cols).arg(master->rows));
        emit imageReady(master);
        return;
    }

    // 准备拼接
    cv::Mat mAligned, sAligned;
    if (!prepareForConcat(*master, *slave, mAligned, sAligned)) {
        // 回退 master
        {
            QMutexLocker locker(&mtx_);
            lastResult_ = master;
        }
        emit imageReady(master);
        return;
    }

    try {
        auto result = std::make_shared<cv::Mat>();
        cv::hconcat(mAligned, sAligned, *result);

        // 转为灰度图
        cv::Mat gray;
        if (result->channels() == 3)
            cv::cvtColor(*result, gray, cv::COLOR_BGR2GRAY);
        else
            gray = *result;

        const uchar whiteThresh = 250;
        int left = 0, right = gray.cols - 1;
        if (useColumnCheck) {
            // 1. 从下往上处理全黑行，把全黑行置为白色
            for (int r = gray.rows - 1; r >= 0; --r) {
                cv::Mat row = gray.row(r);
                double maxVal;
                cv::minMaxLoc(row, nullptr, &maxVal);
                if (maxVal == 0) {
                    row.setTo(255);
                } else {
                    break;
                }
            }
            // 2. 从左找第一个非白列
            for (int c = 0; c < gray.cols; ++c) {
                bool isWhiteCol = true;
                for (int r = 0; r < gray.rows; ++r) {
                    if (gray.at<uchar>(r, c) < whiteThresh) {
                        isWhiteCol = false;
                        break;
                    }
                }
                if (!isWhiteCol) {
                    left = c;
                    break;
                }
            }

            // 3. 从右找第一个非白列
            for (int c = gray.cols - 1; c >= 0; --c) {
                bool isWhiteCol = true;
                for (int r = 0; r < gray.rows; ++r) {
                    if (gray.at<uchar>(r, c) < whiteThresh) {
                        isWhiteCol = false;
                        break;
                    }
                }
                if (!isWhiteCol) {
                    right = c;
                    break;
                }
            }
        } else {
            // 直接用固定列
            left = 208;
            right = 30895;
        }

        // 防止越界，确保宽度大于10
        if (right > left + 10) {
            cv::Rect roi(left, 0, right - left + 1, result->rows);
            *result = (*result)(roi).clone();
        }

        // 保存结果（线程安全）
        {
            QMutexLocker locker(&mtx_);
            lastResult_ = result;
        }
        emit text(QString(u8"拼接成功：%1x%2").arg(result->cols).arg(result->rows));
        emit imageReady(result);
    } catch (const cv::Exception& e) {
        emit error(QString(u8"OpenCV 拼接异常：%1").arg(e.what()));
        // 回退 master
        {
            QMutexLocker locker(&mtx_);
            lastResult_ = master;
        }
        emit imageReady(master);
    }
}

bool CameraImageProcessor::imwriteSmart(const QString& path, const cv::Mat& img, QString& err) {
    std::vector<int> params;
    const QString lower = path.toLower();
    cv::Mat img_to_save = img;  // 默认使用原图

    if (lower.endsWith(".png")) {
        params = {cv::IMWRITE_PNG_COMPRESSION, 1};  // 轻压缩，快
    } else if (lower.endsWith(".jpg") || lower.endsWith(".jpeg")) {
        params = {cv::IMWRITE_JPEG_QUALITY, 95};
    } else if (lower.endsWith(".tif") || lower.endsWith(".tiff")) {
        // LZW 无损，兼容性好；若要BigTIFF或浮点，保持imwrite默认即可
        params = {cv::IMWRITE_TIFF_COMPRESSION, 1};  // 1=LZW
    } else if (lower.endsWith(".bmp")) {
        // BMP 无参数
    } else if (lower.endsWith(".exr")) {
        // OpenEXR 只支持 CV_16U / CV_32F，不支持 CV_8U
        if (img.depth() == CV_8U) {
            img.convertTo(img_to_save, CV_32F, 1.0 / 255.0);
        } else if (img.depth() == CV_16U) {
            img.convertTo(img_to_save, CV_32F, 1.0 / 65535.0);
        }
        // 不设置压缩参数时，OpenEXR 默认使用 ZIP 无损压缩
    }

    try {
        return cv::imwrite(path.toStdString(), img_to_save, params);
    } catch (const cv::Exception& e) {
        err = QString::fromUtf8(e.what());
        return false;
    }
}

void CameraImageProcessor::saveResult(const QString& dir, const QString& prefix, const QString& ext, bool alsoSaveSingles) {
    emit text(QString(u8"正在进行保存，请稍等..."));
    std::shared_ptr<cv::Mat> toSave, m, s;
    {
        QMutexLocker locker(&mtx_);
        // cv::bitwise_not(*lastResult_, *toSave);
        toSave = lastResult_;
        m = lastMaster_;
        s = lastSlave_;
    }

    QDir d(dir);
    if (!d.exists()) d.mkpath(".");

    auto ts = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmsszzz");
    QString base = d.filePath(QString("%1_%2").arg(prefix, ts));
    QString mainPath = base + ext;

    if (!toSave || toSave->empty()) {
        emit error(QString(u8"保存失败：无可保存的结果图像"));
        return;
    }

    QString err;
    if (imwriteSmart(mainPath, *toSave, err)) {
        emit text(QString(u8"已保存：%1").arg(mainPath));
        std::cout << u8"检测是否能发送";
    } else {
        emit error(QString(u8"保存失败：%1 （%2）").arg(mainPath, err));
    }

    if (alsoSaveSingles) {
        if (m && !m->empty()) {
            QString mp = base + "_master" + ext;
            if (imwriteSmart(mp, *m, err))
                emit text(QString(u8"已保存：%1").arg(mp));
            else
                emit error(QString(u8"保存Master失败：%1 （%2）").arg(mp, err));
        }
        if (s && !s->empty()) {
            QString sp = base + "_slave" + ext;
            if (imwriteSmart(sp, *s, err))
                emit text(QString(u8"已保存：%1").arg(sp));
            else
                emit error(QString(u8"保存Slave失败：%1 （%2）").arg(sp, err));
        }
    }
}
void CameraImageProcessor::savePlatfromCailbImg(const QString& prefix, const QString& ext, int paltIndex) {
    emit text(QString(u8"正在进行保存，请稍等..."));
    QString dir = QString(readPlatfromImg_ + "%1/img/").arg(paltIndex);

    if (!all_platfromCalibImg_[paltIndex].empty()) {
        // 清空内存中旧图像
        all_platfromCalibImg_[paltIndex].clear();

        // 删除对应目录下的所有图像文件
        QDir d(dir);
        if (d.exists()) {
            QStringList imgFilters;
            imgFilters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.tif" << "*.tiff";

            QFileInfoList imgs = d.entryInfoList(imgFilters, QDir::Files);

            for (const QFileInfo& f : imgs) {
                QFile::remove(f.absoluteFilePath());
            }
        }
        emit text(QString(u8"已删除当前平台全部图像"));
    }

    std::shared_ptr<cv::Mat> toSave;
    {
        QMutexLocker locker(&mtx_);
        toSave = lastResult_;
    }

    // 创建目录
    QDir d(dir);
    if (!d.exists()) d.mkpath(".");

    auto ts = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmsszzz");
    QString base = d.filePath(QString("%1_%2").arg(prefix, ts));
    QString mainPath = base + ext;

    if (!toSave || toSave->empty()) {
        emit error(QString(u8"保存失败：无可保存的结果图像"));
        return;
    }

    QString err;
    if (imwriteSmart(mainPath, *toSave, err)) {
        emit text(QString(u8"已保存：%1").arg(mainPath));
        std::cout << u8"检测是否能发送";
    } else {
        emit error(QString(u8"保存失败：%1 （%2）").arg(mainPath, err));
    }

    all_platfromCalibImg_[paltIndex].emplace_back((*toSave).clone());
}
void CameraImageProcessor::loadPlatformCalibImages() {
    all_platfromCalibImg_.clear();
    all_platfromCalibImg_.resize(maxPlatformCount_);
    loadMode_ = true;

    QDir rootDir(readPlatfromImg_);
    if (!rootDir.exists()) {
        emit error(QString(u8"读取失败：目录不存在 %1").arg(rootDir.path()));
        return;
    }

    emit text(QString(u8"正在读取平台标定图像，请稍等..."));

    // ========== 遍历 readPlatfromImg 下的每一个子文件夹 ==========
    QFileInfoList subDirs = rootDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QFileInfo& info : subDirs) {
        // 子文件夹名称就是平台编号（如 p1、p2 或数字）
        QString subName = info.fileName();

        bool ok = false;
        int index = subName.toInt(&ok);
        if (!ok) {
            // 名字不是数字，跳过
            continue;
        }

        if (index < 0 || index >= all_platfromCalibImg_.size()) continue;

        QString imgDirPath = info.absoluteFilePath() + "/img/";
        QDir imgDir(imgDirPath);

        if (!imgDir.exists()) continue;

        // 读取所有图像文件
        QStringList filters;
        filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.tif" << "*.tiff";

        QFileInfoList imgFiles = imgDir.entryInfoList(filters, QDir::Files);

        for (const QFileInfo& imgFile : imgFiles) {
            cv::Mat img = cv::imread(imgFile.absoluteFilePath().toStdString(), cv::IMREAD_UNCHANGED);
            if (img.empty()) {
                emit error(QString(u8"读取失败：%1").arg(imgFile.absoluteFilePath()));
                continue;
            }

            all_platfromCalibImg_[index].emplace_back(img.clone());
        }

        emit text(QString(u8"平台 %1 已读取图像数：%2").arg(index).arg(all_platfromCalibImg_[index].size()));
    }

    emit text(QString(u8"平台标定图像全部读取完成"));
}
void CameraImageProcessor::whenClearPlatFromFile(const QString& subFolder) {
    emit text(QString(u8"正在清除平台目录 '%1' 下的所有文件...").arg(subFolder));

    // 清空内存中的所有图像
    all_platfromCalibImg_.clear();
    all_platfromCalibImg_.resize(maxPlatformCount_);

    QDir rootDir(readPlatfromImg_);
    if (!rootDir.exists()) {
        emit error(QString(u8"清除失败：目录不存在 %1").arg(rootDir.path()));
        return;
    }

    // 遍历平台目录
    QFileInfoList subDirs = rootDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QFileInfo& info : subDirs) {
        // 拼成 platform/subFolder/
        QString targetDirPath = info.absoluteFilePath() + "/" + subFolder + "/";
        QDir targetDir(targetDirPath);

        if (!targetDir.exists()) continue;

        // 常见格式
        QStringList filters;
        filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.tif" << "*.tiff" << "*.txt";

        QFileInfoList files = targetDir.entryInfoList(filters, QDir::Files);

        // 删除所有文件
        for (const QFileInfo& f : files) {
            QFile::remove(f.absoluteFilePath());
        }

        emit text(QString(u8"已清空：%1").arg(targetDirPath));
    }

    emit text(QString(u8"清除完成：%1").arg(subFolder));
}
void CameraImageProcessor::clear() {
    QMutexLocker locker(&mtx_);
    lastMaster_.reset();
    lastSlave_.reset();
    lastResult_.reset();
    emit text(QString(u8"处理缓存已清空"));
}

void CameraImageProcessor::whenDetectChessboard() {
    libcbDetector->test();
    std::vector<std::vector<std::vector<cv::Point2d>>> allImagesBoardsPts;
    libcbDetector->processImagesInDirectoryFilePath(readImgPath_, allImagesBoardsPts);
}
void CameraImageProcessor::whenCameraCalibrate() {
    all_image_points_.clear();
    std::string folder = "./data/CISCamera_Image/cameraCalibrate/txt";

    // 遍历文件夹读取所有txt
    for (auto& entry : std::filesystem::directory_iterator(folder)) {
        if (entry.path().extension() == ".txt") {
            std::vector<Eigen::Vector2d> pts;
            if (readPointsFromTxt(entry.path().string(), pts)) {
                all_image_points_.push_back(pts);
            }
        }
    }

    if (all_image_points_.empty()) {
        std::cerr << "没有读取到任何标定点文件！" << std::endl;
        return;
    }

    // 输出结果
    Eigen::Matrix3d K;
    double rmse;
    std::vector<Pose> poses;

    sendSignalToCalibrate(all_image_points_, worldPts, width_, height_, dx_, dy_, K, rmse, poses);
}
void CameraImageProcessor::whenCalibrateCP() {
    if (all_platfromCalibImg_.empty()) {
        loadPlatformCalibImages();
    }
    emit text(QString(u8"正在进初始世界平台的棋盘格检测与外参估计..."));
    PlatformPoseData poseData;

    int N = all_platfromCalibImg_.size();
    poseData.allRotVecs.assign(N + 1, Eigen::Vector3d(0, 0, 0));
    poseData.allTransVecs.assign(N + 1, Eigen::Vector3d(0, 0, 0));
    std::vector<std::vector<std::vector<cv::Point2d>>> calcOriCoordinateSystem;
    libcbDetector->processImagesInDirectoryFilePath("./data/PaltfromCalibrate/orignCor/img", calcOriCoordinateSystem);
    std::vector<Eigen::Vector2d> imgPts;
    for (auto& p : calcOriCoordinateSystem[0][0]) {
        imgPts.emplace_back(p.x, p.y);
    }

    Pose pose = telecentricLineCalibrator->estimateTelecentricPose(K_, coff_dis_, m_, width_ / 2, height_ / 2, dx_, dy_, worldPts,
                                                                   imgPts);
    Eigen::Vector3d v_rot = rotMatToVec(pose.R);
    Eigen::Vector3d v_trans = pose.t;
    // 保存世界坐标系
    poseData.allRotVecs.back() = v_rot;
    poseData.allTransVecs.back() = v_trans;
    TelecentricPlatformCalib calibCamera2Plat(K_, coff_dis_, v_rot, v_trans);
    calibCamera2Plat.runDemo();
    // //  2. 输出结构：platform → image → board → corner
    // std::vector<std::vector<std::vector<std::vector<cv::Point2d>>>> allPlatformsBoardsPts;

    // allPlatformsBoardsPts.resize(all_platfromCalibImg_.size());
    // emit text(QString(u8"正在进行所有平台的棋盘格检测..."));
    // // 3. 遍历每个平台
    // for (size_t p = 0; p < all_platfromCalibImg_.size(); ++p) {
    //     emit text(QString(u8"平台 %1：检测棋盘格...").arg(p));

    //     // 取该平台的所有图像
    //     const auto& imgs = all_platfromCalibImg_[p];

    //     // 输出：图像 × 标定板 × 角点
    //     std::vector<std::vector<std::vector<cv::Point2d>>> onePlatformBoards;

    //     libcbDetector->processImagesFromMats(imgs, onePlatformBoards);

    //     allPlatformsBoardsPts[p] = onePlatformBoards;
    //     for (size_t imgIdx = 0; imgIdx < onePlatformBoards.size(); ++imgIdx) {
    //         const auto& boards = onePlatformBoards[imgIdx];

    //         if (boards.empty()) {
    //             emit text(QString(u8"平台 %1 - 图像 %2：未检测到棋盘格").arg(p).arg(imgIdx));
    //             continue;
    //         }

    //         for (size_t b = 0; b < boards.size(); ++b) {
    //             int detectedCorners = boards[b].size();
    //             int expectedCorners = W_ * H_;

    //             if (detectedCorners == expectedCorners) {
    //                 emit text(QString(u8"平台 %1 - 图像 %2 - 棋盘格 %3：角点数 %4 ✔ 符合规格 (%5×%6)")
    //                               .arg(p)
    //                               .arg(imgIdx)
    //                               .arg(b)
    //                               .arg(detectedCorners)
    //                               .arg(W_)
    //                               .arg(H_));
    //             } else {
    //                 emit text(QString(u8"平台 %1 - 图像 %2 - 棋盘格 %3：角点数 %4 ✘ 不符合规格 (%5×%6)")
    //                               .arg(p)
    //                               .arg(imgIdx)
    //                               .arg(b)
    //                               .arg(detectedCorners)
    //                               .arg(W_)
    //                               .arg(H_));
    //             }
    //         }
    //     }
    // }
    // emit text(QString(u8"所有平台棋盘格检测完成！"));
    // TelecentricPlatformCalib calibCamera2Plat(K_, coff_dis_, v_rot, v_trans);
    // calibCamera2Plat.runDemo();
    // for (size_t p = 0; p < allPlatformsBoardsPts.size(); ++p) {
    //     Eigen::Vector3d r, t;
    //     emit text(QString(u8"平台 %1：求解平台姿态...").arg(p));

    //     if (!calibCamera2Plat.estimatePlatformPoseFromBoards(allPlatformsBoardsPts[p], r, t)) {
    //         emit text(QString(u8"平台 %1 姿态求解失败！").arg(p));
    //         continue;
    //     }
    //     emit text(QString(u8"平台 %1 姿态求解成功！").arg(p));
    //     poseData.allRotVecs[p] = r;
    //     poseData.allTransVecs[p] = t;
    // }
    // poseData.save("./data/calibration_config/platform_pose.json");
    // allRotVecs_ = poseData.allRotVecs;
    // allTransVecs_ = poseData.allTransVecs;

    // emit text(QString(u8"所有平台的姿态求解完成！"));
}
std::vector<Eigen::Vector2d> CameraImageProcessor::convertToWorld(const std::vector<Eigen::Vector2d>& pix_pts) {
    if (pix_pts.empty()) return {};

    Eigen::MatrixXd px = vecToMat(pix_pts);

    // pixel → camera (去畸变 + 归一化)
    Eigen::MatrixXd cam_norm = telecentricLineCalibrator->pixelToCameraCoordinates(px, K_, coff_dis_);

    // camera → world（逆平面变换）
    Eigen::MatrixXd world =
        telecentricLineCalibrator->cameraToWorldCoordinates(cam_norm, allRotVecs_.back(), allTransVecs_.back());
    return matToVec(world);
}
