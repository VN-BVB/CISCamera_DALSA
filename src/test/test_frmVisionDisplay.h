#ifndef TEST_FRMVISIONDISPLAY_H
#define TEST_FRMVISIONDISPLAY_H

#include <QWidget>
#include <QPushButton>
#include "../ui/utils/display/frm_display.h"
#include "../ui/utils/display/display_scene.h"
#include "../ui/utils/display/display_manager.h"
#include "test_edge_assembly.h"

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
    void displayContours(std::vector<std::vector<cv::Point2f>> contours);
    void onBeginButtonClicked();

private:
    Ui::test_FrmVisionDisplay *ui;
    FrmVisionDisplay *m_frmDisplay;
    QPushButton* m_btn_begin;
};

#endif // TEST_FRMVISIONDISPLAY_H
