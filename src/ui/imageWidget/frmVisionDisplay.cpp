#include "frmVisionDisplay.h"
#include "interactiveDisplayManager.h"
#include "interactiveView.h"
#include "interactiveScene.h"
#include <QVBoxLayout>

FrmVisionDisplay::FrmVisionDisplay(QWidget *parent):BaseWidget(parent)
{
    initFrm();
}

FrmVisionDisplay::~FrmVisionDisplay()
{}

void FrmVisionDisplay::initFrm()
{
    QVBoxLayout* vLayoutDisplay = new QVBoxLayout(this);
    vLayoutDisplay->setObjectName("vLayoutDisplay");
    vLayoutDisplay->setContentsMargins(0, 0, 0, 0);
    vLayoutDisplay->setSpacing(0);

    // InteractiveDisplayManager* displayMgr = new InteractiveDisplayManager();
    m_displayMgr = new InteractiveDisplayManager();
    if (m_displayMgr)
    {
        auto view = m_displayMgr->displayView();
        if (view)
        {
            view->setParent(this);
            vLayoutDisplay->addWidget(view);
        }
    }
}

void FrmVisionDisplay::displayImage(const QImage &image, bool autoFit)
{
    if (!m_displayMgr) return;
    InteractiveScene* scene = m_displayMgr->displayScene();
    if (scene)
    {
        scene->whenDisplayImage(image, autoFit);
    }
}

// @TODO:在界面中实现这个窗口，然后利用这个窗口显示图像
















