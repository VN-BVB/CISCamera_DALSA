#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <QObject>
#include <QLabel>

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

private:
    DisplayView* m_displayView = nullptr;   // 显示视图
    QLabel* lbGrayValue = nullptr;            // 鼠标场景坐标标签

};

#endif // DISPLAY_MANAGER_H
