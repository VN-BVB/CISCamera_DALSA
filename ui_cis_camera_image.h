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
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <src/ui/utils/display/frm_display.h>
#include "src/motion/motion_widget.h"

QT_BEGIN_NAMESPACE

class Ui_CISWidget
{
public:
    QGridLayout *gridLayout_14;
    QGridLayout *gridLayout_4;
    QTextEdit *textEdit;
    FrmVisionDisplay *imgSplice;
    QTabWidget *tabWidget;
    QWidget *tab;
    QGridLayout *gridLayout_5;
    QWidget *cameraParamPanel;
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
    QWidget *cameraActionPanel;
    QGridLayout *gridLayout;
    QPushButton *btnSoftWareTrigger;
    QPushButton *btnSave;
    QCheckBox *ckbSplice;
    QPushButton *btnCISConfig;
    QPushButton *btnStopTrigger;
    QPushButton *btnStart;
    QPushButton *btnStop;
    QWidget *tab_2;
    QGridLayout *gridLayout_3;
    QScrollArea *motionScrollArea;
    MotionWidget *motionWidget;
    QWidget *tab_4;
    QGridLayout *gridLayout_2;
    FrmVisionDisplay *graphicsView_5;
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
    QSpacerItem *calibrationControlsSpacer;

    void setupUi(QWidget *CISWidget)
    {
        if (CISWidget->objectName().isEmpty())
            CISWidget->setObjectName(QString::fromUtf8("CISWidget"));
        CISWidget->resize(880, 480);
        CISWidget->setBaseSize(QSize(0, 0));
        QFont font;
        font.setPointSize(10);
        CISWidget->setFont(font);
        gridLayout_14 = new QGridLayout(CISWidget);
        gridLayout_14->setSpacing(6);
        gridLayout_14->setObjectName(QString::fromUtf8("gridLayout_14"));
        gridLayout_14->setContentsMargins(6, 6, 6, 6);
        gridLayout_4 = new QGridLayout();
        gridLayout_4->setSpacing(6);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        gridLayout_4->setContentsMargins(6, 6, 6, 6);
        textEdit = new QTextEdit(CISWidget);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(textEdit->sizePolicy().hasHeightForWidth());
        textEdit->setSizePolicy(sizePolicy);
        textEdit->setMinimumSize(QSize(0, 110));
        textEdit->setMaximumSize(QSize(16777215, 110));

        gridLayout_4->addWidget(textEdit, 1, 0, 1, 2);

        imgSplice = new FrmVisionDisplay(CISWidget);
        imgSplice->setObjectName(QString::fromUtf8("imgSplice"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(imgSplice->sizePolicy().hasHeightForWidth());
        imgSplice->setSizePolicy(sizePolicy1);
        imgSplice->setMinimumSize(QSize(280, 240));

        gridLayout_4->addWidget(imgSplice, 0, 1, 1, 1);

        tabWidget = new QTabWidget(CISWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy2);
        tabWidget->setMinimumSize(QSize(550, 0));
        tabWidget->setMaximumSize(QSize(550, 16777215));
        tabWidget->setMouseTracking(true);
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        tab->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    font-size: 12pt;\n"
"}"));
        gridLayout_5 = new QGridLayout(tab);
        gridLayout_5->setSpacing(4);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        gridLayout_5->setContentsMargins(4, 4, 4, 4);
        cameraParamPanel = new QWidget(tab);
        cameraParamPanel->setObjectName(QString::fromUtf8("cameraParamPanel"));
        cameraParamPanel->setMinimumSize(QSize(238, 0));
        cameraParamPanel->setMaximumSize(QSize(238, 16777215));
        gridLayout_8 = new QGridLayout(cameraParamPanel);
        gridLayout_8->setSpacing(6);
        gridLayout_8->setObjectName(QString::fromUtf8("gridLayout_8"));
        gridLayout_8->setContentsMargins(6, 6, 6, 6);
        btn_ChessboardDetector = new QPushButton(cameraParamPanel);
        btn_ChessboardDetector->setObjectName(QString::fromUtf8("btn_ChessboardDetector"));
        QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(btn_ChessboardDetector->sizePolicy().hasHeightForWidth());
        btn_ChessboardDetector->setSizePolicy(sizePolicy3);
        btn_ChessboardDetector->setMinimumSize(QSize(0, 44));
        btn_ChessboardDetector->setMaximumSize(QSize(16777215, 46));
        QFont font1;
        font1.setPointSize(12);
        btn_ChessboardDetector->setFont(font1);

        gridLayout_8->addWidget(btn_ChessboardDetector, 1, 0, 1, 1);

        btnCameraCalibrate = new QPushButton(cameraParamPanel);
        btnCameraCalibrate->setObjectName(QString::fromUtf8("btnCameraCalibrate"));
        sizePolicy3.setHeightForWidth(btnCameraCalibrate->sizePolicy().hasHeightForWidth());
        btnCameraCalibrate->setSizePolicy(sizePolicy3);
        btnCameraCalibrate->setMinimumSize(QSize(0, 44));
        btnCameraCalibrate->setMaximumSize(QSize(16777215, 46));
        btnCameraCalibrate->setFont(font1);

        gridLayout_8->addWidget(btnCameraCalibrate, 2, 0, 1, 1);

        groupBox = new QGroupBox(cameraParamPanel);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setMinimumSize(QSize(226, 0));
        groupBox->setMaximumSize(QSize(226, 16777215));
        groupBox->setFont(font);
        groupBox->setTabletTracking(false);
        groupBox->setLayoutDirection(Qt::LeftToRight);
        gridLayout_6 = new QGridLayout(groupBox);
        gridLayout_6->setSpacing(6);
        gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
        gridLayout_6->setContentsMargins(6, 6, 6, 6);
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setMinimumSize(QSize(82, 0));
        label->setMaximumSize(QSize(82, 16777215));
        label->setFont(font);
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_6->addWidget(label, 0, 0, 1, 1);

        speed_lineEdit = new QLineEdit(groupBox);
        speed_lineEdit->setObjectName(QString::fromUtf8("speed_lineEdit"));
        QSizePolicy sizePolicy4(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(speed_lineEdit->sizePolicy().hasHeightForWidth());
        speed_lineEdit->setSizePolicy(sizePolicy4);
        speed_lineEdit->setMinimumSize(QSize(108, 0));
        speed_lineEdit->setMaximumSize(QSize(108, 16777215));
        speed_lineEdit->setFont(font);

        gridLayout_6->addWidget(speed_lineEdit, 0, 1, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setMinimumSize(QSize(82, 0));
        label_5->setMaximumSize(QSize(82, 16777215));
        label_5->setFont(font);
        label_5->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_6->addWidget(label_5, 1, 0, 1, 1);

        lead_lineEdit = new QLineEdit(groupBox);
        lead_lineEdit->setObjectName(QString::fromUtf8("lead_lineEdit"));
        sizePolicy4.setHeightForWidth(lead_lineEdit->sizePolicy().hasHeightForWidth());
        lead_lineEdit->setSizePolicy(sizePolicy4);
        lead_lineEdit->setMinimumSize(QSize(108, 0));
        lead_lineEdit->setMaximumSize(QSize(108, 16777215));
        lead_lineEdit->setFont(font);

        gridLayout_6->addWidget(lead_lineEdit, 1, 1, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setMinimumSize(QSize(82, 0));
        label_2->setMaximumSize(QSize(82, 16777215));
        label_2->setFont(font);
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_6->addWidget(label_2, 2, 0, 1, 1);

        start_lineEdit = new QLineEdit(groupBox);
        start_lineEdit->setObjectName(QString::fromUtf8("start_lineEdit"));
        sizePolicy4.setHeightForWidth(start_lineEdit->sizePolicy().hasHeightForWidth());
        start_lineEdit->setSizePolicy(sizePolicy4);
        start_lineEdit->setMinimumSize(QSize(108, 0));
        start_lineEdit->setMaximumSize(QSize(108, 16777215));
        start_lineEdit->setFont(font);

        gridLayout_6->addWidget(start_lineEdit, 2, 1, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setMinimumSize(QSize(82, 0));
        label_3->setMaximumSize(QSize(82, 16777215));
        label_3->setFont(font);
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_6->addWidget(label_3, 3, 0, 1, 1);

        end_lineEdit = new QLineEdit(groupBox);
        end_lineEdit->setObjectName(QString::fromUtf8("end_lineEdit"));
        sizePolicy4.setHeightForWidth(end_lineEdit->sizePolicy().hasHeightForWidth());
        end_lineEdit->setSizePolicy(sizePolicy4);
        end_lineEdit->setMinimumSize(QSize(108, 0));
        end_lineEdit->setMaximumSize(QSize(108, 16777215));
        end_lineEdit->setFont(font);

        gridLayout_6->addWidget(end_lineEdit, 3, 1, 1, 1);

        gridLayout_6->setColumnStretch(1, 1);

        gridLayout_8->addWidget(groupBox, 0, 0, 1, 1);


        gridLayout_5->addWidget(cameraParamPanel, 0, 2, 1, 1);

        cameraActionPanel = new QWidget(tab);
        cameraActionPanel->setObjectName(QString::fromUtf8("cameraActionPanel"));
        cameraActionPanel->setMinimumSize(QSize(250, 0));
        cameraActionPanel->setMaximumSize(QSize(250, 16777215));
        gridLayout = new QGridLayout(cameraActionPanel);
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(6, 6, 6, 6);
        btnSoftWareTrigger = new QPushButton(cameraActionPanel);
        btnSoftWareTrigger->setObjectName(QString::fromUtf8("btnSoftWareTrigger"));
        sizePolicy3.setHeightForWidth(btnSoftWareTrigger->sizePolicy().hasHeightForWidth());
        btnSoftWareTrigger->setSizePolicy(sizePolicy3);
        btnSoftWareTrigger->setMinimumSize(QSize(0, 44));
        btnSoftWareTrigger->setMaximumSize(QSize(16777215, 46));
        btnSoftWareTrigger->setFont(font1);

        gridLayout->addWidget(btnSoftWareTrigger, 4, 0, 1, 2);

        btnSave = new QPushButton(cameraActionPanel);
        btnSave->setObjectName(QString::fromUtf8("btnSave"));
        sizePolicy3.setHeightForWidth(btnSave->sizePolicy().hasHeightForWidth());
        btnSave->setSizePolicy(sizePolicy3);
        btnSave->setMinimumSize(QSize(0, 44));
        btnSave->setMaximumSize(QSize(16777215, 46));
        btnSave->setFont(font1);

        gridLayout->addWidget(btnSave, 6, 1, 1, 1);

        ckbSplice = new QCheckBox(cameraActionPanel);
        ckbSplice->setObjectName(QString::fromUtf8("ckbSplice"));
        sizePolicy3.setHeightForWidth(ckbSplice->sizePolicy().hasHeightForWidth());
        ckbSplice->setSizePolicy(sizePolicy3);
        ckbSplice->setFont(font);

        gridLayout->addWidget(ckbSplice, 6, 0, 1, 1);

        btnCISConfig = new QPushButton(cameraActionPanel);
        btnCISConfig->setObjectName(QString::fromUtf8("btnCISConfig"));
        sizePolicy3.setHeightForWidth(btnCISConfig->sizePolicy().hasHeightForWidth());
        btnCISConfig->setSizePolicy(sizePolicy3);
        btnCISConfig->setMinimumSize(QSize(0, 44));
        btnCISConfig->setMaximumSize(QSize(16777215, 46));
        btnCISConfig->setFont(font1);

        gridLayout->addWidget(btnCISConfig, 1, 0, 1, 2);

        btnStopTrigger = new QPushButton(cameraActionPanel);
        btnStopTrigger->setObjectName(QString::fromUtf8("btnStopTrigger"));
        sizePolicy3.setHeightForWidth(btnStopTrigger->sizePolicy().hasHeightForWidth());
        btnStopTrigger->setSizePolicy(sizePolicy3);
        btnStopTrigger->setMinimumSize(QSize(0, 44));
        btnStopTrigger->setMaximumSize(QSize(16777215, 46));
        btnStopTrigger->setFont(font1);

        gridLayout->addWidget(btnStopTrigger, 5, 0, 1, 2);

        btnStart = new QPushButton(cameraActionPanel);
        btnStart->setObjectName(QString::fromUtf8("btnStart"));
        sizePolicy3.setHeightForWidth(btnStart->sizePolicy().hasHeightForWidth());
        btnStart->setSizePolicy(sizePolicy3);
        btnStart->setMinimumSize(QSize(0, 44));
        btnStart->setMaximumSize(QSize(16777215, 46));
        btnStart->setFont(font1);

        gridLayout->addWidget(btnStart, 2, 0, 1, 1);

        btnStop = new QPushButton(cameraActionPanel);
        btnStop->setObjectName(QString::fromUtf8("btnStop"));
        sizePolicy3.setHeightForWidth(btnStop->sizePolicy().hasHeightForWidth());
        btnStop->setSizePolicy(sizePolicy3);
        btnStop->setMinimumSize(QSize(0, 44));
        btnStop->setMaximumSize(QSize(16777215, 46));
        btnStop->setFont(font1);

        gridLayout->addWidget(btnStop, 2, 1, 1, 1);


        gridLayout_5->addWidget(cameraActionPanel, 0, 0, 1, 1);

        gridLayout_5->setColumnStretch(0, 1);
        gridLayout_5->setColumnStretch(2, 1);
        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        gridLayout_3 = new QGridLayout(tab_2);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        gridLayout_3->setContentsMargins(6, 6, 6, 6);
        motionScrollArea = new QScrollArea(tab_2);
        motionScrollArea->setObjectName(QString::fromUtf8("motionScrollArea"));
        motionScrollArea->setFrameShape(QFrame::NoFrame);
        motionScrollArea->setWidgetResizable(true);
        motionWidget = new MotionWidget();
        motionWidget->setObjectName(QString::fromUtf8("motionWidget"));
        motionWidget->setGeometry(QRect(0, 0, 534, 298));
        motionScrollArea->setWidget(motionWidget);

        gridLayout_3->addWidget(motionScrollArea, 0, 0, 1, 1);

        tabWidget->addTab(tab_2, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QString::fromUtf8("tab_4"));
        gridLayout_2 = new QGridLayout(tab_4);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setContentsMargins(6, 6, 6, 6);
        graphicsView_5 = new FrmVisionDisplay(tab_4);
        graphicsView_5->setObjectName(QString::fromUtf8("graphicsView_5"));
        QSizePolicy sizePolicy5(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(graphicsView_5->sizePolicy().hasHeightForWidth());
        graphicsView_5->setSizePolicy(sizePolicy5);

        gridLayout_2->addWidget(graphicsView_5, 0, 1, 1, 1);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(6, 6, 6, 6);
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(6, 6, 6, 6);
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(6, 6, 6, 6);
        ckbShowPLlatImg = new QCheckBox(tab_4);
        ckbShowPLlatImg->setObjectName(QString::fromUtf8("ckbShowPLlatImg"));
        sizePolicy3.setHeightForWidth(ckbShowPLlatImg->sizePolicy().hasHeightForWidth());
        ckbShowPLlatImg->setSizePolicy(sizePolicy3);
        ckbShowPLlatImg->setFont(font);

        verticalLayout_2->addWidget(ckbShowPLlatImg);

        btnReadLocalImg = new QPushButton(tab_4);
        btnReadLocalImg->setObjectName(QString::fromUtf8("btnReadLocalImg"));
        sizePolicy3.setHeightForWidth(btnReadLocalImg->sizePolicy().hasHeightForWidth());
        btnReadLocalImg->setSizePolicy(sizePolicy3);
        btnReadLocalImg->setFont(font);

        verticalLayout_2->addWidget(btnReadLocalImg);


        horizontalLayout_3->addLayout(verticalLayout_2);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(6, 6, 6, 6);
        btnClearCPImg = new QPushButton(tab_4);
        btnClearCPImg->setObjectName(QString::fromUtf8("btnClearCPImg"));
        sizePolicy3.setHeightForWidth(btnClearCPImg->sizePolicy().hasHeightForWidth());
        btnClearCPImg->setSizePolicy(sizePolicy3);
        btnClearCPImg->setFont(font);

        verticalLayout->addWidget(btnClearCPImg);

        btnClearCPDetectResult = new QPushButton(tab_4);
        btnClearCPDetectResult->setObjectName(QString::fromUtf8("btnClearCPDetectResult"));
        sizePolicy3.setHeightForWidth(btnClearCPDetectResult->sizePolicy().hasHeightForWidth());
        btnClearCPDetectResult->setSizePolicy(sizePolicy3);
        btnClearCPDetectResult->setFont(font);

        verticalLayout->addWidget(btnClearCPDetectResult);


        horizontalLayout_3->addLayout(verticalLayout);


        verticalLayout_3->addLayout(horizontalLayout_3);

        gridLayout_9 = new QGridLayout();
        gridLayout_9->setSpacing(6);
        gridLayout_9->setObjectName(QString::fromUtf8("gridLayout_9"));
        gridLayout_9->setContentsMargins(6, 6, 6, 6);
        btnSaveAligenmentPlatImg = new QPushButton(tab_4);
        btnSaveAligenmentPlatImg->setObjectName(QString::fromUtf8("btnSaveAligenmentPlatImg"));
        sizePolicy3.setHeightForWidth(btnSaveAligenmentPlatImg->sizePolicy().hasHeightForWidth());
        btnSaveAligenmentPlatImg->setSizePolicy(sizePolicy3);
        btnSaveAligenmentPlatImg->setFont(font);

        gridLayout_9->addWidget(btnSaveAligenmentPlatImg, 1, 0, 1, 1);

        btnCalibratePlat = new QPushButton(tab_4);
        btnCalibratePlat->setObjectName(QString::fromUtf8("btnCalibratePlat"));
        sizePolicy3.setHeightForWidth(btnCalibratePlat->sizePolicy().hasHeightForWidth());
        btnCalibratePlat->setSizePolicy(sizePolicy3);
        btnCalibratePlat->setFont(font);

        gridLayout_9->addWidget(btnCalibratePlat, 2, 0, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(6, 6, 6, 6);
        label_4 = new QLabel(tab_4);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setFont(font);

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
        sizePolicy3.setHeightForWidth(cbxPlatform->sizePolicy().hasHeightForWidth());
        cbxPlatform->setSizePolicy(sizePolicy3);
        cbxPlatform->setFont(font);

        horizontalLayout_2->addWidget(cbxPlatform);


        gridLayout_9->addLayout(horizontalLayout_2, 0, 0, 1, 1);


        verticalLayout_3->addLayout(gridLayout_9);

        calibrationControlsSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(calibrationControlsSpacer);

        verticalLayout_3->setStretch(2, 1);

        gridLayout_2->addLayout(verticalLayout_3, 0, 0, 1, 1);

        tabWidget->addTab(tab_4, QString());

        gridLayout_4->addWidget(tabWidget, 0, 0, 1, 1);

        gridLayout_4->setRowStretch(0, 1);
        gridLayout_4->setColumnStretch(1, 1);

        gridLayout_14->addLayout(gridLayout_4, 0, 0, 1, 1);


        retranslateUi(CISWidget);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(CISWidget);
    } // setupUi

    void retranslateUi(QWidget *CISWidget)
    {
        CISWidget->setWindowTitle(QCoreApplication::translate("CISWidget", "Widget", nullptr));
        btn_ChessboardDetector->setText(QCoreApplication::translate("CISWidget", "\346\243\213\347\233\230\346\240\274\350\247\222\347\202\271\346\243\200\346\265\213", nullptr));
        btnCameraCalibrate->setText(QCoreApplication::translate("CISWidget", "\347\233\270\346\234\272\345\206\205\345\217\202\346\240\207\345\256\232", nullptr));
        groupBox->setTitle(QCoreApplication::translate("CISWidget", "\350\275\257\344\273\266\350\247\246\345\217\221\350\275\250\351\201\223\350\256\276\347\275\256", nullptr));
        label->setText(QCoreApplication::translate("CISWidget", "\351\200\237\345\272\246\357\274\232mm/s", nullptr));
        speed_lineEdit->setText(QCoreApplication::translate("CISWidget", "30", nullptr));
        label_5->setText(QCoreApplication::translate("CISWidget", "\351\242\204\350\265\260\357\274\232ms", nullptr));
        lead_lineEdit->setText(QCoreApplication::translate("CISWidget", "0", nullptr));
        label_2->setText(QCoreApplication::translate("CISWidget", "\350\265\267\347\202\271\357\274\232mm", nullptr));
        start_lineEdit->setText(QCoreApplication::translate("CISWidget", "60", nullptr));
        label_3->setText(QCoreApplication::translate("CISWidget", "\347\273\210\347\202\271\357\274\232mm", nullptr));
        end_lineEdit->setText(QCoreApplication::translate("CISWidget", "800", nullptr));
        btnSoftWareTrigger->setText(QCoreApplication::translate("CISWidget", "\345\274\200\345\247\213\350\207\252\345\212\250\346\211\253\346\217\217", nullptr));
        btnSave->setText(QCoreApplication::translate("CISWidget", "\344\277\235\345\255\230\345\233\276\345\203\217", nullptr));
        ckbSplice->setText(QCoreApplication::translate("CISWidget", "\346\213\274\346\216\245\345\233\276\345\203\217", nullptr));
        btnCISConfig->setText(QCoreApplication::translate("CISWidget", "CIS\347\233\270\346\234\272\345\206\205\351\203\250\350\256\276\347\275\256", nullptr));
        btnStopTrigger->setText(QCoreApplication::translate("CISWidget", "\347\273\223\346\235\237\350\207\252\345\212\250\346\211\253\346\217\217", nullptr));
        btnStart->setText(QCoreApplication::translate("CISWidget", "\345\220\257\345\212\250\347\233\270\346\234\272\351\207\207\351\233\206", nullptr));
        btnStop->setText(QCoreApplication::translate("CISWidget", "\347\273\223\346\235\237\347\233\270\346\234\272\351\207\207\351\233\206", nullptr));
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
