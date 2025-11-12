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
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>
#include <src/ui/utils/display/frm_display.h>
#include <src/ui/utils/display/openGLImageWidget.h>
#include "src/rail/rail_widget.h"

QT_BEGIN_NAMESPACE

class Ui_CISWidget
{
public:
    QGridLayout *gridLayout_4;
    QGridLayout *gridLayout_2;
    QWidget *cisConfigHost;
    FrmVisionDisplay *imgSplice;
    openGLImageWidget *imgLive;
    QTabWidget *tabWidget;
    QWidget *tab;
    QGridLayout *gridLayout_5;
    QGridLayout *gridLayout;
    QPushButton *btnSoftWareTrigger;
    QPushButton *btnStart;
    QCheckBox *ckbSplice;
    QPushButton *btnStop;
    QPushButton *btnCISConfig;
    QPushButton *btnSave;
    QPushButton *btnStopTrigger;
    QTextEdit *textEdit;
    QWidget *tab_2;
    QGridLayout *gridLayout_3;
    RailWidget *railWidget;
    QWidget *tab_3;
    QGridLayout *gridLayout_8;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_7;
    QGridLayout *gridLayout_6;
    QLabel *label;
    QLineEdit *start_lineEdit;
    QLabel *label_2;
    QLineEdit *speed_lineEdit;
    QLabel *label_3;
    QLineEdit *end_lineEdit;
    QPushButton *btn_ChessboardDetector;
    QPushButton *btnCameraCalibrate;

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
        cisConfigHost = new QWidget(CISWidget);
        cisConfigHost->setObjectName(QString::fromUtf8("cisConfigHost"));

        gridLayout_2->addWidget(cisConfigHost, 0, 0, 2, 1);

