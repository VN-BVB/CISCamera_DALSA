#include "cameraImage_processor.h"

CameraImageProcessor::CameraImageProcessor(QObject* parent) : QObject(parent) {}

CameraImageProcessor::~CameraImageProcessor() {}

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

void CameraImageProcessor::processPair(std::shared_ptr<cv::Mat> master, std::shared_ptr<cv::Mat> slave,
                                       bool spliceEnabledFromCaller, bool useColumnCheck) {
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

void CameraImageProcessor::clear() {
    QMutexLocker locker(&mtx_);
    lastMaster_.reset();
    lastSlave_.reset();
    lastResult_.reset();
    emit text(QString(u8"处理缓存已清空"));
}
bool CameraImageProcessor::readPointsFromTxt(const std::string& path, std::vector<Eigen::Vector2d>& pts) {
    std::ifstream fin(path);
    if (!fin.is_open()) {
        std::cerr << "无法打开文件: " << path << std::endl;
        return false;
    }

    std::string line;
    pts.clear();
    // 跳过第一行标题
    std::getline(fin, line);

    double idx, x, y;
    while (fin >> idx >> x >> y) {
        pts.emplace_back(x, y);
    }

    if (pts.empty()) {
        std::cerr << "文件 " << path << " 无有效点。" << std::endl;
        return false;
    }
    std::cout << "读取 " << path << " 成功，共 " << pts.size() << " 个点\n";
    return true;
}
void CameraImageProcessor::whenCameraCalibrate() {
    all_image_points.clear();
    std::string folder = "./data/CISCamera_Image/halcon";

    // 遍历文件夹读取所有txt
    for (auto& entry : std::filesystem::directory_iterator(folder)) {
        if (entry.path().extension() == ".txt") {
            std::vector<Eigen::Vector2d> pts;
            if (readPointsFromTxt(entry.path().string(), pts)) {
                all_image_points.push_back(pts);
            }
        }
    }

    if (all_image_points.empty()) {
        std::cerr << "没有读取到任何标定点文件！" << std::endl;
        return;
    }

    // 构造世界坐标系下圆心点
    std::vector<Eigen::Vector2d> worldPts;
    worldPts.reserve(W * H);
    for (int r = 0; r < H; ++r)
        for (int c = 0; c < W; ++c) worldPts.emplace_back(c * spacingMM, r * spacingMM);

    // 输出结果
    Eigen::Matrix3d K;
    double rmse;
    std::vector<Pose> poses;

    sendSignalToCalibrate(all_image_points, worldPts, width, height, dx, dy, K, rmse, poses);
}
