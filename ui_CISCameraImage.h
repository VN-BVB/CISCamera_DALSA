/********************************************************************************
** Form generated from reading UI file 'CISCameraImage.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CISCAMERAIMAGE_H
#define UI_CISCAMERAIMAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>
#include <src/ui/imageWidget/openGLImageWidget.h>

QT_BEGIN_NAMESPACE

class Ui_CISWidget
{
public:
    QGridLayout *gridLayout_2;
    QGridLayout *gridLayout;
    QPushButton *btnFreeze;
    QCheckBox *ckbSave;
    QPushButton *btnStart;
    QPushButton *btnContinue;
    QPushButton *btnStop;
    QCheckBox *ckbSplice;
    openGLImageWidget *imgLive;
    openGLImageWidget *imgSplice;

    void setupUi(QWidget *CISWidget)
    {
        if (CISWidget->objectName().isEmpty())
            CISWidget->setObjectName(QString::fromUtf8("CISWidget"));
        CISWidget->resize(800, 600);
        CISWidget->setBaseSize(QSize(0, 0));
        gridLayout_2 = new QGridLayout(CISWidget);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        btnFreeze = new QPushButton(CISWidget);
        btnFreeze->setObjectName(QString::fromUtf8("btnFreeze"));

        gridLayout->addWidget(btnFreeze, 1, 0, 1, 1);

        ckbSave = new QCheckBox(CISWidget);
        ckbSave->setObjectName(QString::fromUtf8("ckbSave"));

        gridLayout->addWidget(ckbSave, 2, 1, 1, 1);

        btnStart = new QPushButton(CISWidget);
        btnStart->setObjectName(QString::fromUtf8("btnStart"));

        gridLayout->addWidget(btnStart, 0, 0, 1, 1);

        btnContinue = new QPushButton(CISWidget);
        btnContinue->setObjectName(QString::fromUtf8("btnContinue"));

        gridLayout->addWidget(btnContinue, 1, 1, 1, 1);

        btnStop = new QPushButton(CISWidget);
        btnStop->setObjectName(QString::fromUtf8("btnStop"));

        gridLayout->addWidget(btnStop, 0, 1, 1, 1);

        ckbSplice = new QCheckBox(CISWidget);
        ckbSplice->setObjectName(QString::fromUtf8("ckbSplice"));

        gridLayout->addWidget(ckbSplice, 2, 0, 1, 1);


        gridLayout_2->addLayout(gridLayout, 0, 1, 1, 1);

        imgLive = new openGLImageWidget(CISWidget);
        imgLive->setObjectName(QString::fromUtf8("imgLive"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
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
        sizePolicy.setHeightForWidth(imgSplice->sizePolicy().hasHeightForWidth());
        imgSplice->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(imgSplice, 1, 0, 1, 2);


        retranslateUi(CISWidget);

        QMetaObject::connectSlotsByName(CISWidget);
    } // setupUi

    void retranslateUi(QWidget *CISWidget)
    {
        CISWidget->setWindowTitle(QCoreApplication::translate("CISWidget", "Widget", nullptr));
        btnFreeze->setText(QCoreApplication::translate("CISWidget", "\346\232\202\345\201\234\351\207\207\351\233\206", nullptr));
        ckbSave->setText(QCoreApplication::translate("CISWidget", "\344\277\235\345\255\230\345\233\276\345\203\217", nullptr));
        btnStart->setText(QCoreApplication::translate("CISWidget", "\345\274\200\345\247\213\351\207\207\351\233\206", nullptr));
        btnContinue->setText(QCoreApplication::translate("CISWidget", "\347\273\247\347\273\255\351\207\207\351\233\206", nullptr));
        btnStop->setText(QCoreApplication::translate("CISWidget", "\347\273\223\346\235\237\351\207\207\351\233\206", nullptr));
        ckbSplice->setText(QCoreApplication::translate("CISWidget", "\346\213\274\346\216\245\345\233\276\345\203\217", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CISWidget: public Ui_CISWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CISCAMERAIMAGE_H
