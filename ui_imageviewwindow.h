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
#include <src/ui/imageWidget/interactiveView.h>

QT_BEGIN_NAMESPACE

class Ui_ImageViewWindow
{
public:
    QPushButton *pb_open;
    InteractiveView *gv_image;

    void setupUi(QWidget *ImageViewWindow)
    {
        if (ImageViewWindow->objectName().isEmpty())
            ImageViewWindow->setObjectName(QString::fromUtf8("ImageViewWindow"));
        ImageViewWindow->resize(400, 300);
        pb_open = new QPushButton(ImageViewWindow);
        pb_open->setObjectName(QString::fromUtf8("pb_open"));
        pb_open->setGeometry(QRect(160, 250, 80, 18));
        gv_image = new InteractiveView(ImageViewWindow);
        gv_image->setObjectName(QString::fromUtf8("gv_image"));
        gv_image->setGeometry(QRect(60, 20, 256, 192));

        retranslateUi(ImageViewWindow);

        QMetaObject::connectSlotsByName(ImageViewWindow);
    } // setupUi

    void retranslateUi(QWidget *ImageViewWindow)
    {
        ImageViewWindow->setWindowTitle(QCoreApplication::translate("ImageViewWindow", "Form", nullptr));
        pb_open->setText(QCoreApplication::translate("ImageViewWindow", "PushButton", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ImageViewWindow: public Ui_ImageViewWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_IMAGEVIEWWINDOW_H
