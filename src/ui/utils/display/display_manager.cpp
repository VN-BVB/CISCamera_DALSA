#include "display_manager.h"
#include "src/ui/utils/display/display_view.h"
#include "display_scene.h"
#include "display_image_item.h"

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
    // 等待异步任务完成
    if (m_pixelColorFuture.isRunning()) {
        m_pixelColorFuture.waitForFinished();
    }
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
    lbGrayValue = new QLabel(m_displayView);// 显示灰度值
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

    DisplayScene* scene = m_displayView->getScene();
    auto imageItem = scene->getDisplayImageItem();
    // 修改后的鼠标位置处理 - 使用异步线程
    connect(imageItem, &DisplayImageItem::sendHoverImagePosition, this, [=](const QPoint &pt)
            {
                // 如果正在处理上一个请求，跳过新的请求
                if (m_isProcessing) {
                    return;
                }

                // 保存当前鼠标位置
                m_lastMousePos = pt;
                m_isProcessing = true;

                // 在线程中获取像素颜色
                m_pixelColorFuture = QtConcurrent::run([this, imageItem, pt]() {
                    getPixelColor(imageItem->pixmap(), pt, this);
                });
            });
    connect(imageItem, &DisplayImageItem::sendHoverLeave, this, [=]()
            {
                lbGrayValue->setText("");
            });
    connect(scene, &DisplayScene::sendUpdateDisplayImage, this, [=](const QImage& img)
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

// 在线程中获取像素颜色的静态方法
void DisplayManager::getPixelColor(const QPixmap &pixmap, const QPoint &pt, DisplayManager *manager)
{
    if (pixmap.isNull()) {
        return;
    }

    // 将QPixmap转换为QImage并获取像素颜色
    QImage image = pixmap.toImage();
    if (image.isNull() || pt.x() < 0 || pt.y() < 0 || pt.x() >= image.width() || pt.y() >= image.height()) {
        return;
    }

    int nR, nG, nB;
    image.pixelColor(pt.x(), pt.y()).getRgb(&nR, &nG, &nB);

    // 通过信号槽机制将结果传回主线程
    QMetaObject::invokeMethod(manager, "onPixelColorReady", Qt::QueuedConnection,
                              Q_ARG(QPoint, pt), Q_ARG(int, nR), Q_ARG(int, nG), Q_ARG(int, nB));
}

// 在主线程中更新UI的槽函数
void DisplayManager::onPixelColorReady(const QPoint &pt, int r, int g, int b)
{
    // 检查是否还是同一个位置（防止过时的结果）
    if (pt == m_lastMousePos) {
        QString info = QString("X:%1 Y:%2 | [R:%3 G:%4 B:%5]")
        .arg(QString::number((int)pt.x()), 4, ' ')
            .arg(QString::number((int)pt.y()), 4, ' ')
            .arg(QString::number(r), 3, ' ')
            .arg(QString::number(g), 3, ' ')
            .arg(QString::number(b), 3, ' ');
        lbGrayValue->setText(info);
    }

    m_isProcessing = false;
}

DisplayScene* DisplayManager::displayScene() const
{
    if (!m_displayView) return nullptr;
    return m_displayView->getScene();
}























