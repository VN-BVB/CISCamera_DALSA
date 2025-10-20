/********************************************************************************
** Form generated from reading UI file 'jointView.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_JOINTVIEW_H
#define UI_JOINTVIEW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>
#include <src/ui/utils/imageWidget/frmVisionDisplay.h>

QT_BEGIN_NAMESPACE

class Ui_JointView
{
public:
    QPushButton *pb_open;
    FrmVisionDisplay *gv_image;

    void setupUi(QWidget *JointView)
    {
        if (JointView->objectName().isEmpty())
            JointView->setObjectName(QString::fromUtf8("JointView"));
        JointView->resize(1500, 1000);
        pb_open = new QPushButton(JointView);
        pb_open->setObjectName(QString::fromUtf8("pb_open"));
        pb_open->setGeometry(QRect(210, 890, 1000, 41));
        gv_image = new FrmVisionDisplay(JointView);
        gv_image->setObjectName(QString::fromUtf8("gv_image"));
        gv_image->setGeometry(QRect(80, 60, 1300, 800));

        retranslateUi(JointView);

        QMetaObject::connectSlotsByName(JointView);
    } // setupUi

    void retranslateUi(QWidget *JointView)
    {
        JointView->setWindowTitle(QCoreApplication::translate("JointView", "Form", nullptr));
        pb_open->setText(QCoreApplication::translate("JointView", "\346\211\223\345\274\200\345\233\276\345\203\217", nullptr));
    } // retranslateUi

};

namespace Ui {
    class JointView: public Ui_JointView {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_JOINTVIEW_H
