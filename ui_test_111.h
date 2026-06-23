/********************************************************************************
** Form generated from reading UI file 'test_111.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TEST_111_H
#define UI_TEST_111_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_test_111
{
public:

    void setupUi(QWidget *test_111)
    {
        if (test_111->objectName().isEmpty())
            test_111->setObjectName(QString::fromUtf8("test_111"));
        test_111->resize(400, 300);

        retranslateUi(test_111);

        QMetaObject::connectSlotsByName(test_111);
    } // setupUi

    void retranslateUi(QWidget *test_111)
    {
        test_111->setWindowTitle(QCoreApplication::translate("test_111", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class test_111: public Ui_test_111 {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TEST_111_H
