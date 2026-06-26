/********************************************************************************
** Form generated from reading UI file 'test_multiRoi.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TEST_MULTIROI_H
#define UI_TEST_MULTIROI_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_test_multiRoi
{
public:

    void setupUi(QWidget *test_multiRoi)
    {
        if (test_multiRoi->objectName().isEmpty())
            test_multiRoi->setObjectName(QString::fromUtf8("test_multiRoi"));
        test_multiRoi->resize(400, 300);

        retranslateUi(test_multiRoi);

        QMetaObject::connectSlotsByName(test_multiRoi);
    } // setupUi

    void retranslateUi(QWidget *test_multiRoi)
    {
        test_multiRoi->setWindowTitle(QCoreApplication::translate("test_multiRoi", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class test_multiRoi: public Ui_test_multiRoi {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TEST_MULTIROI_H
