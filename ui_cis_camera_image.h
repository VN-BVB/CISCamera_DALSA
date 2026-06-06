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
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <src/ui/utils/display/frm_display.h>
#include <src/ui/utils/display/openGLImageWidget.h>
#include "src/rail/rail_widget.h"

QT_BEGIN_NAMESPACE

class Ui_CISWidget
{
public:
    QGridLayout *gridLayout_14;
    QGridLayout *gridLayout_4;
    QTabWidget *tabWidget;
    QWidget *tab;
    QGridLayout *gridLayout_5;
    QGridLayout *gridLayout;
    QPushButton *btnSoftWareTrigger;
    QPushButton *btnStopTrigger;
    QPushButton *btnStop;
    QCheckBox *ckbSplice;
    QPushButton *btnCISConfig;
    QPushButton *btnSave;
    QPushButton *btnStart;
    QGridLayout *gridLayout_8;
    QPushButton *btn_ChessboardDetector;
    QPushButton *btnCameraCalibrate;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_6;
    QLabel *label;
    QLineEdit *speed_lineEdit;
    QLabel *label_5;
    QLineEdit *lead_lineEdit;
    QLabel *label_2;
    QLineEdit *start_lineEdit;
    QLabel *label_3;
    QLineEdit *end_lineEdit;
    QWidget *tab_2;
    QGridLayout *gridLayout_3;
    RailWidget *railWidget;
    QWidget *tab_4;
    QGridLayout *gridLayout_2;
    QHBoxLayout *horizontalLayout;
    FrmVisionDisplay *graphicsView;
    FrmVisionDisplay *graphicsView_3;
    FrmVisionDisplay *graphicsView_4;
    FrmVisionDisplay *graphicsView_2;
    FrmVisionDisplay *graphicsView_6;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_3;
    QVBoxLayout *verticalLayout_2;
    QCheckBox *ckbShowPLlatImg;
    QPushButton *btnReadLocalImg;
    QVBoxLayout *verticalLayout;
    QPushButton *btnClearCPImg;
    QPushButton *btnClearCPDetectResult;
    QGridLayout *gridLayout_9;
    QPushButton *btnSaveAligenmentPlatImg;
    QPushButton *btnCalibratePlat;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_4;
    QComboBox *cbxPlatform;
    FrmVisionDisplay *graphicsView_5;
    QWidget *cisConfigHost;
    QTextEdit *textEdit;
    FrmVisionDisplay *imgSplice;
    openGLImageWidget *imgLive;

