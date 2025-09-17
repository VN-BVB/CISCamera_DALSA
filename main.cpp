#include <QApplication>

#include "src/ui/CISCameraImage.h"
#include "src/ui/imageviewwindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    // CISWidget w;
    // w.show();
    ImageViewWindow w;
    w.show();
    return a.exec();
}
