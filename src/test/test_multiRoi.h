#ifndef TEST_MULTIROI_H
#define TEST_MULTIROI_H

#include <QWidget>
#include <QPushButton>
#include "../ui/utils/display/frm_display.h"
#include "../ui/utils/display/display_scene.h"
#include "../ui/utils/display/display_manager.h"

namespace Ui {
class test_multiRoi;
}

class test_multiRoi : public QWidget
{
    Q_OBJECT

public:
    explicit test_multiRoi(QWidget *parent = nullptr);
    ~test_multiRoi();

public slots:
    void displayMultiRoi();
    void onDisplayMultiRoiCliked();

private:
    Ui::test_multiRoi *ui;
    FrmVisionDisplay *m_frmDisplay;
    QPushButton* m_btn_display_multiRoi;
};

#endif // TEST_MULTIROI_H
