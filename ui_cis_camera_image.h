/********************************************************************************
** Form generated from reading UI file 'cis_camera_image.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
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
#include <QtWidgets/QWidget>
#include <src/ui/utils/imageWidget/openGLImageWidget.h>
#include "src/rail/rail_widget.h"

QT_BEGIN_NAMESPACE

class Ui_CISWidget
{
public:
    QGridLayout *gridLayout_4;
    QGridLayout *gridLayout_2;
    openGLImageWidget *imgSplice;
    QTabWidget *tabWidget;
    QWidget *tab;
    QHBoxLayout *horizontalLayout;
    QGridLayout *gridLayout;
    QPushButton *btnFreeze;
    QCheckBox *ckbSave;
    QPushButton *btnStop;
    QPushButton *btnSoftWareTrigger;
    QPushButton *btnContinue;
    QPushButton *btnStart;
    QCheckBox *ckbSplice;
    QPushButton *btnCISConfig;
    QWidget *tab_2;
    QGridLayout *gridLayout_3;
    RailWidget *railWidget;
    openGLImageWidget *imgLive;

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
        imgSplice = new openGLImageWidget(CISWidget);
        imgSplice->setObjectName(QString::fromUtf8("imgSplice"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(imgSplice->sizePolicy().hasHeightForWidth());
        imgSplice->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(imgSplice, 1, 0, 1, 2);

        tabWidget = new QTabWidget(CISWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy1);
        tabWidget->setMouseTracking(true);
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        horizontalLayout = new QHBoxLayout(tab);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        btnFreeze = new QPushButton(tab);
        btnFreeze->setObjectName(QString::fromUtf8("btnFreeze"));
        QFont font;
        font.setPointSize(20);
        btnFreeze->setFont(font);

        gridLayout->addWidget(btnFreeze, 1, 0, 1, 1);

        ckbSave = new QCheckBox(tab);
        ckbSave->setObjectName(QString::fromUtf8("ckbSave"));
        ckbSave->setFont(font);

        gridLayout->addWidget(ckbSave, 4, 1, 1, 1);

        btnStop = new QPushButton(tab);
        btnStop->setObjectName(QString::fromUtf8("btnStop"));
        btnStop->setFont(font);

        gridLayout->addWidget(btnStop, 0, 1, 1, 1);

        btnSoftWareTrigger = new QPushButton(tab);
        btnSoftWareTrigger->setObjectName(QString::fromUtf8("btnSoftWareTrigger"));
        QFont font1;
        font1.setPointSize(15);
        btnSoftWareTrigger->setFont(font1);

        gridLayout->addWidget(btnSoftWareTrigger, 3, 0, 1, 2);

        btnContinue = new QPushButton(tab);
        btnContinue->setObjectName(QString::fromUtf8("btnContinue"));
        btnContinue->setFont(font);

        gridLayout->addWidget(btnContinue, 1, 1, 1, 1);

        btnStart = new QPushButton(tab);
        btnStart->setObjectName(QString::fromUtf8("btnStart"));
        btnStart->setFont(font);

        gridLayout->addWidget(btnStart, 0, 0, 1, 1);

        ckbSplice = new QCheckBox(tab);
        ckbSplice->setObjectName(QString::fromUtf8("ckbSplice"));
        ckbSplice->setFont(font);

        gridLayout->addWidget(ckbSplice, 4, 0, 1, 1);

        btnCISConfig = new QPushButton(tab);
        btnCISConfig->setObjectName(QString::fromUtf8("btnCISConfig"));
        btnCISConfig->setFont(font);

        gridLayout->addWidget(btnCISConfig, 2, 0, 1, 2);


        horizontalLayout->addLayout(gridLayout);

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

        imgLive = new openGLImageWidget(CISWidget);
        imgLive->setObjectName(QString::fromUtf8("imgLive"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(imgLive->sizePolicy().hasHeightForWidth());
        imgLive->setSizePolicy(sizePolicy2);
        imgLive->setMaximumSize(QSize(16777215, 16777215));
        imgLive->setSizeIncrement(QSize(0, 0));
        imgLive->setBaseSize(QSize(0, 0));

        gridLayout_2->addWidget(imgLive, 0, 0, 1, 1);

        gridLayout_2->setColumnStretch(0, 1);
        gridLayout_2->setColumnStretch(1, 1);
        gridLayout_2->setColumnMinimumWidth(0, 1);
        gridLayout_2->setColumnMinimumWidth(1, 1);

        gridLayout_4->addLayout(gridLayout_2, 0, 0, 1, 1);


        retranslateUi(CISWidget);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(CISWidget);
    } // setupUi

    void retranslateUi(QWidget *CISWidget)
    {
        CISWidget->setWindowTitle(QCoreApplication::translate("CISWidget", "Widget", nullptr));
        btnFreeze->setText(QCoreApplication::translate("CISWidget", "\346\232\202\345\201\234\351\207\207\351\233\206", nullptr));
        ckbSave->setText(QCoreApplication::translate("CISWidget", "\344\277\235\345\255\230\345\233\276\345\203\217", nullptr));
        btnStop->setText(QCoreApplication::translate("CISWidget", "\347\273\223\346\235\237\351\207\207\351\233\206", nullptr));
        btnSoftWareTrigger->setText(QCoreApplication::translate("CISWidget", "\350\275\257\344\273\266\350\247\246\345\217\221", nullptr));
        btnContinue->setText(QCoreApplication::translate("CISWidget", "\347\273\247\347\273\255\351\207\207\351\233\206", nullptr));
        btnStart->setText(QCoreApplication::translate("CISWidget", "\345\274\200\345\247\213\351\207\207\351\233\206", nullptr));
        ckbSplice->setText(QCoreApplication::translate("CISWidget", "\346\213\274\346\216\245\345\233\276\345\203\217", nullptr));
        btnCISConfig->setText(QCoreApplication::translate("CISWidget", "CIS\347\233\270\346\234\272\345\206\205\351\203\250\350\256\276\347\275\256", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("CISWidget", "Tab 1", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("CISWidget", "Tab 2", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CISWidget: public Ui_CISWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CIS_CAMERA_IMAGE_H
