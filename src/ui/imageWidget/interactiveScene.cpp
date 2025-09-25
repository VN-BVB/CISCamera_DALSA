#include "interactiveScene.h"
#include "interactiveView.h"
#include "interactiveImageItem.h"

#include <QGraphicsItem>

/*************************/
// [InteracitveScenePrivate]
/*************************/
class InteracitveScenePrivate
{
    Q_DISABLE_COPY(InteracitveScenePrivate) // 禁止拷贝赋值操作
    Q_DECLARE_PUBLIC(InteracitveScene)  // 设置公共访问权限
public:
    InteracitveScenePrivate(InteracitveScene* q):q_ptr(q)
    {
        roiDrawing = false;
    }
    virtual ~InteracitveScenePrivate(){}
public:
    InteracitveScene* const q_str;
    bool roiDrawing;
};

/*************************/
//* [XvDisplayScene]
/*************************/




























