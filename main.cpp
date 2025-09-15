#include <QApplication>

#include "src/ui/CISCameraImage.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    CISWidget w;
    w.show();
    return a.exec();
}
