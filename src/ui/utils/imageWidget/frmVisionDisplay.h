#ifndef FRMVISIONDISPLAY_H
#define FRMVISIONDISPLAY_H

#include "baseWidget.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

class InteractiveDisplayManager;

class FrmVisionDisplay : public BaseWidget
{
    Q_OBJECT
public:
    FrmVisionDisplay();
    explicit FrmVisionDisplay(QWidget *parent = nullptr);
    ~FrmVisionDisplay();
    Q_DISABLE_COPY(FrmVisionDisplay)

    InteractiveDisplayManager* getDisplayManager();

public slots:
    void displayImage(const QImage &image, bool autoFit = true);
    void displayImage(const cv::Mat &image, bool autoFit = true);
    void displayImage(std::shared_ptr<cv::Mat> image, bool autoFit = true);


protected:
    void initFrm() override;

private:
    InteractiveDisplayManager* m_displayMgr;
};

#endif // FRMVISIONDISPLAY_H