        imgSplice = new FrmVisionDisplay(CISWidget);
        imgSplice->setObjectName(QString::fromUtf8("imgSplice"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(imgSplice->sizePolicy().hasHeightForWidth());
        imgSplice->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(imgSplice, 1, 1, 1, 2);

        imgLive = new openGLImageWidget(CISWidget);
        imgLive->setObjectName(QString::fromUtf8("imgLive"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(imgLive->sizePolicy().hasHeightForWidth());
        imgLive->setSizePolicy(sizePolicy1);
        imgLive->setMaximumSize(QSize(16777215, 16777215));
        imgLive->setSizeIncrement(QSize(0, 0));
        imgLive->setBaseSize(QSize(0, 0));

        gridLayout_2->addWidget(imgLive, 0, 1, 1, 1);

        tabWidget = new QTabWidget(CISWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        sizePolicy.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy);
        tabWidget->setMouseTracking(true);
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        gridLayout_5 = new QGridLayout(tab);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        btnSoftWareTrigger = new QPushButton(tab);
        btnSoftWareTrigger->setObjectName(QString::fromUtf8("btnSoftWareTrigger"));
        sizePolicy.setHeightForWidth(btnSoftWareTrigger->sizePolicy().hasHeightForWidth());
        btnSoftWareTrigger->setSizePolicy(sizePolicy);
        QFont font;
        font.setPointSize(20);
        btnSoftWareTrigger->setFont(font);

        gridLayout->addWidget(btnSoftWareTrigger, 2, 0, 1, 2);

        btnStart = new QPushButton(tab);
        btnStart->setObjectName(QString::fromUtf8("btnStart"));
        sizePolicy.setHeightForWidth(btnStart->sizePolicy().hasHeightForWidth());
        btnStart->setSizePolicy(sizePolicy);
        btnStart->setFont(font);

        gridLayout->addWidget(btnStart, 0, 0, 1, 1);

        ckbSplice = new QCheckBox(tab);
        ckbSplice->setObjectName(QString::fromUtf8("ckbSplice"));
        sizePolicy.setHeightForWidth(ckbSplice->sizePolicy().hasHeightForWidth());
        ckbSplice->setSizePolicy(sizePolicy);
        ckbSplice->setFont(font);

        gridLayout->addWidget(ckbSplice, 5, 0, 1, 1);

        btnStop = new QPushButton(tab);
        btnStop->setObjectName(QString::fromUtf8("btnStop"));
        sizePolicy.setHeightForWidth(btnStop->sizePolicy().hasHeightForWidth());
        btnStop->setSizePolicy(sizePolicy);
        btnStop->setFont(font);

        gridLayout->addWidget(btnStop, 0, 1, 1, 1);

        btnCISConfig = new QPushButton(tab);
        btnCISConfig->setObjectName(QString::fromUtf8("btnCISConfig"));
        sizePolicy.setHeightForWidth(btnCISConfig->sizePolicy().hasHeightForWidth());
        btnCISConfig->setSizePolicy(sizePolicy);
        btnCISConfig->setFont(font);

        gridLayout->addWidget(btnCISConfig, 1, 0, 1, 2);

        btnSave = new QPushButton(tab);
        btnSave->setObjectName(QString::fromUtf8("btnSave"));
        sizePolicy.setHeightForWidth(btnSave->sizePolicy().hasHeightForWidth());
        btnSave->setSizePolicy(sizePolicy);
        btnSave->setFont(font);

        gridLayout->addWidget(btnSave, 5, 1, 1, 1);

        btnStopTrigger = new QPushButton(tab);
        btnStopTrigger->setObjectName(QString::fromUtf8("btnStopTrigger"));
        sizePolicy.setHeightForWidth(btnStopTrigger->sizePolicy().hasHeightForWidth());
        btnStopTrigger->setSizePolicy(sizePolicy);
        btnStopTrigger->setFont(font);

        gridLayout->addWidget(btnStopTrigger, 3, 0, 1, 2);


        gridLayout_5->addLayout(gridLayout, 0, 0, 1, 1);

        textEdit = new QTextEdit(tab);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));
        QSizePolicy sizePolicy2(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(textEdit->sizePolicy().hasHeightForWidth());
        textEdit->setSizePolicy(sizePolicy2);

        gridLayout_5->addWidget(textEdit, 0, 1, 1, 1);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        gridLayout_3 = new QGridLayout(tab_2);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        railWidget = new RailWidget(tab_2);
        railWidget->setObjectName(QString::fromUtf8("railWidget"));

        gridLayout_3->addWidget(railWidget, 0, 0, 1, 1);

        tabWidget->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QString::fromUtf8("tab_3"));
        gridLayout_8 = new QGridLayout(tab_3);
        gridLayout_8->setObjectName(QString::fromUtf8("gridLayout_8"));
        groupBox = new QGroupBox(tab_3);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        QFont font1;
        font1.setPointSize(15);
        groupBox->setFont(font1);
        groupBox->setTabletTracking(false);
        groupBox->setLayoutDirection(Qt::LeftToRight);
        gridLayout_7 = new QGridLayout(groupBox);
        gridLayout_7->setObjectName(QString::fromUtf8("gridLayout_7"));
        gridLayout_6 = new QGridLayout();
        gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        QFont font2;
        font2.setPointSize(12);
        label->setFont(font2);

        gridLayout_6->addWidget(label, 0, 0, 1, 1);

        start_lineEdit = new QLineEdit(groupBox);
        start_lineEdit->setObjectName(QString::fromUtf8("start_lineEdit"));
        start_lineEdit->setFont(font1);

        gridLayout_6->addWidget(start_lineEdit, 1, 1, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setFont(font2);

        gridLayout_6->addWidget(label_2, 1, 0, 1, 1);

        speed_lineEdit = new QLineEdit(groupBox);
        speed_lineEdit->setObjectName(QString::fromUtf8("speed_lineEdit"));
        speed_lineEdit->setFont(font1);

        gridLayout_6->addWidget(speed_lineEdit, 0, 1, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setFont(font2);

        gridLayout_6->addWidget(label_3, 2, 0, 1, 1);

        end_lineEdit = new QLineEdit(groupBox);
        end_lineEdit->setObjectName(QString::fromUtf8("end_lineEdit"));
        end_lineEdit->setFont(font1);

        gridLayout_6->addWidget(end_lineEdit, 2, 1, 1, 1);

        gridLayout_6->setColumnStretch(0, 10);
        gridLayout_6->setColumnStretch(1, 15);

        gridLayout_7->addLayout(gridLayout_6, 0, 0, 1, 1);


        gridLayout_8->addWidget(groupBox, 0, 0, 1, 1);

        btn_ChessboardDetector = new QPushButton(tab_3);
        btn_ChessboardDetector->setObjectName(QString::fromUtf8("btn_ChessboardDetector"));
        btn_ChessboardDetector->setFont(font1);

        gridLayout_8->addWidget(btn_ChessboardDetector, 1, 0, 1, 1);

        btnCameraCalibrate = new QPushButton(tab_3);
        btnCameraCalibrate->setObjectName(QString::fromUtf8("btnCameraCalibrate"));
        btnCameraCalibrate->setFont(font1);

        gridLayout_8->addWidget(btnCameraCalibrate, 2, 0, 1, 1);

        tabWidget->addTab(tab_3, QString());

        gridLayout_2->addWidget(tabWidget, 0, 2, 1, 1);

        gridLayout_2->setRowStretch(0, 1);
        gridLayout_2->setColumnStretch(0, 5);
        gridLayout_2->setColumnStretch(1, 5);
        gridLayout_2->setColumnStretch(2, 1);
        gridLayout_2->setColumnMinimumWidth(0, 5);
        gridLayout_2->setColumnMinimumWidth(1, 5);
        gridLayout_2->setColumnMinimumWidth(2, 1);

        gridLayout_4->addLayout(gridLayout_2, 0, 1, 1, 1);


        retranslateUi(CISWidget);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(CISWidget);
    } // setupUi

    void retranslateUi(QWidget *CISWidget)
    {
        CISWidget->setWindowTitle(QCoreApplication::translate("CISWidget", "Widget", nullptr));
        btnSoftWareTrigger->setText(QCoreApplication::translate("CISWidget", "\350\275\257\344\273\266\350\247\246\345\217\221", nullptr));
        btnStart->setText(QCoreApplication::translate("CISWidget", "\345\274\200\345\247\213\351\207\207\351\233\206", nullptr));
        ckbSplice->setText(QCoreApplication::translate("CISWidget", "\346\213\274\346\216\245\345\233\276\345\203\217", nullptr));
        btnStop->setText(QCoreApplication::translate("CISWidget", "\347\273\223\346\235\237\351\207\207\351\233\206", nullptr));
        btnCISConfig->setText(QCoreApplication::translate("CISWidget", "CIS\347\233\270\346\234\272\345\206\205\351\203\250\350\256\276\347\275\256", nullptr));
        btnSave->setText(QCoreApplication::translate("CISWidget", "\344\277\235\345\255\230\345\233\276\345\203\217", nullptr));
        btnStopTrigger->setText(QCoreApplication::translate("CISWidget", "\345\201\234\346\255\242\350\247\246\345\217\221", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("CISWidget", "\347\233\270\346\234\272\346\265\213\350\257\225\345\212\237\350\203\275", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("CISWidget", "\350\275\250\351\201\223\350\277\220\345\212\250\345\212\237\350\203\275", nullptr));
        groupBox->setTitle(QCoreApplication::translate("CISWidget", "\350\275\257\344\273\266\350\247\246\345\217\221\350\275\250\351\201\223\350\256\276\347\275\256", nullptr));
        label->setText(QCoreApplication::translate("CISWidget", "\351\200\237\345\272\246\357\274\232mm/s", nullptr));
        start_lineEdit->setText(QCoreApplication::translate("CISWidget", "-80", nullptr));
        label_2->setText(QCoreApplication::translate("CISWidget", "\350\265\267\347\202\271\357\274\232mm", nullptr));
        speed_lineEdit->setText(QCoreApplication::translate("CISWidget", "30", nullptr));
        label_3->setText(QCoreApplication::translate("CISWidget", "\347\273\210\347\202\271\357\274\232mm", nullptr));
        end_lineEdit->setText(QCoreApplication::translate("CISWidget", "260", nullptr));
        btn_ChessboardDetector->setText(QCoreApplication::translate("CISWidget", "\346\243\213\347\233\230\346\240\274\350\247\222\347\202\271\346\243\200\346\265\213", nullptr));
        btnCameraCalibrate->setText(QCoreApplication::translate("CISWidget", "\347\233\270\346\234\272\345\206\205\345\217\202\346\240\207\345\256\232", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QCoreApplication::translate("CISWidget", "\350\256\276\347\275\256", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CISWidget: public Ui_CISWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CIS_CAMERA_IMAGE_H
