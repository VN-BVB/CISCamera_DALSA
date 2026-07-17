#include "frm_display.h"

#include <plog/Log.h>

#include <QVBoxLayout>

#include "display_manager.h"
#include "display_scene.h"
#include "display_view.h"

FrmVisionDisplay::FrmVisionDisplay(QWidget* parent) : BaseWidget(parent) { initFrm(); }

FrmVisionDisplay::~FrmVisionDisplay() {}

void FrmVisionDisplay::initFrm() {
    QVBoxLayout* vLayoutDisplay = new QVBoxLayout(this);
    vLayoutDisplay->setObjectName("vLayoutDisplay");
    vLayoutDisplay->setContentsMargins(0, 0, 0, 0);
    vLayoutDisplay->setSpacing(0);

    m_displayMgr = new DisplayManager(this);
    if (m_displayMgr) {
        auto view = m_displayMgr->displayView();
        if (view) {
            view->setParent(this);
            vLayoutDisplay->addWidget(view);
        }
    }
}

DisplayManager* FrmVisionDisplay::getDisplayManager() { return m_displayMgr; }

void FrmVisionDisplay::displayImage(std::shared_ptr<cv::Mat> image, bool autoFit) {
    if (!image || image->empty()) {
        PLOG_WARNING << "显示时传入的图像为空";
        return;
    }

    QImage qimg;
    if (image->type() == CV_8UC1) {
        qimg = QImage(image->data, image->cols, image->rows, static_cast<int>(image->step), QImage::Format_Grayscale8);
    } else {
        cv::Mat img_rgb;
        cv::cvtColor(*image, img_rgb, cv::COLOR_BGR2RGB);
        qimg = QImage(img_rgb.data, img_rgb.cols, img_rgb.rows, static_cast<int>(img_rgb.step), QImage::Format_RGB888);
    }

    if (!m_displayMgr) return;
    DisplayScene* scene = m_displayMgr->displayScene();
    if (scene) {
        scene->whenAddDisplayImage(qimg, QPointF(0, 0));
    }
}

void FrmVisionDisplay::addGraphicComponent(std::shared_ptr<GraphicsItemComponent> component) {
    if (!m_displayMgr) return;
    DisplayScene* scene = m_displayMgr->displayScene();
    if (scene) {
        scene->whenAddGraphicComponent(component);
    }
}

void FrmVisionDisplay::removeGraphicComponent(std::shared_ptr<GraphicsItemComponent> component) {
    if (!m_displayMgr) return;
    DisplayScene* scene = m_displayMgr->displayScene();
    if (scene) {
        scene->whenRemoveGraphicComponent(component);
    }
}

void FrmVisionDisplay::clearAllGraphicComponents() {
    if (!m_displayMgr) return;
    DisplayScene* scene = m_displayMgr->displayScene();
    if (scene) {
        scene->whenClearAllGraphicComponents();
    }
}

void FrmVisionDisplay::clearAllDisplayImages() {
    if (!m_displayMgr) return;
    DisplayScene* scene = m_displayMgr->displayScene();
    if (scene) {
        scene->whenClearAllDisplayImages();
    }
}
