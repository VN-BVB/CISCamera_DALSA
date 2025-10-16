#ifndef TEST_FRMVISIONDISPLAY_H
#define TEST_FRMVISIONDISPLAY_H

#include <QWidget>
#include "utils/imageWidget/frmVisionDisplay.h"

namespace Ui {
class test_FrmVisionDisplay;
}

class test_FrmVisionDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit test_FrmVisionDisplay(QWidget *parent = nullptr);
    ~test_FrmVisionDisplay();

public slots:
    void displayImage(const QString &imagePath);

private:
    Ui::test_FrmVisionDisplay *ui;
    FrmVisionDisplay *m_frmDisplay;
};

#endif // TEST_FRMVISIONDISPLAY_H