    void setupUi(QWidget *CISWidget)
    {
        if (CISWidget->objectName().isEmpty())
            CISWidget->setObjectName(QString::fromUtf8("CISWidget"));
        CISWidget->resize(893, 600);
        CISWidget->setBaseSize(QSize(0, 0));
        gridLayout_14 = new QGridLayout(CISWidget);
        gridLayout_14->setObjectName(QString::fromUtf8("gridLayout_14"));
        gridLayout_4 = new QGridLayout();
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        tabWidget = new QTabWidget(CISWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy);
        tabWidget->setMaximumSize(QSize(16777215, 16777215));
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

        btnStopTrigger = new QPushButton(tab);
        btnStopTrigger->setObjectName(QString::fromUtf8("btnStopTrigger"));
        sizePolicy.setHeightForWidth(btnStopTrigger->sizePolicy().hasHeightForWidth());
        btnStopTrigger->setSizePolicy(sizePolicy);
        btnStopTrigger->setFont(font);

        gridLayout->addWidget(btnStopTrigger, 3, 0, 1, 2);

        btnStop = new QPushButton(tab);
        btnStop->setObjectName(QString::fromUtf8("btnStop"));
        sizePolicy.setHeightForWidth(btnStop->sizePolicy().hasHeightForWidth());
        btnStop->setSizePolicy(sizePolicy);
        btnStop->setFont(font);

        gridLayout->addWidget(btnStop, 0, 1, 1, 1);

        ckbSplice = new QCheckBox(tab);
        ckbSplice->setObjectName(QString::fromUtf8("ckbSplice"));
        sizePolicy.setHeightForWidth(ckbSplice->sizePolicy().hasHeightForWidth());
        ckbSplice->setSizePolicy(sizePolicy);
        ckbSplice->setFont(font);

        gridLayout->addWidget(ckbSplice, 5, 0, 1, 1);

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

        btnStart = new QPushButton(tab);
        btnStart->setObjectName(QString::fromUtf8("btnStart"));
        sizePolicy.setHeightForWidth(btnStart->sizePolicy().hasHeightForWidth());
        btnStart->setSizePolicy(sizePolicy);
        btnStart->setFont(font);

        gridLayout->addWidget(btnStart, 0, 0, 1, 1);


        gridLayout_5->addLayout(gridLayout, 0, 0, 1, 1);

        gridLayout_8 = new QGridLayout();
        gridLayout_8->setObjectName(QString::fromUtf8("gridLayout_8"));
        btn_ChessboardDetector = new QPushButton(tab);
        btn_ChessboardDetector->setObjectName(QString::fromUtf8("btn_ChessboardDetector"));
        QFont font1;
        font1.setPointSize(15);
        btn_ChessboardDetector->setFont(font1);

        gridLayout_8->addWidget(btn_ChessboardDetector, 1, 0, 1, 1);

        btnCameraCalibrate = new QPushButton(tab);
        btnCameraCalibrate->setObjectName(QString::fromUtf8("btnCameraCalibrate"));
        btnCameraCalibrate->setFont(font1);

        gridLayout_8->addWidget(btnCameraCalibrate, 2, 0, 1, 1);

        groupBox = new QGroupBox(tab);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setFont(font1);
        groupBox->setTabletTracking(false);
        groupBox->setLayoutDirection(Qt::LeftToRight);
        gridLayout_6 = new QGridLayout(groupBox);
        gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        QFont font2;
        font2.setPointSize(12);
        label->setFont(font2);

        gridLayout_6->addWidget(label, 0, 0, 1, 1);

        speed_lineEdit = new QLineEdit(groupBox);
        speed_lineEdit->setObjectName(QString::fromUtf8("speed_lineEdit"));
        speed_lineEdit->setFont(font1);

        gridLayout_6->addWidget(speed_lineEdit, 0, 1, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setFont(font2);

        gridLayout_6->addWidget(label_5, 1, 0, 1, 1);

        lead_lineEdit = new QLineEdit(groupBox);
        lead_lineEdit->setObjectName(QString::fromUtf8("lead_lineEdit"));
        lead_lineEdit->setFont(font1);

        gridLayout_6->addWidget(lead_lineEdit, 1, 1, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setFont(font2);

        gridLayout_6->addWidget(label_2, 2, 0, 1, 1);

        start_lineEdit = new QLineEdit(groupBox);
        start_lineEdit->setObjectName(QString::fromUtf8("start_lineEdit"));
        start_lineEdit->setFont(font1);

        gridLayout_6->addWidget(start_lineEdit, 2, 1, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setFont(font2);

        gridLayout_6->addWidget(label_3, 3, 0, 1, 1);

        end_lineEdit = new QLineEdit(groupBox);
        end_lineEdit->setObjectName(QString::fromUtf8("end_lineEdit"));
        end_lineEdit->setFont(font1);

        gridLayout_6->addWidget(end_lineEdit, 3, 1, 1, 1);


        gridLayout_8->addWidget(groupBox, 0, 0, 1, 1);


        gridLayout_5->addLayout(gridLayout_8, 0, 2, 1, 1);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        gridLayout_3 = new QGridLayout(tab_2);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        railWidget = new RailWidget(tab_2);
        railWidget->setObjectName(QString::fromUtf8("railWidget"));

        gridLayout_3->addWidget(railWidget, 0, 0, 1, 1);

        tabWidget->addTab(tab_2, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QString::fromUtf8("tab_4"));
        gridLayout_2 = new QGridLayout(tab_4);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        graphicsView = new FrmVisionDisplay(tab_4);
        graphicsView->setObjectName(QString::fromUtf8("graphicsView"));
        graphicsView->setMaximumSize(QSize(16777215, 1000));

        horizontalLayout->addWidget(graphicsView);

        graphicsView_3 = new FrmVisionDisplay(tab_4);
        graphicsView_3->setObjectName(QString::fromUtf8("graphicsView_3"));
        graphicsView_3->setMaximumSize(QSize(16777215, 1000));

        horizontalLayout->addWidget(graphicsView_3);

        graphicsView_4 = new FrmVisionDisplay(tab_4);
        graphicsView_4->setObjectName(QString::fromUtf8("graphicsView_4"));
        graphicsView_4->setMaximumSize(QSize(16777215, 1000));

        horizontalLayout->addWidget(graphicsView_4);

        graphicsView_2 = new FrmVisionDisplay(tab_4);
        graphicsView_2->setObjectName(QString::fromUtf8("graphicsView_2"));
        graphicsView_2->setMaximumSize(QSize(16777215, 1000));

        horizontalLayout->addWidget(graphicsView_2);

        graphicsView_6 = new FrmVisionDisplay(tab_4);
        graphicsView_6->setObjectName(QString::fromUtf8("graphicsView_6"));

        horizontalLayout->addWidget(graphicsView_6);

        horizontalLayout->setStretch(0, 1);
        horizontalLayout->setStretch(1, 1);
        horizontalLayout->setStretch(2, 1);
        horizontalLayout->setStretch(3, 1);
        horizontalLayout->setStretch(4, 1);

        gridLayout_2->addLayout(horizontalLayout, 0, 0, 1, 2);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        ckbShowPLlatImg = new QCheckBox(tab_4);
        ckbShowPLlatImg->setObjectName(QString::fromUtf8("ckbShowPLlatImg"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(ckbShowPLlatImg->sizePolicy().hasHeightForWidth());
        ckbShowPLlatImg->setSizePolicy(sizePolicy1);
        QFont font3;
        font3.setPointSize(16);
        ckbShowPLlatImg->setFont(font3);

        verticalLayout_2->addWidget(ckbShowPLlatImg);

        btnReadLocalImg = new QPushButton(tab_4);
        btnReadLocalImg->setObjectName(QString::fromUtf8("btnReadLocalImg"));
        btnReadLocalImg->setFont(font3);

        verticalLayout_2->addWidget(btnReadLocalImg);


        horizontalLayout_3->addLayout(verticalLayout_2);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        btnClearCPImg = new QPushButton(tab_4);
        btnClearCPImg->setObjectName(QString::fromUtf8("btnClearCPImg"));
        btnClearCPImg->setFont(font2);

        verticalLayout->addWidget(btnClearCPImg);

        btnClearCPDetectResult = new QPushButton(tab_4);
        btnClearCPDetectResult->setObjectName(QString::fromUtf8("btnClearCPDetectResult"));
        btnClearCPDetectResult->setFont(font2);

        verticalLayout->addWidget(btnClearCPDetectResult);


        horizontalLayout_3->addLayout(verticalLayout);


        verticalLayout_3->addLayout(horizontalLayout_3);

        gridLayout_9 = new QGridLayout();
        gridLayout_9->setObjectName(QString::fromUtf8("gridLayout_9"));
        gridLayout_9->setHorizontalSpacing(0);
        btnSaveAligenmentPlatImg = new QPushButton(tab_4);
        btnSaveAligenmentPlatImg->setObjectName(QString::fromUtf8("btnSaveAligenmentPlatImg"));
        sizePolicy1.setHeightForWidth(btnSaveAligenmentPlatImg->sizePolicy().hasHeightForWidth());
        btnSaveAligenmentPlatImg->setSizePolicy(sizePolicy1);
        btnSaveAligenmentPlatImg->setFont(font3);

        gridLayout_9->addWidget(btnSaveAligenmentPlatImg, 1, 0, 1, 1);

        btnCalibratePlat = new QPushButton(tab_4);
        btnCalibratePlat->setObjectName(QString::fromUtf8("btnCalibratePlat"));
        sizePolicy1.setHeightForWidth(btnCalibratePlat->sizePolicy().hasHeightForWidth());
        btnCalibratePlat->setSizePolicy(sizePolicy1);
        btnCalibratePlat->setFont(font3);

        gridLayout_9->addWidget(btnCalibratePlat, 2, 0, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_4 = new QLabel(tab_4);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setFont(font3);

        horizontalLayout_2->addWidget(label_4);

        cbxPlatform = new QComboBox(tab_4);
        cbxPlatform->addItem(QString());
        cbxPlatform->addItem(QString());
        cbxPlatform->addItem(QString());
        cbxPlatform->addItem(QString());
        cbxPlatform->addItem(QString());
        cbxPlatform->addItem(QString());
        cbxPlatform->addItem(QString());
        cbxPlatform->addItem(QString());
        cbxPlatform->addItem(QString());
        cbxPlatform->setObjectName(QString::fromUtf8("cbxPlatform"));
        cbxPlatform->setFont(font3);

        horizontalLayout_2->addWidget(cbxPlatform);


        gridLayout_9->addLayout(horizontalLayout_2, 0, 0, 1, 1);

        gridLayout_9->setRowStretch(0, 4);

        verticalLayout_3->addLayout(gridLayout_9);


        gridLayout_2->addLayout(verticalLayout_3, 1, 0, 1, 1);

        graphicsView_5 = new FrmVisionDisplay(tab_4);
        graphicsView_5->setObjectName(QString::fromUtf8("graphicsView_5"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(graphicsView_5->sizePolicy().hasHeightForWidth());
        graphicsView_5->setSizePolicy(sizePolicy2);

        gridLayout_2->addWidget(graphicsView_5, 1, 1, 1, 1);

        tabWidget->addTab(tab_4, QString());

        gridLayout_4->addWidget(tabWidget, 0, 2, 1, 1);

        cisConfigHost = new QWidget(CISWidget);
        cisConfigHost->setObjectName(QString::fromUtf8("cisConfigHost"));
        cisConfigHost->setMinimumSize(QSize(100, 0));

        gridLayout_4->addWidget(cisConfigHost, 0, 0, 2, 1);

        textEdit = new QTextEdit(CISWidget);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));
        sizePolicy.setHeightForWidth(textEdit->sizePolicy().hasHeightForWidth());
        textEdit->setSizePolicy(sizePolicy);

        gridLayout_4->addWidget(textEdit, 0, 4, 1, 1);

        imgSplice = new FrmVisionDisplay(CISWidget);
        imgSplice->setObjectName(QString::fromUtf8("imgSplice"));
        sizePolicy.setHeightForWidth(imgSplice->sizePolicy().hasHeightForWidth());
        imgSplice->setSizePolicy(sizePolicy);

        gridLayout_4->addWidget(imgSplice, 1, 1, 1, 4);

        imgLive = new openGLImageWidget(CISWidget);
        imgLive->setObjectName(QString::fromUtf8("imgLive"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(imgLive->sizePolicy().hasHeightForWidth());
        imgLive->setSizePolicy(sizePolicy3);
        imgLive->setMinimumSize(QSize(200, 0));
        imgLive->setMaximumSize(QSize(16777215, 16777215));
        imgLive->setSizeIncrement(QSize(0, 0));
        imgLive->setBaseSize(QSize(0, 0));

        gridLayout_4->addWidget(imgLive, 0, 1, 1, 1);

        gridLayout_4->setRowStretch(0, 1);
        gridLayout_4->setColumnStretch(0, 6);
        gridLayout_4->setColumnStretch(1, 6);
        gridLayout_4->setColumnStretch(2, 6);
        gridLayout_4->setColumnStretch(4, 3);

        gridLayout_14->addLayout(gridLayout_4, 0, 0, 1, 1);


        retranslateUi(CISWidget);

        tabWidget->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(CISWidget);
    } // setupUi

    void retranslateUi(QWidget *CISWidget)
    {
        CISWidget->setWindowTitle(QCoreApplication::translate("CISWidget", "Widget", nullptr));
        btnSoftWareTrigger->setText(QCoreApplication::translate("CISWidget", "\350\275\257\344\273\266\350\247\246\345\217\221", nullptr));
        btnStopTrigger->setText(QCoreApplication::translate("CISWidget", "\345\201\234\346\255\242\350\247\246\345\217\221", nullptr));
        btnStop->setText(QCoreApplication::translate("CISWidget", "\347\273\223\346\235\237\351\207\207\351\233\206", nullptr));
        ckbSplice->setText(QCoreApplication::translate("CISWidget", "\346\213\274\346\216\245\345\233\276\345\203\217", nullptr));
        btnCISConfig->setText(QCoreApplication::translate("CISWidget", "CIS\347\233\270\346\234\272\345\206\205\351\203\250\350\256\276\347\275\256", nullptr));
        btnSave->setText(QCoreApplication::translate("CISWidget", "\344\277\235\345\255\230\345\233\276\345\203\217", nullptr));
        btnStart->setText(QCoreApplication::translate("CISWidget", "\345\274\200\345\247\213\351\207\207\351\233\206", nullptr));
        btn_ChessboardDetector->setText(QCoreApplication::translate("CISWidget", "\346\243\213\347\233\230\346\240\274\350\247\222\347\202\271\346\243\200\346\265\213", nullptr));
        btnCameraCalibrate->setText(QCoreApplication::translate("CISWidget", "\347\233\270\346\234\272\345\206\205\345\217\202\346\240\207\345\256\232", nullptr));
        groupBox->setTitle(QCoreApplication::translate("CISWidget", "\350\275\257\344\273\266\350\247\246\345\217\221\350\275\250\351\201\223\350\256\276\347\275\256", nullptr));
        label->setText(QCoreApplication::translate("CISWidget", "\351\200\237\345\272\246\357\274\232mm/s", nullptr));
        speed_lineEdit->setText(QCoreApplication::translate("CISWidget", "30", nullptr));
        label_5->setText(QCoreApplication::translate("CISWidget", "\351\242\204\350\265\260\357\274\232ms", nullptr));
        lead_lineEdit->setText(QCoreApplication::translate("CISWidget", "1000", nullptr));
        label_2->setText(QCoreApplication::translate("CISWidget", "\350\265\267\347\202\271\357\274\232mm", nullptr));
        start_lineEdit->setText(QCoreApplication::translate("CISWidget", "200", nullptr));
        label_3->setText(QCoreApplication::translate("CISWidget", "\347\273\210\347\202\271\357\274\232mm", nullptr));
        end_lineEdit->setText(QCoreApplication::translate("CISWidget", "500", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("CISWidget", "\347\233\270\346\234\272\346\265\213\350\257\225\345\212\237\350\203\275", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("CISWidget", "\350\275\250\351\201\223\350\277\220\345\212\250\345\212\237\350\203\275", nullptr));
        ckbShowPLlatImg->setText(QCoreApplication::translate("CISWidget", "\346\230\276\347\244\272\345\233\276\345\203\217", nullptr));
        btnReadLocalImg->setText(QCoreApplication::translate("CISWidget", "\350\257\273\345\217\226\346\234\254\345\234\260\345\233\276\345\203\217", nullptr));
        btnClearCPImg->setText(QCoreApplication::translate("CISWidget", "\346\270\205\351\231\244\346\240\207\345\256\232\345\233\276\345\203\217", nullptr));
        btnClearCPDetectResult->setText(QCoreApplication::translate("CISWidget", "\346\270\205\351\231\244\346\243\200\346\265\213\347\273\223\346\236\234", nullptr));
        btnSaveAligenmentPlatImg->setText(QCoreApplication::translate("CISWidget", "\344\277\235\345\255\230\345\257\271\344\275\215\346\240\207\345\256\232\345\233\276\345\203\217", nullptr));
        btnCalibratePlat->setText(QCoreApplication::translate("CISWidget", "\345\257\271\344\275\215\345\271\263\345\217\260\346\240\207\345\256\232", nullptr));
        label_4->setText(QCoreApplication::translate("CISWidget", "\345\257\271\344\275\215\345\271\263\345\217\260\345\272\217\345\217\267", nullptr));
        cbxPlatform->setItemText(0, QCoreApplication::translate("CISWidget", "0", nullptr));
        cbxPlatform->setItemText(1, QCoreApplication::translate("CISWidget", "1", nullptr));
        cbxPlatform->setItemText(2, QCoreApplication::translate("CISWidget", "2", nullptr));
        cbxPlatform->setItemText(3, QCoreApplication::translate("CISWidget", "3", nullptr));
        cbxPlatform->setItemText(4, QCoreApplication::translate("CISWidget", "4", nullptr));
        cbxPlatform->setItemText(5, QCoreApplication::translate("CISWidget", "5", nullptr));
        cbxPlatform->setItemText(6, QCoreApplication::translate("CISWidget", "6", nullptr));
        cbxPlatform->setItemText(7, QCoreApplication::translate("CISWidget", "7", nullptr));
        cbxPlatform->setItemText(8, QCoreApplication::translate("CISWidget", "8", nullptr));

        tabWidget->setTabText(tabWidget->indexOf(tab_4), QCoreApplication::translate("CISWidget", "\347\233\270\346\234\272-\345\257\271\344\275\215\345\271\263\345\217\260\346\240\207\345\256\232", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CISWidget: public Ui_CISWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CIS_CAMERA_IMAGE_H
