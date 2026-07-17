#ifndef BASE_WIDGET_H
#define BASE_WIDGET_H

#include <QWidget>

class BaseWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BaseWidget(QWidget *parent = nullptr);
protected:
    virtual void initFrm()=0;
signals:
};

#endif // BASE_WIDGET_H
