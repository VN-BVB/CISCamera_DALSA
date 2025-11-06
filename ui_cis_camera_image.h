/********************************************************************************
** Form generated from reading UI file 'cis_camera_image.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CIS_CAMERA_IMAGE_H
#define UI_CIS_CAMERA_IMAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>
#include <src/ui/utils/display/openGLImageWidget.h>
#include "src/rail/rail_widget.h"

QT_BEGIN_NAMESPACE

class Ui_CISWidget
{
public:
    QGridLayout *gridLayout_4;
    QGridLayout *gridLayout_2;
    openGLImageWidget *imgLive;
    openGLImageWidget *imgSplice;
    QTabWidget *tabWidget;
    QWidget *tab;
    QHBoxLayout *horizontalLayout;
    QGridLayout *gridLayout;
    QCheckBox *ckbSplice;
    QPushButton *btnSave;
    QPushButton *btnSoftWareTrigger;
    QPushButton *btnStop;
    QPushButton *btnStart;
    QPushButton *btnCISConfig;
    QTextEdit *textEdit;
    QWidget *tab_2;
    QGridLayout *gridLayout_3;
    RailWidget *railWidget;

    void setupUi(QWidget *CISWidget)
    {
        if (CISWidget->objectName().isEmpty())
            CISWidget->setObjectName(QString::fromUtf8("CISWidget"));
        CISWidget->resize(800, 600);
        CISWidget->setBaseSize(QSize(0, 0));
        gridLayout_4 = new QGridLayout(CISWidget);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        imgLive = new openGLImageWidget(CISWidget);
        imgLive->setObjectName(QString::fromUtf8("imgLive"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(imgLive->sizePolicy().hasHeightForWidth());
        imgLive->setSizePolicy(sizePolicy);
        imgLive->setMaximumSize(QSize(16777215, 16777215));
        imgLive->setSizeIncrement(QSize(0, 0));
        imgLive->setBaseSize(QSize(0, 0));

        gridLayout_2->addWidget(imgLive, 0, 0, 1, 1);

        imgSplice = new openGLImageWidget(CISWidget);
        imgSplice->setObjectName(QString::fromUtf8("imgSplice"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(imgSplice->sizePolicy().hasHeightForWidth());
        imgSplice->setSizePolicy(sizePolicy1);

        gridLayout_2->addWidget(imgSplice, 1, 0, 1, 2);

        tabWidget = new QTabWidget(CISWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy2);
        tabWidget->setMouseTracking(true);
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        horizontalLayout = new QHBoxLayout(tab);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        ckbSplice = new QCheckBox(tab);
        ckbSplice->setObjectName(QString::fromUtf8("ckbSplice"));
        sizePolicy2.setHeightForWidth(ckbSplice->sizePolicy().hasHeightForWidth());
        ckbSplice->setSizePolicy(sizePolicy2);
        QFont font;
        font.setPointSize(20);
        ckbSplice->setFont(font);

        gridLayout->addWidget(ckbSplice, 3, 0, 1, 1);

        btnSave = new QPushButton(tab);
        btnSave->setObjectName(QString::fromUtf8("btnSave"));
        sizePolicy2.setHeightForWidth(btnSave->sizePolicy().hasHeightForWidth());
        btnSave->setSizePolicy(sizePolicy2);
        btnSave->setFont(font);

        gridLayout->addWidget(btnSave, 3, 1, 1, 1);

        btnSoftWareTrigger = new QPushButton(tab);
        btnSoftWareTrigger->setObjectName(QString::fromUtf8("btnSoftWareTrigger"));
        sizePolicy2.setHeightForWidth(btnSoftWareTrigger->sizePolicy().hasHeightForWidth());
        btnSoftWareTrigger->setSizePolicy(sizePolicy2);
        btnSoftWareTrigger->setFont(font);

        gridLayout->addWidget(btnSoftWareTrigger, 2, 0, 1, 2);

        btnStop = new QPushButton(tab);
        btnStop->setObjectName(QString::fromUtf8("btnStop"));
        sizePolicy2.setHeightForWidth(btnStop->sizePolicy().hasHeightForWidth());
        btnStop->setSizePolicy(sizePolicy2);
        btnStop->setFont(font);

        gridLayout->addWidget(btnStop, 0, 1, 1, 1);

        btnStart = new QPushButton(tab);
        btnStart->setObjectName(QString::fromUtf8("btnStart"));
        sizePolicy2.setHeightForWidth(btnStart->sizePolicy().hasHeightForWidth());
        btnStart->setSizePolicy(sizePolicy2);
        btnStart->setFont(font);

        gridLayout->addWidget(btnStart, 0, 0, 1, 1);

        btnCISConfig = new QPushButton(tab);
        btnCISConfig->setObjectName(QString::fromUtf8("btnCISConfig"));
        sizePolicy2.setHeightForWidth(btnCISConfig->sizePolicy().hasHeightForWidth());
        btnCISConfig->setSizePolicy(sizePolicy2);
        btnCISConfig->setFont(font);

        gridLayout->addWidget(btnCISConfig, 1, 0, 1, 2);


        horizontalLayout->addLayout(gridLayout);

        textEdit = new QTextEdit(tab);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));
        QSizePolicy sizePolicy3(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(textEdit->sizePolicy().hasHeightForWidth());
        textEdit->setSizePolicy(sizePolicy3);

        horizontalLayout->addWidget(textEdit);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        gridLayout_3 = new QGridLayout(tab_2);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        railWidget = new RailWidget(tab_2);
        railWidget->setObjectName(QString::fromUtf8("railWidget"));

        gridLayout_3->addWidget(railWidget, 0, 0, 1, 1);

        tabWidget->addTab(tab_2, QString());

        gridLayout_2->addWidget(tabWidget, 0, 1, 1, 1);

        gridLayout_2->setColumnStretch(0, 1);

        gridLayout_4->addLayout(gridLayout_2, 0, 0, 1, 1);


        retranslateUi(CISWidget);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(CISWidget);
    } // setupUi

    void retranslateUi(QWidget *CISWidget)
    {
        CISWidget->setWindowTitle(QCoreApplication::translate("CISWidget", "Widget", nullptr));
        ckbSplice->setText(QCoreApplication::translate("CISWidget", "\346\213\274\346\216\245\345\233\276\345\203\217", nullptr));
        btnSave->setText(QCoreApplication::translate("CISWidget", "\344\277\235\345\255\230\345\233\276\345\203\217", nullptr));
        btnSoftWareTrigger->setText(QCoreApplication::translate("CISWidget", "\350\275\257\344\273\266\350\247\246\345\217\221", nullptr));
        btnStop->setText(QCoreApplication::translate("CISWidget", "\347\273\223\346\235\237\351\207\207\351\233\206", nullptr));
        btnStart->setText(QCoreApplication::translate("CISWidget", "\345\274\200\345\247\213\351\207\207\351\233\206", nullptr));
        btnCISConfig->setText(QCoreApplication::translate("CISWidget", "CIS\347\233\270\346\234\272\345\206\205\351\203\250\350\256\276\347\275\256", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("CISWidget", "\347\233\270\346\234\272\346\265\213\350\257\225\345\212\237\350\203\275", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("CISWidget", "\350\275\250\351\201\223\350\277\220\345\212\250\345\212\237\350\203\275", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CISWidget: public Ui_CISWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CIS_CAMERA_IMAGE_H
