#include "cameraImage_processor.h"

#include "src/config/config_manager.h"
#include "src/telecentricLineCalibrator/libcbdetect/lib_cb_detecor.h"
#include "src/telecentricLineCalibrator/telecentric_line_calibrator.h"
#include "src/telecentricLineCalibrator/telecentricplatform_calib.h"
#include "src/utils/image_utils.cpp"

CameraImageProcessor::CameraImageProcessor(QObject* parent) : QObject(parent) {}

CameraImageProcessor::~CameraImageProcessor() {}
void CameraImageProcessor::initCameraCalibrator() {
    lodaCameraCalibrateParams();
    lodaCam2PlatCalibrateParams();
    libcbDetector = std::make_shared<LibCBDetector>();
    telecentricLineCalibrator = std::make_shared<TelecentricLineCalibrator>();
    telecentricPlatCalibrator = std::make_shared<TelecentricPlatformCalib>(K_, coff_dis_, allRotVecs_.back(), allTransVecs_.back());
    connect(this, &CameraImageProcessor::sendSignalToCalibrate, telecentricLineCalibrator.get(),
            &TelecentricLineCalibrator::calibrateCameraFromPointsDemo, Qt::QueuedConnection);
    connect(telecentricLineCalibrator.get(), &TelecentricLineCalibrator::sendSignalSuccessCalib, this,
            &CameraImageProcessor::lodaCameraCalibrateParams, Qt::QueuedConnection);
    connect(telecentricLineCalibrator.get(), &TelecentricLineCalibrator::sendLogMessage, this,
            &CameraImageProcessor::text);  // text 信号已连到 UI 的 whenAppendMessageLog  // text 信号已连到 UI 的 whenAppendMessageLog
    worldPts.reserve(W_ * H_);
    for (int c = 0; c < W_; ++c)
        for (int r = H_ - 1; r >= 0; --r) worldPts.emplace_back(c * spacingMM_, r * spacingMM_);
    // std::cout << "neican" << K_ << std::endl;
    // std::cout << "jibianxishu" << coff_dis_ << std::endl;
}
void CameraImageProcessor::lodaCameraCalibrateParams() {
    v_rot_s.clear();
    v_trans_s.clear();

    // 使用统一配置加载方式，这样可以避免每次初始化这个类都打开配置文件
    auto appConfig = ConfigManager::getInstance().getConfig();
    const auto& calibParam = appConfig.camera_calibration;

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
        // 文件不存在时用默认值，避免崩溃
        std::cout << "nonononoononononononononon" << std::endl;
        allRotVecs_.resize(1, Eigen::Vector3d(0, 0, 0));
        allTransVecs_.resize(1, Eigen::Vector3d(0, 0, 0));
        return;
    }
    allRotVecs_ = cam2PlatParam.allRotVecs;
    allTransVecs_ = cam2PlatParam.allTransVecs;

    // load 存的是世界坐标 → 转为相机坐标
    if (!allRotVecs_.empty() && allTransVecs_.size() > 1 && allTransVecs_.back().norm() > 1e-6) {
        Eigen::Vector3d wR = allRotVecs_.back(), wT = allTransVecs_.back();
        cv::Mat r_wc(3,1,CV_64F), R_wc(3,3,CV_64F), t_wc(3,1,CV_64F);
        for(int j=0;j<3;++j){r_wc.at<double>(j)=wR(j);t_wc.at<double>(j)=wT(j);}
        cv::Rodrigues(r_wc, R_wc);
        for(size_t i=0;i+1<allRotVecs_.size();++i){
            if(allTransVecs_[i].norm()<1e-6) continue;
            cv::Mat r_pw(3,1,CV_64F),R_pw(3,3,CV_64F),t_pw(3,1,CV_64F);
            for(int j=0;j<3;++j){r_pw.at<double>(j)=allRotVecs_[i](j);t_pw.at<double>(j)=allTransVecs_[i](j);}
            cv::Rodrigues(r_pw,R_pw);
            cv::Mat R_pc=R_wc*R_pw, t_pc=R_wc*t_pw+t_wc, r_pc;
            cv::Rodrigues(R_pc,r_pc);
            allRotVecs_[i]=Eigen::Vector3d(r_pc.at<double>(0),r_pc.at<double>(1),r_pc.at<double>(2));
            allTransVecs_[i]=Eigen::Vector3d(t_pc.at<double>(0),t_pc.at<double>(1),t_pc.at<double>(2));
        }
    }
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
    platformGroups_.clear();
    platformGroups_.resize(maxPlatformCount_);

    QDir rootDir(readPlatfromImg_);
    QFileInfoList subDirs = rootDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QFileInfo& info : subDirs) {
        bool ok = false;
        int idx = info.fileName().toInt(&ok);
        if (!ok || idx < 0 || idx >= maxPlatformCount_) continue;

        auto loadDir = [&](const QString& sub) -> std::vector<std::string> {
            std::vector<std::string> paths;
            QDir d(info.absoluteFilePath() + "/" + sub);
            QStringList filters = {"*.bmp", "*.png", "*.jpg", "*.tif", "*.tiff"};
            for (auto& f : d.entryInfoList(filters, QDir::Files)) {
                paths.push_back(f.absoluteFilePath().toStdString());
            }
            return paths;
        };

        platformGroups_[idx].x = loadDir("x");
        platformGroups_[idx].y = loadDir("y");
        platformGroups_[idx].rot = loadDir("rot");

        emit text(QString(u8"平台 %1: x=%2, y=%3, rot=%4")
                      .arg(idx)
                      .arg(platformGroups_[idx].x.size())
                      .arg(platformGroups_[idx].y.size())
                      .arg(platformGroups_[idx].rot.size()));
    }
}
void CameraImageProcessor::whenClearPlatFromFile(const QString& subFolder) {
    emit text(QString(u8"正在清除平台目录 '%1' 下的所有文件...").arg(subFolder));

    // 清空内存中的所有图像
    platformGroups_.clear();
    platformGroups_.resize(maxPlatformCount_);

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
    if (platformGroups_.empty()) {
        loadPlatformCalibImages();
    }

    // ====== 1. 世界坐标系（同原来） ======
    emit text(QString(u8"正在进行初始世界平台的棋盘格检测与外参估计..."));
    PlatformPoseData poseData;
    int N = platformGroups_.size();
    poseData.allRotVecs.assign(N + 1, Eigen::Vector3d(0, 0, 0));
    poseData.allTransVecs.assign(N + 1, Eigen::Vector3d(0, 0, 0));

    std::vector<std::vector<std::vector<cv::Point2d>>> calcOriCoordinateSystem;
    libcbDetector->processImagesInDirectoryFilePath("./data/PaltfromCalibrate/orignCor/img", calcOriCoordinateSystem);
    std::vector<Eigen::Vector2d> imgPts;
    for (auto& p : calcOriCoordinateSystem[0][0]) imgPts.emplace_back(p.x, p.y);

    // libcbdetect 输出是列优先、上→下，生成匹配的 worldPts
    std::vector<Eigen::Vector2d> worldPtsMatch;
    worldPtsMatch.reserve(W_ * H_);
    for (int c = 0; c < W_; ++c)
        for (int r = 0; r < H_; ++r) worldPtsMatch.emplace_back(c * spacingMM_, r * spacingMM_);

    Pose pose = telecentricLineCalibrator->estimateTelecentricPose(K_, coff_dis_, m_, width_ / 2, height_ / 2, dx_, dy_, worldPtsMatch, imgPts);
    Eigen::Vector3d v_rot = rotMatToVec(pose.R);
    Eigen::Vector3d v_trans = pose.t;
    poseData.allRotVecs.back() = v_rot;
    poseData.allTransVecs.back() = v_trans;

    // 立即更新成员变量，让后续 convertToWorld 使用新外参
    allRotVecs_.back() = v_rot;
    allTransVecs_.back() = v_trans;

    // ====== 2. 辅助: 逐张读取→检测→转换→释放 ======
    auto detectAndConvert = [this](const std::string& filePath) -> std::vector<Eigen::Vector2d> {
        // 生成 txt 缓存路径
        std::string txtPath = filePath.substr(0, filePath.find_last_of('.')) + ".txt";

        // 1. 尝试读缓存
        std::vector<Eigen::Vector2d> cachedPts;
        if (readPointsFromTxt(txtPath, cachedPts) && !cachedPts.empty()) {
            if (useCamCoordsForPlat_) {
                Eigen::MatrixXd pxMat = vecToMat(cachedPts);
                Eigen::MatrixXd cam = telecentricLineCalibrator->pixelToCameraCoordinates(pxMat, K_, coff_dis_);
                return matToVec(cam);
            } else {
                return convertToWorld(cachedPts);
            }
        }

        // 2. 读图 + 检测
        cv::Mat img = readLargeBMP(filePath);
        if (img.empty()) {
            img = cv::imread(filePath, cv::IMREAD_UNCHANGED);
        }
        if (img.empty()) return {};

        std::vector<std::vector<std::vector<cv::Point2d>>> boards;
        libcbDetector->processImagesFromMats({img}, boards);

        if (boards.empty() || boards[0].empty()) return {};

        // // ---- 可视化角点序号 ----
        // {
        //     std::string vizPath = filePath.substr(0, filePath.find_last_of('.')) + "_viz.png";
        //     libcbDetector->visualizeCorners(img, boards[0][0], vizPath);
        // }

        // 3. 保存 txt 缓存
        {
            std::ofstream ofs(txtPath);
            if (ofs.is_open()) {
                ofs << std::setprecision(15);
                ofs << "# Index\tX\tY\n";
                for (size_t i = 0; i < boards[0][0].size(); ++i) ofs << i << "\t" << boards[0][0][i].x << "\t" << boards[0][0][i].y << "\n";
            }
        }

        std::vector<Eigen::Vector2d> pix;
        for (auto& pt : boards[0][0]) pix.emplace_back(pt.x, pt.y);
        if (useCamCoordsForPlat_) {
            Eigen::MatrixXd pxMat = vecToMat(pix);
            Eigen::MatrixXd cam = telecentricLineCalibrator->pixelToCameraCoordinates(pxMat, K_, coff_dis_);
            return matToVec(cam);
        } else {
            return convertToWorld(pix);
        }
    };

    // ====== 3. 遍历每个平台 ======
    TelecentricPlatformCalib calibCamera2Plat(K_, coff_dis_, v_rot, v_trans);

    for (size_t p = 0; p < platformGroups_.size(); ++p) {
        auto& g = platformGroups_[p];
        if (g.x.size() < 2 || g.y.size() < 2 || g.rot.size() < 2) {
            emit text(QString(u8"平台 %1 数据不全").arg(p));
            continue;
        }

        emit text(QString(u8"平台 %1：检测 X 组(%2张)...").arg(p).arg(g.x.size()));
        std::vector<std::vector<Eigen::Vector2d>> xws;
        for (auto& img : g.x) {
            auto w = detectAndConvert(img);
            if (!w.empty()) xws.push_back(w);
        }
        emit text(QString(u8"平台 %1：检测 Y 组(%2张)...").arg(p).arg(g.y.size()));
        std::vector<std::vector<Eigen::Vector2d>> yws;
        for (auto& img : g.y) {
            auto w = detectAndConvert(img);
            if (!w.empty()) yws.push_back(w);
        }
        emit text(QString(u8"平台 %1：检测 Rot 组(%2张)...").arg(p).arg(g.rot.size()));
        std::vector<std::vector<Eigen::Vector2d>> rws;
        for (auto& img : g.rot) {
            auto w = detectAndConvert(img);
            if (!w.empty()) rws.push_back(w);
        }

        emit text(QString(u8"平台 %1：求解平台姿态...").arg(p));
        Eigen::Vector3d r, t;
        if (!calibCamera2Plat.estimatePlatformPoseFromBoards(xws, yws, rws, useCamCoordsForPlat_, r, t)) {
            emit text(QString(u8"平台 %1 姿态求解失败！").arg(p));
            continue;
        }
        poseData.allRotVecs[p] = r;
        poseData.allTransVecs[p] = t;
        emit text(QString(u8"平台 %1 姿态求解成功！").arg(p));
    }

    // 相机坐标 → 世界坐标（save/load纯IO，转换放这里）
    {
        Eigen::Vector3d wR = poseData.allRotVecs.back(), wT = poseData.allTransVecs.back();
        cv::Mat r_wc(3,1,CV_64F), R_wc(3,3,CV_64F), t_wc(3,1,CV_64F);
        for(int j=0;j<3;++j){r_wc.at<double>(j)=wR(j);t_wc.at<double>(j)=wT(j);}
        cv::Rodrigues(r_wc,R_wc); cv::Mat R_cw=R_wc.t();
        for(size_t i=0;i+1<poseData.allRotVecs.size();++i){
            if(poseData.allTransVecs[i].norm()<1e-6)continue;
            cv::Mat r_pc(3,1,CV_64F),R_pc(3,3,CV_64F),t_pc(3,1,CV_64F);
            for(int j=0;j<3;++j){r_pc.at<double>(j)=poseData.allRotVecs[i](j);t_pc.at<double>(j)=poseData.allTransVecs[i](j);}
            cv::Rodrigues(r_pc,R_pc);
            cv::Mat R_pw=R_cw*R_pc, t_pw=R_cw*(t_pc-t_wc), r_pw; t_pw.at<double>(2)=0;
            cv::Rodrigues(R_pw,r_pw);
            poseData.allRotVecs[i]=Eigen::Vector3d(r_pw.at<double>(0),r_pw.at<double>(1),r_pw.at<double>(2));
            poseData.allTransVecs[i]=Eigen::Vector3d(t_pw.at<double>(0),t_pw.at<double>(1),t_pw.at<double>(2));
        }
    }
    poseData.save("./data/calibration_config/platform_pose.json", poseData.allRotVecs.back(), poseData.allTransVecs.back());
    allRotVecs_ = poseData.allRotVecs;
    allTransVecs_ = poseData.allTransVecs;

    emit text(QString(u8"所有平台的姿态求解完成！"));
    emit platformCalibDone();
}

