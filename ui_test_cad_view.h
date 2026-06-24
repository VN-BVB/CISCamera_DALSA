/********************************************************************************
** Form generated from reading UI file 'test_cad_view.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TEST_CAD_VIEW_H
#define UI_TEST_CAD_VIEW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TestCADView
{
public:

    void setupUi(QWidget *TestCADView)
    {
        if (TestCADView->objectName().isEmpty())
            TestCADView->setObjectName(QString::fromUtf8("TestCADView"));
        TestCADView->resize(400, 300);

        retranslateUi(TestCADView);

        QMetaObject::connectSlotsByName(TestCADView);
    } // setupUi

    void retranslateUi(QWidget *TestCADView)
    {
        TestCADView->setWindowTitle(QCoreApplication::translate("TestCADView", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TestCADView: public Ui_TestCADView {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TEST_CAD_VIEW_H
