#ifndef INTERACTIVE_DISPLAY_MANAGER_H
#define INTERACTIVE_DISPLAY_MANAGER_H

#include <QObject>

const int DisplayViewSceneSize = 2000;

class InteractiveView;
class InteractiveScene;
class InteractiveDisplayManagerPrivate;

class InteractiveDisplayManager : public QObject
{
    Q_OBJECT
public:
    explicit InteractiveDisplayManager(QObject *parent = nullptr);
    ~InteractiveDisplayManager();

protected:
    const QScopedPointer<InteractiveDisplayManagerPrivate> d_ptr;

public:
    // 获取显示视图
    InteractiveView* displayView() const { return m_displayView;}
    // 获取显示场景
    InteractiveScene* displayScene() const;

protected:  // 初始化接口
    // 初始化
    void init();
    // 初始化视图
    void initView();
    // 后续可添加初始化这个小组件的工具栏等内容


private:
    InteractiveView* m_displayView = nullptr;   // 显示视图

};

#endif // INTERACTIVE_DISPLAY_MANAGER_H
