#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <QObject>
#include <QLabel>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

const int DisplayViewSceneSize = 2000;

class DisplayView;
class DisplayScene;
class DisplayManagerPrivate;

class DisplayManager : public QObject
{
    Q_OBJECT
public:
    explicit DisplayManager(QObject *parent = nullptr);
    ~DisplayManager();

protected:
    const QScopedPointer<DisplayManagerPrivate> d_ptr;

public:
    // 获取显示视图
    DisplayView* displayView() const { return m_displayView;}
    // 获取显示场景
    DisplayScene* displayScene() const;

protected:  // 初始化接口
    // 初始化
    void init();
    // 初始化视图
    void initView();
    // 后续可添加初始化这个小组件的工具栏等内容

private slots:
    void onPixelColorReady(const QPoint &pt, int r, int g, int b);

private:
    // 在线程中获取像素颜色
    static void getPixelColor(const QPixmap &pixmap, const QPoint &pt, DisplayManager *manager);

private:
    DisplayView* m_displayView = nullptr;   // 显示视图
    QLabel* lbGrayValue = nullptr;            // 灰度值标签
    QFuture<void> m_pixelColorFuture;           // 异步任务
    QPoint m_lastMousePos;                      // 最后鼠标位置
    bool m_isProcessing = false;                // 是否正在处理

};

#endif // DISPLAY_MANAGER_H
