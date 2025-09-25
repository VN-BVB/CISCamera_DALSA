#ifndef INTERACTIVEIMAGEITEM_H
#define INTERACTIVEIMAGEITEM_H

#include <QGraphicsPixmapItem>

class InteractiveImageItem :  public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    InteractiveImageItem();
};

#endif // INTERACTIVEIMAGEITEM_H
