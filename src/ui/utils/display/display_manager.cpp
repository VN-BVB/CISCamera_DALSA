#include "display_manager.h"
#include "src/ui/utils/display/display_view.h"
#include "display_scene.h"

// Qt
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>

/**************************************************************/
//* [InteractiveDisplayManagerPrivate]
/**************************************************************/
class DisplayManagerPrivate
{
    Q_DISABLE_COPY(DisplayManagerPrivate)
    Q_DECLARE_PUBLIC(DisplayManager)

public:
    DisplayManagerPrivate(DisplayManager *q):q_ptr(q)
    {

    }
    ~DisplayManagerPrivate(){}

public:
    DisplayManager              *const q_ptr;


};

/**************************************************************/
//* [InteractiveDisplayManager]
/**************************************************************/
DisplayManager::DisplayManager(QObject *parent)
    : QObject{parent},
    d_ptr(new DisplayManagerPrivate(this))
{
    init();
}

DisplayManager::~DisplayManager()
{
}

void DisplayManager::init()
{
    initView();
}

void DisplayManager::initView()
{
    m_displayView = new DisplayView();
    // m_displayView->setSceneRect(-DisplayViewSceneSize/2,-DisplayViewSceneSize/2,DisplayViewSceneSize,DisplayViewSceneSize);
    // m_displayView->setSceneRect(0,0,3000,3000);
    m_displayView->setMinZoomCoeff(ViewMinZoomCoeff_Default);
    m_displayView->setMaxZoomCoeff(ViewMaxZoomCoeff_Default);

    auto lyView = new QVBoxLayout(m_displayView);
    lyView->setContentsMargins(0, 0, 0, 0); // 设置无边距
    lyView->setSpacing(0);  // 设置组件间无间隔

    auto topWdg = new QWidget(m_displayView);
    topWdg->setFixedHeight(30);
    auto hLy = new QHBoxLayout(topWdg);
    hLy->setSpacing(10);
    hLy->setContentsMargins(0, 0, 0, 0);
    lbGrayValue = new QLabel(m_displayView);// 显示鼠标场景坐标
    lbGrayValue->setStyleSheet("color:rgb(0,255,0); background-color:rgba(100,100,100,155); font-size: 17px;font-weight: 200px;");
    lbGrayValue->setFixedHeight(30);
    lbGrayValue->setAlignment(Qt::AlignCenter);
    lbGrayValue->setText("");
    hLy->addWidget(lbGrayValue);
    hLy->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    lyView->addWidget(topWdg);
    lyView->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum,QSizePolicy::Expanding));

    // 鼠标场景坐标：监听 view 的 mouseMove 信号，不依赖具体 imageItem
    connect(m_displayView, &DisplayView::sendMouseScenePos, this, [=](const QPointF& scenePos)
            {
                QString info = QString("X:%1 Y:%2")
                    .arg(QString::number((int)scenePos.x()), 4, ' ')
                    .arg(QString::number((int)scenePos.y()), 4, ' ');
                lbGrayValue->setText(info);
            });
}

DisplayScene* DisplayManager::displayScene() const
{
    if (!m_displayView) return nullptr;
    return m_displayView->getScene();
}
