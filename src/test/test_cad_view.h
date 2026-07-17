#ifndef TEST_CAD_VIEW_H
#define TEST_CAD_VIEW_H

#include <QWidget>

namespace Ui {
class TestCADView;
}

class RGraphicsViewQt;
class RDocument;
class RDocumentInterface;

class TestCADView : public QWidget
{
    Q_OBJECT

public:
    explicit TestCADView(QWidget *parent = nullptr);
    ~TestCADView();

private:
    Ui::TestCADView *ui;

    RGraphicsViewQt* cadView;
};

#endif // TEST_CAD_VIEW_H
