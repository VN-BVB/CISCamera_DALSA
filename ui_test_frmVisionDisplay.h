/********************************************************************************
** Form generated from reading UI file 'test_frmVisionDisplay.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TEST_FRMVISIONDISPLAY_H
#define UI_TEST_FRMVISIONDISPLAY_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_test_FrmVisionDisplay
{
public:

    void setupUi(QWidget *test_FrmVisionDisplay)
    {
        if (test_FrmVisionDisplay->objectName().isEmpty())
            test_FrmVisionDisplay->setObjectName(QString::fromUtf8("test_FrmVisionDisplay"));
        test_FrmVisionDisplay->resize(400, 300);

        retranslateUi(test_FrmVisionDisplay);

        QMetaObject::connectSlotsByName(test_FrmVisionDisplay);
    } // setupUi

    void retranslateUi(QWidget *test_FrmVisionDisplay)
    {
        test_FrmVisionDisplay->setWindowTitle(QCoreApplication::translate("test_FrmVisionDisplay", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class test_FrmVisionDisplay: public Ui_test_FrmVisionDisplay {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TEST_FRMVISIONDISPLAY_H
