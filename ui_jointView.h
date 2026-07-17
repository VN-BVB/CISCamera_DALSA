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
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <src/ui/utils/imageWidget/frmVisionDisplay.h>

QT_BEGIN_NAMESPACE

class Ui_JointView
{
public:
    FrmVisionDisplay *gv_image;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QPushButton *pb_open;
    QCheckBox *ckb_pixelContoursSquare;
    QCheckBox *ckb_pixelContoursLine;
    QCheckBox *ckb_subpixelContours;
    QCheckBox *ckb_fitlines;
    QCheckBox *ckb_endPoints;

    void setupUi(QWidget *JointView)
    {
        if (JointView->objectName().isEmpty())
            JointView->setObjectName(QString::fromUtf8("JointView"));
        JointView->resize(1512, 851);
        gv_image = new FrmVisionDisplay(JointView);
        gv_image->setObjectName(QString::fromUtf8("gv_image"));
        gv_image->setGeometry(QRect(30, 60, 1231, 721));
        widget = new QWidget(JointView);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(1300, 130, 160, 561));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        pb_open = new QPushButton(widget);
        pb_open->setObjectName(QString::fromUtf8("pb_open"));

        verticalLayout->addWidget(pb_open);

        ckb_pixelContoursSquare = new QCheckBox(widget);
        ckb_pixelContoursSquare->setObjectName(QString::fromUtf8("ckb_pixelContoursSquare"));

        verticalLayout->addWidget(ckb_pixelContoursSquare);

        ckb_pixelContoursLine = new QCheckBox(widget);
        ckb_pixelContoursLine->setObjectName(QString::fromUtf8("ckb_pixelContoursLine"));

        verticalLayout->addWidget(ckb_pixelContoursLine);

        ckb_subpixelContours = new QCheckBox(widget);
        ckb_subpixelContours->setObjectName(QString::fromUtf8("ckb_subpixelContours"));

        verticalLayout->addWidget(ckb_subpixelContours);

        ckb_fitlines = new QCheckBox(widget);
        ckb_fitlines->setObjectName(QString::fromUtf8("ckb_fitlines"));

        verticalLayout->addWidget(ckb_fitlines);

        ckb_endPoints = new QCheckBox(widget);
        ckb_endPoints->setObjectName(QString::fromUtf8("ckb_endPoints"));

        verticalLayout->addWidget(ckb_endPoints);


        retranslateUi(JointView);

        QMetaObject::connectSlotsByName(JointView);
    } // setupUi

    void retranslateUi(QWidget *JointView)
    {
        JointView->setWindowTitle(QCoreApplication::translate("JointView", "Form", nullptr));
        pb_open->setText(QCoreApplication::translate("JointView", "\346\211\223\345\274\200\345\233\276\345\203\217", nullptr));
        ckb_pixelContoursSquare->setText(QCoreApplication::translate("JointView", "\345\203\217\347\264\240\347\272\247\350\275\256\345\273\223\357\274\210\346\226\271\345\235\227\357\274\211", nullptr));
        ckb_pixelContoursLine->setText(QCoreApplication::translate("JointView", "\345\203\217\347\264\240\347\272\247\350\275\256\345\273\223\357\274\210\347\272\277\346\235\241\357\274\211", nullptr));
        ckb_subpixelContours->setText(QCoreApplication::translate("JointView", "\344\272\232\345\203\217\347\264\240\347\272\247\350\275\256\345\273\223", nullptr));
        ckb_fitlines->setText(QCoreApplication::translate("JointView", "\346\213\237\345\220\210\347\272\277\346\235\241", nullptr));
        ckb_endPoints->setText(QCoreApplication::translate("JointView", "\346\213\274\347\274\235\347\253\257\347\202\271", nullptr));
    } // retranslateUi

};

namespace Ui {
    class JointView: public Ui_JointView {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_JOINTVIEW_H
