#include "interactiveDisplayManager.h"
#include "src/ui/imageWidget/interactiveView.h"
#include "interactiveScene.h"
#include "interactiveImageItem.h"

// Qt
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>

/**************************************************************/
//* [InteractiveDisplayManagerPrivate]
/**************************************************************/
class InteractiveDisplayManagerPrivate
{
    Q_DISABLE_COPY(InteractiveDisplayManagerPrivate)
    Q_DECLARE_PUBLIC(InteractiveDisplayManager)

public:
    InteractiveDisplayManagerPrivate(InteractiveDisplayManager *q):q_ptr(q)
    {

    }
    ~InteractiveDisplayManagerPrivate(){}

public:
    InteractiveDisplayManager              *const q_ptr;


};

/**************************************************************/
//* [InteractiveDisplayManager]
/**************************************************************/
InteractiveDisplayManager::InteractiveDisplayManager(QObject *parent)
    : QObject{parent},
    d_ptr(new InteractiveDisplayManagerPrivate(this))
{
    init();
}

InteractiveDisplayManager::~InteractiveDisplayManager()
{}

void InteractiveDisplayManager::init()
{
    initView();
}

void InteractiveDisplayManager::initView()
{
    m_displayView = new InteractiveView();
    // m_displayView->setSceneRect(-DisplayViewSceneSize/2,-DisplayViewSceneSize/2,DisplayViewSceneSize,DisplayViewSceneSize);
    m_displayView->setSceneRect(0,0,3000,3000);
    m_displayView->setMinZoomCoeff(ViewMinZoomCoeff_Default * 10);
    m_displayView->setMaxZoomCoeff(ViewMaxZoomCoeff_Default * 5);

    auto lyView = new QVBoxLayout(m_displayView);
    lyView->setContentsMargins(0, 0, 0, 0); // 设置无边距
    lyView->setSpacing(0);  // 设置组件间无间隔

    auto topWdg = new QWidget(m_displayView);
    topWdg->setFixedHeight(30);
    auto hLy = new QHBoxLayout(topWdg);
    hLy->setSpacing(10);
    hLy->setContentsMargins(0, 0, 0, 0);
    auto lbGrayValue = new QLabel(m_displayView);// 显示灰度值
    lbGrayValue->setStyleSheet("color:rgb(0,255,0); background-color:rgba(100,100,100,155); font-size: 17px;font-weight: 200px;");
    lbGrayValue->setFixedHeight(30);
    lbGrayValue->setAlignment(Qt::AlignCenter);
    lbGrayValue->setText("");
    hLy->addWidget(lbGrayValue);
    hLy->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    auto downWdg = new QWidget(m_displayView);
    downWdg->setFixedHeight(30);
    hLy = new QHBoxLayout(downWdg);
    hLy->setSpacing(10);
    hLy->setContentsMargins(0, 0, 0, 0);
    auto lbSizeValue = new QLabel(m_displayView);// 显示尺寸大小
    lbSizeValue->setStyleSheet("color:rgb(0,255,0); background-color:rgba(100,100,100,155); font-size: 17px;font-weight: 200px;");
    lbSizeValue->setFixedHeight(30);
    lbSizeValue->setAlignment(Qt::AlignCenter);
    lbSizeValue->setText("");
    hLy->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding,QSizePolicy::Minimum));
    hLy->addWidget(lbSizeValue);

    lyView->addWidget(topWdg);
    lyView->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum,QSizePolicy::Expanding));
    lyView->addWidget(downWdg);

    InteractiveScene* scene = m_displayView->getScene();
    auto imageItem = scene->getDisplayImageItem();
    connect(imageItem, &InteractiveImageItem::sendHoverImagePosition, this, [=](const QPoint &pt)
    {
        int nR, nG, nB;
        imageItem->pixmap().toImage().pixelColor(pt.x(), pt.y()).getRgb(&nR, &nG, &nB);
        QString info = QString("X:%1 Y:%2 | [R:%3 G:%4 B:%4")
                           .arg(QString::number((int)pt.x()), 4, ' ')
                           .arg(QString::number((int)pt.y()),4,' ')
                           .arg(QString::number(nR),3,' ')
                           .arg(QString::number(nG),3,' ')
                           .arg(QString::number(nB),3,' ');
        lbGrayValue->setText(info);
    });
    connect(imageItem, &InteractiveImageItem::sendHoverLeave, this, [=]()
    {
        lbGrayValue->setText("");
    });
    connect(scene, &InteractiveScene::sendUpdateDisplayImage, this, [=](const QImage& img)
    {
        if(!img.isNull())
        {
            QString info = QString("[W:%1 H:%2]")
                               .arg(QString::number(img.width()), 3, ' ')
                               .arg(QString::number(img.height()), 3, ' ');
            lbSizeValue->setText(info);
        }
        else
        {
            lbSizeValue->setText("");
        }
    });
}

InteractiveScene* InteractiveDisplayManager::displayScene() const
{
    if (!m_displayView) return nullptr;
    return m_displayView->getScene();
}























