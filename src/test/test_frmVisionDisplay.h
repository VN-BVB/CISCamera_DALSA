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
    void displayPoints(std::vector<cv::Point2f> points);
    void displayRotateRects(std::vector<cv::RotatedRect>& RotatedRects);
    void displayLines(std::vector<cv::Vec4f> lines);
    void displayBSpline(std::vector<cv::Point2f> controlPoints);
    void onBeginButtonClicked();
    void onDrawLinesBtnClicked();
    void onDrawContoursBtnClicked();
    void onDrawPointsBtnClicked();
    void onDrawBSplineBtnClicked();
    void onDrawRotatedRectBtnClicked();
    void onClearDisplayBtnClicked();
private:
    Ui::test_FrmVisionDisplay *ui;
    FrmVisionDisplay *m_frmDisplay;
    QPushButton* m_btn_begin;
    QPushButton* m_btn_draw_lines;
    QPushButton* m_btn_draw_contours;
    QPushButton* m_btn_draw_points;
    QPushButton* m_btn_draw_bspline;
    QPushButton* m_btn_draw_rotated_rect;
    QPushButton* m_btn_clear_display;
};

#endif // TEST_FRMVISIONDISPLAY_H
