#ifndef FRM_DISPLAY_H
#define FRM_DISPLAY_H

#include "base_widget.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

class DisplayManager;

class FrmVisionDisplay : public BaseWidget
{
    Q_OBJECT
public:
    FrmVisionDisplay();
    explicit FrmVisionDisplay(QWidget *parent = nullptr);
    ~FrmVisionDisplay();
    Q_DISABLE_COPY(FrmVisionDisplay)

    DisplayManager* getDisplayManager();

public slots:
    void displayImage(std::shared_ptr<cv::Mat> image, bool autoFit = true);


protected:
    void initFrm() override;

private:
    DisplayManager* m_displayMgr;
};

#endif // FRM_DISPLAY_H
