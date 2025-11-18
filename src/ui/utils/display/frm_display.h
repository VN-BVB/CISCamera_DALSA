#ifndef FRM_DISPLAY_H
#define FRM_DISPLAY_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

#include "base_widget.h"
#include "graphicItems/graphic_item_Component.h"
#include "graphicItems/graphic_item_composite.h"

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

    // 图形图元操作接口
    void addGraphicComponent(std::shared_ptr<GraphicsItemComponent> component);
    void removeGraphicComponent(std::shared_ptr<GraphicsItemComponent> component);
    void clearAllGraphicComponents();


protected:
    void initFrm() override;

private:
    DisplayManager* m_displayMgr;
};

#endif // FRM_DISPLAY_H
