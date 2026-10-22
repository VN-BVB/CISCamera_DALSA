#include <QtTest>
#include <QVBoxLayout>

#include "frm_display.h"
#include "display_manager.h"
#include "display_scene.h"
#include "display_view.h"

class DisplayRegression : public QObject {
    Q_OBJECT
private:
    static std::shared_ptr<cv::Mat> image(int width = 2400, int height = 1200) {
        return std::make_shared<cv::Mat>(height, width, CV_8UC1, cv::Scalar(80));
    }
    static bool isFitted(DisplayView *view) {
        const QRectF imageRect = view->getScene()->itemsBoundingRect();
        const QRectF displayed = view->viewportTransform().mapRect(imageRect);
        const QRectF viewport = view->viewport()->rect();
        const QPointF offset = displayed.center() - viewport.center();
        return viewport.adjusted(-1, -1, 1, 1).contains(displayed) &&
               std::abs(offset.x()) <= 2.0 && std::abs(offset.y()) <= 2.0;
    }
private slots:
    void imageFitsWhenVisible() {
        FrmVisionDisplay display(nullptr);
        display.resize(800, 600);
        display.show();
        QTest::qWait(20);
        display.displayImage(image(), true);
        auto view = display.getDisplayManager()->displayView();
        qInfo() << "image center in viewport" << view->viewportTransform().map(view->getScene()->itemsBoundingRect().center())
                << "viewport" << view->viewport()->rect() << "cursor" << view->viewport()->cursor().shape();
        QVERIFY2(isFitted(view), "Image must be entirely visible and centered on first display");
    }
    void imageLoadedBeforeShowUsesFinalViewport() {
        QWidget window;
        auto layout = new QVBoxLayout(&window);
        auto display = new FrmVisionDisplay(&window);
        layout->addWidget(display);
        display->displayImage(image(), true);
        window.resize(1000, 700);
        window.show();
        QTest::qWait(20);
        QVERIFY2(isFitted(display->getDisplayManager()->displayView()), "Loading in a constructor must fit the final visible viewport");
    }
    void autoFitTracksResize() {
        FrmVisionDisplay display(nullptr);
        display.resize(900, 600);
        display.show();
        QTest::qWait(20);
        display.displayImage(image(), true);
        display.resize(500, 850);
        QTest::qWait(20);
        QVERIFY(isFitted(display.getDisplayManager()->displayView()));
    }
    void autoFitFalsePreservesManualView() {
        FrmVisionDisplay display(nullptr);
        display.resize(800, 600);
        display.show();
        QTest::qWait(20);
        display.displayImage(image(), true);
        auto view = display.getDisplayManager()->displayView();
        view->zoomUp();
        view->translate(QPointF(80, 40));
        const QTransform transform = view->transform();
        const QPointF center = view->mapToScene(view->viewport()->rect().center());
        display.clearAllDisplayImages();
        display.displayImage(image(), false);
        QCOMPARE(view->transform(), transform);
        QCOMPARE(view->mapToScene(view->viewport()->rect().center()), center);
        display.resize(1000, 700);
        QTest::qWait(20);
        QCOMPARE(view->transform(), transform);
    }
    void newFrameCanRestoreFit() {
        FrmVisionDisplay display(nullptr);
        display.resize(800, 600);
        display.show();
        QTest::qWait(20);
        display.displayImage(image(), true);
        auto view = display.getDisplayManager()->displayView();
        view->zoomUp();
        view->translate(QPointF(100, 60));
        display.clearAllDisplayImages();
        display.displayImage(image(), true);
        QVERIFY(isFitted(view));
    }
    void manualZoomSurvivesResizeAndDoubleClickRestoresFit() {
        FrmVisionDisplay display(nullptr);
        display.resize(800, 600);
        display.show();
        QTest::qWait(20);
        display.displayImage(image(), true);
        auto view = display.getDisplayManager()->displayView();
        view->zoomUp();
        const QTransform manualTransform = view->transform();
        display.resize(1000, 700);
        QTest::qWait(20);
        QCOMPARE(view->transform(), manualTransform);
        QTest::mouseDClick(view->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(300, 300));
        QVERIFY(isFitted(view));
    }
    void cursorDoesNotUseHiddenSystemCursor() {
        FrmVisionDisplay display(nullptr);
        auto view = display.getDisplayManager()->displayView();
        QVERIFY2(view->viewport()->cursor().shape() != Qt::BlankCursor,
                 "Use a standard visible cursor rather than a hidden cursor plus a custom overlay");
    }
};

QTEST_MAIN(DisplayRegression)
#include "display_regression.moc"