std::vector<Eigen::Vector2d> CameraImageProcessor::convertToWorld(const std::vector<Eigen::Vector2d>& pix_pts) {
    if (pix_pts.empty()) return {};

    Eigen::MatrixXd px = vecToMat(pix_pts);

    // pixel → camera (去畸变 + 归一化)
    Eigen::MatrixXd cam_norm = telecentricLineCalibrator->pixelToCameraCoordinates(px, K_, coff_dis_);

    // camera → world（逆平面变换）
    Eigen::MatrixXd world = telecentricLineCalibrator->cameraToWorldCoordinates(cam_norm, allRotVecs_.back(), allTransVecs_.back());
    return matToVec(world);
}

std::vector<Eigen::Vector2d> CameraImageProcessor::convertToPix(const std::vector<Eigen::Vector2d>& world_pts) {
    if (world_pts.empty()) return {};

    // 世界→相机外参 (.back() = orignCor)
    Eigen::Vector3d rvec = allRotVecs_.back();
    Eigen::Vector3d tvec = allTransVecs_.back();
    cv::Mat r_cv(3, 1, CV_64F), R_cv(3, 3, CV_64F);
    for (int i = 0; i < 3; ++i) r_cv.at<double>(i) = rvec(i);
    cv::Rodrigues(r_cv, R_cv);
    Eigen::Matrix3d R_wc;
    cv::cv2eigen(R_cv, R_wc);
    Eigen::Matrix2d R2 = R_wc.block<2, 2>(0, 0);
    Eigen::Vector2d t2 = tvec.head<2>();

    std::vector<Eigen::Vector2d> pix_pts;
    pix_pts.reserve(world_pts.size());

    for (size_t i = 0; i < world_pts.size(); ++i) {
        // ----------- Step 1: 仿射到相机归一化平面 -----------
        Eigen::Vector2d cam_xy = R2 * world_pts[i] + t2;

        // ----------- Step 2: 畸变 -----------
        Eigen::MatrixXd camPt(1, 2);
        camPt.row(0) = cam_xy.transpose();
        Eigen::MatrixXd distortedH = telecentricLineCalibrator->distort(coff_dis_, camPt);

        // ----------- Step 3: 相机内参 -----------
        Eigen::Vector3d uvw = K_ * distortedH.row(0).transpose();

        // ----------- Step 4: 归一化到像素坐标 -----------
        pix_pts.emplace_back(uvw(0) / uvw(2), uvw(1) / uvw(2));
    }
    return pix_pts;
}
