/********************************************************************************
** Form generated from reading UI file 'ImageViewWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_IMAGEVIEWWINDOW_H
#define UI_IMAGEVIEWWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>
#include <src/ui/utils/imageWidget/frmVisionDisplay.h>

QT_BEGIN_NAMESPACE

class Ui_ImageViewWindow
{
public:
    QPushButton *pb_open;
    FrmVisionDisplay *gv_image;

    void setupUi(QWidget *ImageViewWindow)
    {
        if (ImageViewWindow->objectName().isEmpty())
            ImageViewWindow->setObjectName(QString::fromUtf8("ImageViewWindow"));
        ImageViewWindow->resize(1500, 1000);
        pb_open = new QPushButton(ImageViewWindow);
        pb_open->setObjectName(QString::fromUtf8("pb_open"));
        pb_open->setGeometry(QRect(210, 890, 1000, 41));
        gv_image = new FrmVisionDisplay(ImageViewWindow);
        gv_image->setObjectName(QString::fromUtf8("gv_image"));
        gv_image->setGeometry(QRect(80, 60, 1300, 800));

        retranslateUi(ImageViewWindow);

        QMetaObject::connectSlotsByName(ImageViewWindow);
    } // setupUi

    void retranslateUi(QWidget *ImageViewWindow)
    {
        ImageViewWindow->setWindowTitle(QCoreApplication::translate("ImageViewWindow", "Form", nullptr));
        pb_open->setText(QCoreApplication::translate("ImageViewWindow", "\346\211\223\345\274\200\345\233\276\345\203\217", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ImageViewWindow: public Ui_ImageViewWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_IMAGEVIEWWINDOW_H
