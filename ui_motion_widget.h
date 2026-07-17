/********************************************************************************
** Form generated from reading UI file 'motion_widget.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MOTION_WIDGET_H
#define UI_MOTION_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MotionWidget
{
public:
    QVBoxLayout *verticalLayout_6;
    QWidget *widget_7;
    QHBoxLayout *horizontalLayout_8;
    QWidget *widget_rail;
    QVBoxLayout *verticalLayout_4;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QWidget *widget;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QLineEdit *rail_edit_targetPos;
    QWidget *widget_2;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_2;
    QLineEdit *rail_edit_targetVel;
    QWidget *widget_3;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_3;
    QLineEdit *rail_edit_curPos;
    QWidget *widget_4;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_4;
    QLineEdit *rail_edit_curVel;
    QGroupBox *groupbox;
    QGridLayout *gridLayout;
    QPushButton *rail_btn_connect;
    QPushButton *rail_btn_disconnect;
    QPushButton *rail_btn_absLocate;
    QPushButton *rail_btn_stop;
    QWidget *widget_platform;
    QVBoxLayout *verticalLayout_5;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_3;
    QTableWidget *table_axispos;
    QWidget *widget_6;
    QHBoxLayout *horizontalLayout_7;
    QPushButton *plt_btn_singlepltmover;
    QComboBox *combo_singlepltchoose;
    QWidget *widget_5;
    QHBoxLayout *horizontalLayout_6;
    QPushButton *plt_btn_singleaxismover;
    QComboBox *combo_singleaxispltchoose;
    QComboBox *combo_singleaxischoose;
    QLineEdit *plt_edit_singleaxismoverpos;
    QGroupBox *groupBox_4;
    QGridLayout *gridLayout_2;
    QPushButton *plt_btn_resetAll;
    QPushButton *plt_btn_enableAll;
    QPushButton *plt_btn_homeAll;
    QPushButton *plt_btn_disenableAll;
    QPushButton *plt_btn_locateAll;
    QPushButton *plt_btn_stopAll;
    QWidget *widget_8;
    QGridLayout *gridLayout_3;
    QFrame *card_rail;
    QVBoxLayout *verticalLayout_7;
    QWidget *widget_11;
    QHBoxLayout *horizontalLayout;
    QLabel *label_5;
    QLabel *rail_label_status;
    QWidget *widget_12;
    QHBoxLayout *horizontalLayout_11;
    QLabel *label_11;
    QLabel *rail_label_state;
    QFrame *card_plt0;
    QVBoxLayout *verticalLayout_8;
    QWidget *widget_13;
    QHBoxLayout *horizontalLayout_9;
    QLabel *label_13;
    QLabel *plt0_label_Xstatus;
    QLabel *plt0_label_Ystatus;
    QLabel *plt0_label_Zstatus;
    QWidget *widget_16;
    QHBoxLayout *horizontalLayout_15;
    QLabel *label_19;
    QLabel *plt0_label_state;
    QFrame *card_plt1;
    QVBoxLayout *verticalLayout_9;
    QWidget *widget_17;
    QHBoxLayout *horizontalLayout_10;
    QLabel *label_21;
    QLabel *plt1_label_Xstatus;
    QLabel *plt1_label_Ystatus;
    QLabel *plt1_label_Zstatus;
    QWidget *widget_20;
    QHBoxLayout *horizontalLayout_19;
    QLabel *label_27;
    QLabel *plt1_label_state;
    QFrame *card_plt2;
    QVBoxLayout *verticalLayout_10;
    QWidget *widget_21;
    QHBoxLayout *horizontalLayout_12;
    QLabel *label_29;
    QLabel *plt2_label_Xstatus;
    QLabel *plt2_label_Ystatus;
    QLabel *plt2_label_Zstatus;
    QWidget *widget_24;
    QHBoxLayout *horizontalLayout_23;
    QLabel *label_35;
    QLabel *plt2_label_state;
    QFrame *card_plt3;
    QVBoxLayout *verticalLayout_11;
    QWidget *widget_25;
    QHBoxLayout *horizontalLayout_13;
    QLabel *label_37;
    QLabel *plt3_label_Xstatus;
    QLabel *plt3_label_Ystatus;
    QLabel *plt3_label_Zstatus;
    QWidget *widget_28;
    QHBoxLayout *horizontalLayout_27;
    QLabel *label_43;
    QLabel *plt3_label_state;
    QFrame *card_plt4;
    QVBoxLayout *verticalLayout_12;
    QWidget *widget_29;
    QHBoxLayout *horizontalLayout_14;
    QLabel *label_45;
    QLabel *plt4_label_Xstatus;
    QLabel *plt4_label_Ystatus;
    QLabel *plt4_label_Zstatus;
    QWidget *widget_32;
    QHBoxLayout *horizontalLayout_31;
    QLabel *label_51;
    QLabel *plt4_label_state;
    QFrame *card_plt5;
    QVBoxLayout *verticalLayout_15;
    QWidget *widget_41;
    QHBoxLayout *horizontalLayout_16;
    QLabel *label_69;
    QLabel *plt5_label_Xstatus;
    QLabel *plt5_label_Ystatus;
    QLabel *plt5_label_Zstatus;
    QWidget *widget_44;
    QHBoxLayout *horizontalLayout_43;
    QLabel *label_75;
    QLabel *plt5_label_state;
    QFrame *card_plt6;
    QVBoxLayout *verticalLayout_16;
    QWidget *widget_45;
    QHBoxLayout *horizontalLayout_17;
    QLabel *label_77;
    QLabel *plt6_label_Xstatus;
    QLabel *plt6_label_Ystatus;
    QLabel *plt6_label_Zstatus;
    QWidget *widget_48;
    QHBoxLayout *horizontalLayout_47;
    QLabel *label_83;
    QLabel *plt6_label_state;

    void setupUi(QWidget *MotionWidget)
    {
        if (MotionWidget->objectName().isEmpty())
            MotionWidget->setObjectName(QString::fromUtf8("MotionWidget"));
        MotionWidget->resize(1107, 870);
        QFont font;
        font.setPointSize(12);
        MotionWidget->setFont(font);
        verticalLayout_6 = new QVBoxLayout(MotionWidget);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        widget_7 = new QWidget(MotionWidget);
        widget_7->setObjectName(QString::fromUtf8("widget_7"));
        horizontalLayout_8 = new QHBoxLayout(widget_7);
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        widget_rail = new QWidget(widget_7);
        widget_rail->setObjectName(QString::fromUtf8("widget_rail"));
        verticalLayout_4 = new QVBoxLayout(widget_rail);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        groupBox = new QGroupBox(widget_rail);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy);
        groupBox->setMinimumSize(QSize(0, 150));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        widget = new QWidget(groupBox);
        widget->setObjectName(QString::fromUtf8("widget"));
        horizontalLayout_2 = new QHBoxLayout(widget);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label = new QLabel(widget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_2->addWidget(label);

        rail_edit_targetPos = new QLineEdit(widget);
        rail_edit_targetPos->setObjectName(QString::fromUtf8("rail_edit_targetPos"));

        horizontalLayout_2->addWidget(rail_edit_targetPos);


        verticalLayout_2->addWidget(widget);

        widget_2 = new QWidget(groupBox);
        widget_2->setObjectName(QString::fromUtf8("widget_2"));
        horizontalLayout_3 = new QHBoxLayout(widget_2);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_2 = new QLabel(widget_2);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_3->addWidget(label_2);

        rail_edit_targetVel = new QLineEdit(widget_2);
        rail_edit_targetVel->setObjectName(QString::fromUtf8("rail_edit_targetVel"));

        horizontalLayout_3->addWidget(rail_edit_targetVel);


        verticalLayout_2->addWidget(widget_2);

        widget_3 = new QWidget(groupBox);
        widget_3->setObjectName(QString::fromUtf8("widget_3"));
        horizontalLayout_4 = new QHBoxLayout(widget_3);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_3 = new QLabel(widget_3);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_4->addWidget(label_3);

        rail_edit_curPos = new QLineEdit(widget_3);
        rail_edit_curPos->setObjectName(QString::fromUtf8("rail_edit_curPos"));

        horizontalLayout_4->addWidget(rail_edit_curPos);


        verticalLayout_2->addWidget(widget_3);

        widget_4 = new QWidget(groupBox);
        widget_4->setObjectName(QString::fromUtf8("widget_4"));
        horizontalLayout_5 = new QHBoxLayout(widget_4);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_4 = new QLabel(widget_4);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_5->addWidget(label_4);

        rail_edit_curVel = new QLineEdit(widget_4);
        rail_edit_curVel->setObjectName(QString::fromUtf8("rail_edit_curVel"));

        horizontalLayout_5->addWidget(rail_edit_curVel);


        verticalLayout_2->addWidget(widget_4);


        verticalLayout_4->addWidget(groupBox);

        groupbox = new QGroupBox(widget_rail);
        groupbox->setObjectName(QString::fromUtf8("groupbox"));
        gridLayout = new QGridLayout(groupbox);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        rail_btn_connect = new QPushButton(groupbox);
        rail_btn_connect->setObjectName(QString::fromUtf8("rail_btn_connect"));

        gridLayout->addWidget(rail_btn_connect, 0, 0, 1, 1);

        rail_btn_disconnect = new QPushButton(groupbox);
        rail_btn_disconnect->setObjectName(QString::fromUtf8("rail_btn_disconnect"));

        gridLayout->addWidget(rail_btn_disconnect, 0, 1, 1, 1);

        rail_btn_absLocate = new QPushButton(groupbox);
        rail_btn_absLocate->setObjectName(QString::fromUtf8("rail_btn_absLocate"));

        gridLayout->addWidget(rail_btn_absLocate, 1, 0, 1, 1);

        rail_btn_stop = new QPushButton(groupbox);
        rail_btn_stop->setObjectName(QString::fromUtf8("rail_btn_stop"));

        gridLayout->addWidget(rail_btn_stop, 1, 1, 1, 1);


        verticalLayout_4->addWidget(groupbox);


        horizontalLayout_8->addWidget(widget_rail);

        widget_platform = new QWidget(widget_7);
        widget_platform->setObjectName(QString::fromUtf8("widget_platform"));
        verticalLayout_5 = new QVBoxLayout(widget_platform);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        groupBox_3 = new QGroupBox(widget_platform);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        verticalLayout_3 = new QVBoxLayout(groupBox_3);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        table_axispos = new QTableWidget(groupBox_3);
        if (table_axispos->columnCount() < 3)
            table_axispos->setColumnCount(3);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        table_axispos->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        table_axispos->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        table_axispos->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        if (table_axispos->rowCount() < 7)
            table_axispos->setRowCount(7);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        table_axispos->setVerticalHeaderItem(0, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        table_axispos->setVerticalHeaderItem(1, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        table_axispos->setVerticalHeaderItem(2, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        table_axispos->setVerticalHeaderItem(3, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        table_axispos->setVerticalHeaderItem(4, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        table_axispos->setVerticalHeaderItem(5, __qtablewidgetitem8);
        QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
        table_axispos->setVerticalHeaderItem(6, __qtablewidgetitem9);
        table_axispos->setObjectName(QString::fromUtf8("table_axispos"));
        table_axispos->setMinimumSize(QSize(250, 250));

        verticalLayout_3->addWidget(table_axispos);

        widget_6 = new QWidget(groupBox_3);
        widget_6->setObjectName(QString::fromUtf8("widget_6"));
        horizontalLayout_7 = new QHBoxLayout(widget_6);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        plt_btn_singlepltmover = new QPushButton(widget_6);
        plt_btn_singlepltmover->setObjectName(QString::fromUtf8("plt_btn_singlepltmover"));

        horizontalLayout_7->addWidget(plt_btn_singlepltmover);

        combo_singlepltchoose = new QComboBox(widget_6);
        combo_singlepltchoose->addItem(QString());
        combo_singlepltchoose->addItem(QString());
        combo_singlepltchoose->addItem(QString());
        combo_singlepltchoose->addItem(QString());
        combo_singlepltchoose->addItem(QString());
        combo_singlepltchoose->addItem(QString());
        combo_singlepltchoose->addItem(QString());
        combo_singlepltchoose->setObjectName(QString::fromUtf8("combo_singlepltchoose"));

        horizontalLayout_7->addWidget(combo_singlepltchoose);


        verticalLayout_3->addWidget(widget_6);

        widget_5 = new QWidget(groupBox_3);
        widget_5->setObjectName(QString::fromUtf8("widget_5"));
        horizontalLayout_6 = new QHBoxLayout(widget_5);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        plt_btn_singleaxismover = new QPushButton(widget_5);
        plt_btn_singleaxismover->setObjectName(QString::fromUtf8("plt_btn_singleaxismover"));

        horizontalLayout_6->addWidget(plt_btn_singleaxismover);

        combo_singleaxispltchoose = new QComboBox(widget_5);
        combo_singleaxispltchoose->addItem(QString());
        combo_singleaxispltchoose->addItem(QString());
        combo_singleaxispltchoose->addItem(QString());
        combo_singleaxispltchoose->addItem(QString());
        combo_singleaxispltchoose->addItem(QString());
        combo_singleaxispltchoose->addItem(QString());
        combo_singleaxispltchoose->addItem(QString());
        combo_singleaxispltchoose->setObjectName(QString::fromUtf8("combo_singleaxispltchoose"));

        horizontalLayout_6->addWidget(combo_singleaxispltchoose);

        combo_singleaxischoose = new QComboBox(widget_5);
        combo_singleaxischoose->addItem(QString());
        combo_singleaxischoose->addItem(QString());
        combo_singleaxischoose->addItem(QString());
        combo_singleaxischoose->setObjectName(QString::fromUtf8("combo_singleaxischoose"));

        horizontalLayout_6->addWidget(combo_singleaxischoose);

        plt_edit_singleaxismoverpos = new QLineEdit(widget_5);
        plt_edit_singleaxismoverpos->setObjectName(QString::fromUtf8("plt_edit_singleaxismoverpos"));

        horizontalLayout_6->addWidget(plt_edit_singleaxismoverpos);


        verticalLayout_3->addWidget(widget_5);


        verticalLayout_5->addWidget(groupBox_3);

        groupBox_4 = new QGroupBox(widget_platform);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        gridLayout_2 = new QGridLayout(groupBox_4);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        plt_btn_resetAll = new QPushButton(groupBox_4);
        plt_btn_resetAll->setObjectName(QString::fromUtf8("plt_btn_resetAll"));

        gridLayout_2->addWidget(plt_btn_resetAll, 2, 0, 1, 1);

        plt_btn_enableAll = new QPushButton(groupBox_4);
        plt_btn_enableAll->setObjectName(QString::fromUtf8("plt_btn_enableAll"));

        gridLayout_2->addWidget(plt_btn_enableAll, 0, 0, 1, 1);

        plt_btn_homeAll = new QPushButton(groupBox_4);
        plt_btn_homeAll->setObjectName(QString::fromUtf8("plt_btn_homeAll"));

        gridLayout_2->addWidget(plt_btn_homeAll, 1, 0, 1, 1);

        plt_btn_disenableAll = new QPushButton(groupBox_4);
        plt_btn_disenableAll->setObjectName(QString::fromUtf8("plt_btn_disenableAll"));

        gridLayout_2->addWidget(plt_btn_disenableAll, 0, 1, 1, 1);

        plt_btn_locateAll = new QPushButton(groupBox_4);
        plt_btn_locateAll->setObjectName(QString::fromUtf8("plt_btn_locateAll"));

        gridLayout_2->addWidget(plt_btn_locateAll, 1, 1, 1, 1);

        plt_btn_stopAll = new QPushButton(groupBox_4);
        plt_btn_stopAll->setObjectName(QString::fromUtf8("plt_btn_stopAll"));

        gridLayout_2->addWidget(plt_btn_stopAll, 2, 1, 1, 1);


        verticalLayout_5->addWidget(groupBox_4);


        horizontalLayout_8->addWidget(widget_platform);


        verticalLayout_6->addWidget(widget_7);

        widget_8 = new QWidget(MotionWidget);
        widget_8->setObjectName(QString::fromUtf8("widget_8"));
        gridLayout_3 = new QGridLayout(widget_8);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        card_rail = new QFrame(widget_8);
        card_rail->setObjectName(QString::fromUtf8("card_rail"));
        card_rail->setStyleSheet(QString::fromUtf8("QFrame#card_rail, QFrame#card_plt0, QFrame#card_plt1, QFrame#card_plt2,\n"
"QFrame#card_plt3, QFrame#card_plt4, QFrame#card_plt5, QFrame#card_plt6 {\n"
"    background: #ffffff;\n"
"    border: 1px solid #b0b8c1;\n"
"    border-radius: 4px;\n"
"}"));
        card_rail->setFrameShape(QFrame::StyledPanel);
        card_rail->setFrameShadow(QFrame::Raised);
        verticalLayout_7 = new QVBoxLayout(card_rail);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        widget_11 = new QWidget(card_rail);
        widget_11->setObjectName(QString::fromUtf8("widget_11"));
        horizontalLayout = new QHBoxLayout(widget_11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_5 = new QLabel(widget_11);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout->addWidget(label_5);

        rail_label_status = new QLabel(widget_11);
        rail_label_status->setObjectName(QString::fromUtf8("rail_label_status"));
        rail_label_status->setMinimumSize(QSize(14, 14));
        rail_label_status->setMaximumSize(QSize(14, 14));
        rail_label_status->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout->addWidget(rail_label_status);


        verticalLayout_7->addWidget(widget_11);

        widget_12 = new QWidget(card_rail);
        widget_12->setObjectName(QString::fromUtf8("widget_12"));
        horizontalLayout_11 = new QHBoxLayout(widget_12);
        horizontalLayout_11->setObjectName(QString::fromUtf8("horizontalLayout_11"));
        label_11 = new QLabel(widget_12);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        horizontalLayout_11->addWidget(label_11);

        rail_label_state = new QLabel(widget_12);
        rail_label_state->setObjectName(QString::fromUtf8("rail_label_state"));

        horizontalLayout_11->addWidget(rail_label_state);


        verticalLayout_7->addWidget(widget_12);


        gridLayout_3->addWidget(card_rail, 0, 0, 1, 1);

        card_plt0 = new QFrame(widget_8);
        card_plt0->setObjectName(QString::fromUtf8("card_plt0"));
        card_plt0->setStyleSheet(QString::fromUtf8("QFrame#card_rail, QFrame#card_plt0, QFrame#card_plt1, QFrame#card_plt2,\n"
"QFrame#card_plt3, QFrame#card_plt4, QFrame#card_plt5, QFrame#card_plt6 {\n"
"    background: #ffffff;\n"
"    border: 1px solid #b0b8c1;\n"
"    border-radius: 4px;\n"
"}"));
        card_plt0->setFrameShape(QFrame::StyledPanel);
        card_plt0->setFrameShadow(QFrame::Raised);
        verticalLayout_8 = new QVBoxLayout(card_plt0);
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        widget_13 = new QWidget(card_plt0);
        widget_13->setObjectName(QString::fromUtf8("widget_13"));
        horizontalLayout_9 = new QHBoxLayout(widget_13);
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        label_13 = new QLabel(widget_13);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        horizontalLayout_9->addWidget(label_13);

        plt0_label_Xstatus = new QLabel(widget_13);
        plt0_label_Xstatus->setObjectName(QString::fromUtf8("plt0_label_Xstatus"));
        plt0_label_Xstatus->setMinimumSize(QSize(14, 14));
        plt0_label_Xstatus->setMaximumSize(QSize(14, 14));
        plt0_label_Xstatus->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout_9->addWidget(plt0_label_Xstatus);

        plt0_label_Ystatus = new QLabel(widget_13);
        plt0_label_Ystatus->setObjectName(QString::fromUtf8("plt0_label_Ystatus"));
        plt0_label_Ystatus->setMinimumSize(QSize(14, 14));
        plt0_label_Ystatus->setMaximumSize(QSize(14, 14));
        plt0_label_Ystatus->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout_9->addWidget(plt0_label_Ystatus);

        plt0_label_Zstatus = new QLabel(widget_13);
        plt0_label_Zstatus->setObjectName(QString::fromUtf8("plt0_label_Zstatus"));
        plt0_label_Zstatus->setMinimumSize(QSize(14, 14));
        plt0_label_Zstatus->setMaximumSize(QSize(14, 14));
        plt0_label_Zstatus->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout_9->addWidget(plt0_label_Zstatus);


        verticalLayout_8->addWidget(widget_13);

        widget_16 = new QWidget(card_plt0);
        widget_16->setObjectName(QString::fromUtf8("widget_16"));
        horizontalLayout_15 = new QHBoxLayout(widget_16);
        horizontalLayout_15->setObjectName(QString::fromUtf8("horizontalLayout_15"));
        label_19 = new QLabel(widget_16);
        label_19->setObjectName(QString::fromUtf8("label_19"));

        horizontalLayout_15->addWidget(label_19);

        plt0_label_state = new QLabel(widget_16);
        plt0_label_state->setObjectName(QString::fromUtf8("plt0_label_state"));

        horizontalLayout_15->addWidget(plt0_label_state);


        verticalLayout_8->addWidget(widget_16);


        gridLayout_3->addWidget(card_plt0, 0, 1, 1, 1);

        card_plt1 = new QFrame(widget_8);
        card_plt1->setObjectName(QString::fromUtf8("card_plt1"));
        card_plt1->setStyleSheet(QString::fromUtf8("QFrame#card_rail, QFrame#card_plt0, QFrame#card_plt1, QFrame#card_plt2,\n"
"QFrame#card_plt3, QFrame#card_plt4, QFrame#card_plt5, QFrame#card_plt6 {\n"
"    background: #ffffff;\n"
"    border: 1px solid #b0b8c1;\n"
"    border-radius: 4px;\n"
"}"));
        card_plt1->setFrameShape(QFrame::StyledPanel);
        card_plt1->setFrameShadow(QFrame::Raised);
        verticalLayout_9 = new QVBoxLayout(card_plt1);
        verticalLayout_9->setObjectName(QString::fromUtf8("verticalLayout_9"));
        widget_17 = new QWidget(card_plt1);
        widget_17->setObjectName(QString::fromUtf8("widget_17"));
        horizontalLayout_10 = new QHBoxLayout(widget_17);
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
        label_21 = new QLabel(widget_17);
        label_21->setObjectName(QString::fromUtf8("label_21"));

        horizontalLayout_10->addWidget(label_21);

        plt1_label_Xstatus = new QLabel(widget_17);
        plt1_label_Xstatus->setObjectName(QString::fromUtf8("plt1_label_Xstatus"));
        plt1_label_Xstatus->setMinimumSize(QSize(14, 14));
        plt1_label_Xstatus->setMaximumSize(QSize(14, 14));
        plt1_label_Xstatus->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout_10->addWidget(plt1_label_Xstatus);

        plt1_label_Ystatus = new QLabel(widget_17);
        plt1_label_Ystatus->setObjectName(QString::fromUtf8("plt1_label_Ystatus"));
        plt1_label_Ystatus->setMinimumSize(QSize(14, 14));
        plt1_label_Ystatus->setMaximumSize(QSize(14, 14));
        plt1_label_Ystatus->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout_10->addWidget(plt1_label_Ystatus);

        plt1_label_Zstatus = new QLabel(widget_17);
        plt1_label_Zstatus->setObjectName(QString::fromUtf8("plt1_label_Zstatus"));
        plt1_label_Zstatus->setMinimumSize(QSize(14, 14));
        plt1_label_Zstatus->setMaximumSize(QSize(14, 14));
        plt1_label_Zstatus->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout_10->addWidget(plt1_label_Zstatus);


        verticalLayout_9->addWidget(widget_17);

        widget_20 = new QWidget(card_plt1);
        widget_20->setObjectName(QString::fromUtf8("widget_20"));
        horizontalLayout_19 = new QHBoxLayout(widget_20);
        horizontalLayout_19->setObjectName(QString::fromUtf8("horizontalLayout_19"));
        label_27 = new QLabel(widget_20);
        label_27->setObjectName(QString::fromUtf8("label_27"));

        horizontalLayout_19->addWidget(label_27);

        plt1_label_state = new QLabel(widget_20);
        plt1_label_state->setObjectName(QString::fromUtf8("plt1_label_state"));

        horizontalLayout_19->addWidget(plt1_label_state);


        verticalLayout_9->addWidget(widget_20);


        gridLayout_3->addWidget(card_plt1, 0, 2, 1, 1);

        card_plt2 = new QFrame(widget_8);
        card_plt2->setObjectName(QString::fromUtf8("card_plt2"));
        card_plt2->setStyleSheet(QString::fromUtf8("QFrame#card_rail, QFrame#card_plt0, QFrame#card_plt1, QFrame#card_plt2,\n"
"QFrame#card_plt3, QFrame#card_plt4, QFrame#card_plt5, QFrame#card_plt6 {\n"
"    background: #ffffff;\n"
"    border: 1px solid #b0b8c1;\n"
"    border-radius: 4px;\n"
"}"));
        card_plt2->setFrameShape(QFrame::StyledPanel);
        card_plt2->setFrameShadow(QFrame::Raised);
        verticalLayout_10 = new QVBoxLayout(card_plt2);
        verticalLayout_10->setObjectName(QString::fromUtf8("verticalLayout_10"));
        widget_21 = new QWidget(card_plt2);
        widget_21->setObjectName(QString::fromUtf8("widget_21"));
        horizontalLayout_12 = new QHBoxLayout(widget_21);
        horizontalLayout_12->setObjectName(QString::fromUtf8("horizontalLayout_12"));
        label_29 = new QLabel(widget_21);
        label_29->setObjectName(QString::fromUtf8("label_29"));

        horizontalLayout_12->addWidget(label_29);

        plt2_label_Xstatus = new QLabel(widget_21);
        plt2_label_Xstatus->setObjectName(QString::fromUtf8("plt2_label_Xstatus"));
        plt2_label_Xstatus->setMinimumSize(QSize(14, 14));
        plt2_label_Xstatus->setMaximumSize(QSize(14, 14));
        plt2_label_Xstatus->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout_12->addWidget(plt2_label_Xstatus);

        plt2_label_Ystatus = new QLabel(widget_21);
        plt2_label_Ystatus->setObjectName(QString::fromUtf8("plt2_label_Ystatus"));
        plt2_label_Ystatus->setMinimumSize(QSize(14, 14));
        plt2_label_Ystatus->setMaximumSize(QSize(14, 14));
        plt2_label_Ystatus->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout_12->addWidget(plt2_label_Ystatus);

        plt2_label_Zstatus = new QLabel(widget_21);
        plt2_label_Zstatus->setObjectName(QString::fromUtf8("plt2_label_Zstatus"));
        plt2_label_Zstatus->setMinimumSize(QSize(14, 14));
        plt2_label_Zstatus->setMaximumSize(QSize(14, 14));
        plt2_label_Zstatus->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout_12->addWidget(plt2_label_Zstatus);


        verticalLayout_10->addWidget(widget_21);

        widget_24 = new QWidget(card_plt2);
        widget_24->setObjectName(QString::fromUtf8("widget_24"));
        horizontalLayout_23 = new QHBoxLayout(widget_24);
        horizontalLayout_23->setObjectName(QString::fromUtf8("horizontalLayout_23"));
        label_35 = new QLabel(widget_24);
        label_35->setObjectName(QString::fromUtf8("label_35"));

        horizontalLayout_23->addWidget(label_35);

        plt2_label_state = new QLabel(widget_24);
        plt2_label_state->setObjectName(QString::fromUtf8("plt2_label_state"));

        horizontalLayout_23->addWidget(plt2_label_state);


        verticalLayout_10->addWidget(widget_24);


        gridLayout_3->addWidget(card_plt2, 0, 3, 1, 1);

        card_plt3 = new QFrame(widget_8);
        card_plt3->setObjectName(QString::fromUtf8("card_plt3"));
        card_plt3->setStyleSheet(QString::fromUtf8("QFrame#card_rail, QFrame#card_plt0, QFrame#card_plt1, QFrame#card_plt2,\n"
"QFrame#card_plt3, QFrame#card_plt4, QFrame#card_plt5, QFrame#card_plt6 {\n"
"    background: #ffffff;\n"
"    border: 1px solid #b0b8c1;\n"
"    border-radius: 4px;\n"
"}"));
        card_plt3->setFrameShape(QFrame::StyledPanel);
        card_plt3->setFrameShadow(QFrame::Raised);
        verticalLayout_11 = new QVBoxLayout(card_plt3);
        verticalLayout_11->setObjectName(QString::fromUtf8("verticalLayout_11"));
        widget_25 = new QWidget(card_plt3);
        widget_25->setObjectName(QString::fromUtf8("widget_25"));
        horizontalLayout_13 = new QHBoxLayout(widget_25);
        horizontalLayout_13->setObjectName(QString::fromUtf8("horizontalLayout_13"));
        label_37 = new QLabel(widget_25);
        label_37->setObjectName(QString::fromUtf8("label_37"));

        horizontalLayout_13->addWidget(label_37);

        plt3_label_Xstatus = new QLabel(widget_25);
        plt3_label_Xstatus->setObjectName(QString::fromUtf8("plt3_label_Xstatus"));
        plt3_label_Xstatus->setMinimumSize(QSize(14, 14));
        plt3_label_Xstatus->setMaximumSize(QSize(14, 14));
        plt3_label_Xstatus->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout_13->addWidget(plt3_label_Xstatus);

        plt3_label_Ystatus = new QLabel(widget_25);
        plt3_label_Ystatus->setObjectName(QString::fromUtf8("plt3_label_Ystatus"));
        plt3_label_Ystatus->setMinimumSize(QSize(14, 14));
        plt3_label_Ystatus->setMaximumSize(QSize(14, 14));
        plt3_label_Ystatus->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout_13->addWidget(plt3_label_Ystatus);

        plt3_label_Zstatus = new QLabel(widget_25);
        plt3_label_Zstatus->setObjectName(QString::fromUtf8("plt3_label_Zstatus"));
        plt3_label_Zstatus->setMinimumSize(QSize(14, 14));
        plt3_label_Zstatus->setMaximumSize(QSize(14, 14));
        plt3_label_Zstatus->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout_13->addWidget(plt3_label_Zstatus);


        verticalLayout_11->addWidget(widget_25);

        widget_28 = new QWidget(card_plt3);
        widget_28->setObjectName(QString::fromUtf8("widget_28"));
        horizontalLayout_27 = new QHBoxLayout(widget_28);
        horizontalLayout_27->setObjectName(QString::fromUtf8("horizontalLayout_27"));
        label_43 = new QLabel(widget_28);
        label_43->setObjectName(QString::fromUtf8("label_43"));

        horizontalLayout_27->addWidget(label_43);

        plt3_label_state = new QLabel(widget_28);
        plt3_label_state->setObjectName(QString::fromUtf8("plt3_label_state"));

        horizontalLayout_27->addWidget(plt3_label_state);


        verticalLayout_11->addWidget(widget_28);


        gridLayout_3->addWidget(card_plt3, 1, 0, 1, 1);

        card_plt4 = new QFrame(widget_8);
        card_plt4->setObjectName(QString::fromUtf8("card_plt4"));
        card_plt4->setStyleSheet(QString::fromUtf8("QFrame#card_rail, QFrame#card_plt0, QFrame#card_plt1, QFrame#card_plt2,\n"
"QFrame#card_plt3, QFrame#card_plt4, QFrame#card_plt5, QFrame#card_plt6 {\n"
"    background: #ffffff;\n"
"    border: 1px solid #b0b8c1;\n"
"    border-radius: 4px;\n"
"}"));
        card_plt4->setFrameShape(QFrame::StyledPanel);
        card_plt4->setFrameShadow(QFrame::Raised);
        verticalLayout_12 = new QVBoxLayout(card_plt4);
        verticalLayout_12->setObjectName(QString::fromUtf8("verticalLayout_12"));
        widget_29 = new QWidget(card_plt4);
        widget_29->setObjectName(QString::fromUtf8("widget_29"));
        horizontalLayout_14 = new QHBoxLayout(widget_29);
        horizontalLayout_14->setObjectName(QString::fromUtf8("horizontalLayout_14"));
        label_45 = new QLabel(widget_29);
        label_45->setObjectName(QString::fromUtf8("label_45"));

        horizontalLayout_14->addWidget(label_45);

        plt4_label_Xstatus = new QLabel(widget_29);
        plt4_label_Xstatus->setObjectName(QString::fromUtf8("plt4_label_Xstatus"));
        plt4_label_Xstatus->setMinimumSize(QSize(14, 14));
        plt4_label_Xstatus->setMaximumSize(QSize(14, 14));
        plt4_label_Xstatus->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout_14->addWidget(plt4_label_Xstatus);

        plt4_label_Ystatus = new QLabel(widget_29);
        plt4_label_Ystatus->setObjectName(QString::fromUtf8("plt4_label_Ystatus"));
        plt4_label_Ystatus->setMinimumSize(QSize(14, 14));
        plt4_label_Ystatus->setMaximumSize(QSize(14, 14));
        plt4_label_Ystatus->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout_14->addWidget(plt4_label_Ystatus);

        plt4_label_Zstatus = new QLabel(widget_29);
        plt4_label_Zstatus->setObjectName(QString::fromUtf8("plt4_label_Zstatus"));
        plt4_label_Zstatus->setMinimumSize(QSize(14, 14));
        plt4_label_Zstatus->setMaximumSize(QSize(14, 14));
        plt4_label_Zstatus->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout_14->addWidget(plt4_label_Zstatus);


        verticalLayout_12->addWidget(widget_29);

        widget_32 = new QWidget(card_plt4);
        widget_32->setObjectName(QString::fromUtf8("widget_32"));
        horizontalLayout_31 = new QHBoxLayout(widget_32);
        horizontalLayout_31->setObjectName(QString::fromUtf8("horizontalLayout_31"));
        label_51 = new QLabel(widget_32);
        label_51->setObjectName(QString::fromUtf8("label_51"));

        horizontalLayout_31->addWidget(label_51);

        plt4_label_state = new QLabel(widget_32);
        plt4_label_state->setObjectName(QString::fromUtf8("plt4_label_state"));

        horizontalLayout_31->addWidget(plt4_label_state);


        verticalLayout_12->addWidget(widget_32);


        gridLayout_3->addWidget(card_plt4, 1, 1, 1, 1);

        card_plt5 = new QFrame(widget_8);
        card_plt5->setObjectName(QString::fromUtf8("card_plt5"));
        card_plt5->setStyleSheet(QString::fromUtf8("QFrame#card_rail, QFrame#card_plt0, QFrame#card_plt1, QFrame#card_plt2,\n"
"QFrame#card_plt3, QFrame#card_plt4, QFrame#card_plt5, QFrame#card_plt6 {\n"
"    background: #ffffff;\n"
"    border: 1px solid #b0b8c1;\n"
"    border-radius: 4px;\n"
"}"));
        card_plt5->setFrameShape(QFrame::StyledPanel);
        card_plt5->setFrameShadow(QFrame::Raised);
        verticalLayout_15 = new QVBoxLayout(card_plt5);
        verticalLayout_15->setObjectName(QString::fromUtf8("verticalLayout_15"));
        widget_41 = new QWidget(card_plt5);
        widget_41->setObjectName(QString::fromUtf8("widget_41"));
        horizontalLayout_16 = new QHBoxLayout(widget_41);
        horizontalLayout_16->setObjectName(QString::fromUtf8("horizontalLayout_16"));
        label_69 = new QLabel(widget_41);
        label_69->setObjectName(QString::fromUtf8("label_69"));

        horizontalLayout_16->addWidget(label_69);

        plt5_label_Xstatus = new QLabel(widget_41);
        plt5_label_Xstatus->setObjectName(QString::fromUtf8("plt5_label_Xstatus"));
        plt5_label_Xstatus->setMinimumSize(QSize(14, 14));
        plt5_label_Xstatus->setMaximumSize(QSize(14, 14));
        plt5_label_Xstatus->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout_16->addWidget(plt5_label_Xstatus);

        plt5_label_Ystatus = new QLabel(widget_41);
        plt5_label_Ystatus->setObjectName(QString::fromUtf8("plt5_label_Ystatus"));
        plt5_label_Ystatus->setMinimumSize(QSize(14, 14));
        plt5_label_Ystatus->setMaximumSize(QSize(14, 14));
        plt5_label_Ystatus->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout_16->addWidget(plt5_label_Ystatus);

        plt5_label_Zstatus = new QLabel(widget_41);
        plt5_label_Zstatus->setObjectName(QString::fromUtf8("plt5_label_Zstatus"));
        plt5_label_Zstatus->setMinimumSize(QSize(14, 14));
        plt5_label_Zstatus->setMaximumSize(QSize(14, 14));
        plt5_label_Zstatus->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout_16->addWidget(plt5_label_Zstatus);


        verticalLayout_15->addWidget(widget_41);

        widget_44 = new QWidget(card_plt5);
        widget_44->setObjectName(QString::fromUtf8("widget_44"));
        horizontalLayout_43 = new QHBoxLayout(widget_44);
        horizontalLayout_43->setObjectName(QString::fromUtf8("horizontalLayout_43"));
        label_75 = new QLabel(widget_44);
        label_75->setObjectName(QString::fromUtf8("label_75"));

        horizontalLayout_43->addWidget(label_75);

        plt5_label_state = new QLabel(widget_44);
        plt5_label_state->setObjectName(QString::fromUtf8("plt5_label_state"));

        horizontalLayout_43->addWidget(plt5_label_state);


        verticalLayout_15->addWidget(widget_44);


        gridLayout_3->addWidget(card_plt5, 1, 2, 1, 1);

        card_plt6 = new QFrame(widget_8);
        card_plt6->setObjectName(QString::fromUtf8("card_plt6"));
        card_plt6->setStyleSheet(QString::fromUtf8("QFrame#card_rail, QFrame#card_plt0, QFrame#card_plt1, QFrame#card_plt2,\n"
"QFrame#card_plt3, QFrame#card_plt4, QFrame#card_plt5, QFrame#card_plt6 {\n"
"    background: #ffffff;\n"
"    border: 1px solid #b0b8c1;\n"
"    border-radius: 4px;\n"
"}"));
        card_plt6->setFrameShape(QFrame::StyledPanel);
        card_plt6->setFrameShadow(QFrame::Raised);
        verticalLayout_16 = new QVBoxLayout(card_plt6);
        verticalLayout_16->setObjectName(QString::fromUtf8("verticalLayout_16"));
        widget_45 = new QWidget(card_plt6);
        widget_45->setObjectName(QString::fromUtf8("widget_45"));
        horizontalLayout_17 = new QHBoxLayout(widget_45);
        horizontalLayout_17->setObjectName(QString::fromUtf8("horizontalLayout_17"));
        label_77 = new QLabel(widget_45);
        label_77->setObjectName(QString::fromUtf8("label_77"));

        horizontalLayout_17->addWidget(label_77);

        plt6_label_Xstatus = new QLabel(widget_45);
        plt6_label_Xstatus->setObjectName(QString::fromUtf8("plt6_label_Xstatus"));
        plt6_label_Xstatus->setMinimumSize(QSize(14, 14));
        plt6_label_Xstatus->setMaximumSize(QSize(14, 14));
        plt6_label_Xstatus->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout_17->addWidget(plt6_label_Xstatus);

        plt6_label_Ystatus = new QLabel(widget_45);
        plt6_label_Ystatus->setObjectName(QString::fromUtf8("plt6_label_Ystatus"));
        plt6_label_Ystatus->setMinimumSize(QSize(14, 14));
        plt6_label_Ystatus->setMaximumSize(QSize(14, 14));
        plt6_label_Ystatus->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout_17->addWidget(plt6_label_Ystatus);

        plt6_label_Zstatus = new QLabel(widget_45);
        plt6_label_Zstatus->setObjectName(QString::fromUtf8("plt6_label_Zstatus"));
        plt6_label_Zstatus->setMinimumSize(QSize(14, 14));
        plt6_label_Zstatus->setMaximumSize(QSize(14, 14));
        plt6_label_Zstatus->setStyleSheet(QString::fromUtf8("    background-color: #9ca3af;   /* \351\273\230\350\256\244\347\201\260\350\211\262 */\n"
"    border-radius: 7px;          /* \345\256\275\351\253\230\347\232\204\344\270\200\345\215\212 = \346\255\243\345\234\206 */\n"
"    border: 1px solid #d1d5db;"));

        horizontalLayout_17->addWidget(plt6_label_Zstatus);


        verticalLayout_16->addWidget(widget_45);

        widget_48 = new QWidget(card_plt6);
        widget_48->setObjectName(QString::fromUtf8("widget_48"));
        horizontalLayout_47 = new QHBoxLayout(widget_48);
        horizontalLayout_47->setObjectName(QString::fromUtf8("horizontalLayout_47"));
        label_83 = new QLabel(widget_48);
        label_83->setObjectName(QString::fromUtf8("label_83"));

        horizontalLayout_47->addWidget(label_83);

        plt6_label_state = new QLabel(widget_48);
        plt6_label_state->setObjectName(QString::fromUtf8("plt6_label_state"));

        horizontalLayout_47->addWidget(plt6_label_state);


        verticalLayout_16->addWidget(widget_48);


        gridLayout_3->addWidget(card_plt6, 1, 3, 1, 1);


        verticalLayout_6->addWidget(widget_8);


        retranslateUi(MotionWidget);

        QMetaObject::connectSlotsByName(MotionWidget);
    } // setupUi

    void retranslateUi(QWidget *MotionWidget)
    {
        MotionWidget->setWindowTitle(QCoreApplication::translate("MotionWidget", "RailWidget", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MotionWidget", "\345\234\260\350\275\250\346\216\247\345\210\266\345\217\202\346\225\260\350\256\276\347\275\256", nullptr));
        label->setText(QCoreApplication::translate("MotionWidget", "\350\277\220\345\212\250\344\275\215\347\275\256", nullptr));
        label_2->setText(QCoreApplication::translate("MotionWidget", "\350\277\220\345\212\250\351\200\237\345\272\246", nullptr));
        label_3->setText(QCoreApplication::translate("MotionWidget", "\345\275\223\345\211\215\344\275\215\347\275\256", nullptr));
        label_4->setText(QCoreApplication::translate("MotionWidget", "\345\275\223\345\211\215\351\200\237\345\272\246", nullptr));
        groupbox->setTitle(QCoreApplication::translate("MotionWidget", "\345\234\260\350\275\250\346\216\247\345\210\266\345\221\275\344\273\244", nullptr));
        rail_btn_connect->setText(QCoreApplication::translate("MotionWidget", "\350\277\236\346\216\245\345\234\260\350\275\250", nullptr));
        rail_btn_disconnect->setText(QCoreApplication::translate("MotionWidget", "\346\226\255\345\274\200\345\234\260\350\275\250", nullptr));
        rail_btn_absLocate->setText(QCoreApplication::translate("MotionWidget", "\347\273\235\345\257\271\345\256\232\344\275\215", nullptr));
        rail_btn_stop->setText(QCoreApplication::translate("MotionWidget", "\345\201\234\346\255\242\350\277\220\345\212\250", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\260\350\277\220\345\212\250\344\275\215\347\275\256\350\256\276\347\275\256", nullptr));
        QTableWidgetItem *___qtablewidgetitem = table_axispos->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("MotionWidget", "X", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = table_axispos->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("MotionWidget", "Y", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = table_axispos->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("MotionWidget", "Z", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = table_axispos->verticalHeaderItem(0);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2600", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = table_axispos->verticalHeaderItem(1);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2601", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = table_axispos->verticalHeaderItem(2);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2602", nullptr));
        QTableWidgetItem *___qtablewidgetitem6 = table_axispos->verticalHeaderItem(3);
        ___qtablewidgetitem6->setText(QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2603", nullptr));
        QTableWidgetItem *___qtablewidgetitem7 = table_axispos->verticalHeaderItem(4);
        ___qtablewidgetitem7->setText(QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2604", nullptr));
        QTableWidgetItem *___qtablewidgetitem8 = table_axispos->verticalHeaderItem(5);
        ___qtablewidgetitem8->setText(QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2605", nullptr));
        QTableWidgetItem *___qtablewidgetitem9 = table_axispos->verticalHeaderItem(6);
        ___qtablewidgetitem9->setText(QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2606", nullptr));
        plt_btn_singlepltmover->setText(QCoreApplication::translate("MotionWidget", "\345\215\225\345\257\271\344\275\215\345\271\263\345\217\260\350\277\220\345\212\250", nullptr));
        combo_singlepltchoose->setItemText(0, QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2600", nullptr));
        combo_singlepltchoose->setItemText(1, QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2601", nullptr));
        combo_singlepltchoose->setItemText(2, QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2602", nullptr));
        combo_singlepltchoose->setItemText(3, QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2603", nullptr));
        combo_singlepltchoose->setItemText(4, QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2604", nullptr));
        combo_singlepltchoose->setItemText(5, QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2605", nullptr));
        combo_singlepltchoose->setItemText(6, QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2606", nullptr));

        plt_btn_singleaxismover->setText(QCoreApplication::translate("MotionWidget", "\345\215\225\350\275\264\350\277\220\345\212\250", nullptr));
        combo_singleaxispltchoose->setItemText(0, QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2600", nullptr));
        combo_singleaxispltchoose->setItemText(1, QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2601", nullptr));
        combo_singleaxispltchoose->setItemText(2, QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2602", nullptr));
        combo_singleaxispltchoose->setItemText(3, QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2603", nullptr));
        combo_singleaxispltchoose->setItemText(4, QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2604", nullptr));
        combo_singleaxispltchoose->setItemText(5, QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2605", nullptr));
        combo_singleaxispltchoose->setItemText(6, QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2606", nullptr));

        combo_singleaxischoose->setItemText(0, QCoreApplication::translate("MotionWidget", "X", nullptr));
        combo_singleaxischoose->setItemText(1, QCoreApplication::translate("MotionWidget", "Y", nullptr));
        combo_singleaxischoose->setItemText(2, QCoreApplication::translate("MotionWidget", "Z", nullptr));

        groupBox_4->setTitle(QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\260\350\201\224\346\216\247", nullptr));
        plt_btn_resetAll->setText(QCoreApplication::translate("MotionWidget", "\344\270\200\351\224\256\345\244\215\344\275\215", nullptr));
        plt_btn_enableAll->setText(QCoreApplication::translate("MotionWidget", "\344\270\200\351\224\256\344\275\277\350\203\275", nullptr));
        plt_btn_homeAll->setText(QCoreApplication::translate("MotionWidget", "\344\270\200\351\224\256\345\233\236\345\217\202", nullptr));
        plt_btn_disenableAll->setText(QCoreApplication::translate("MotionWidget", "\344\270\200\351\224\256\345\216\273\344\275\277\350\203\275", nullptr));
        plt_btn_locateAll->setText(QCoreApplication::translate("MotionWidget", "\344\270\200\351\224\256\345\256\232\344\275\215", nullptr));
        plt_btn_stopAll->setText(QCoreApplication::translate("MotionWidget", "\344\270\200\351\224\256\345\201\234\346\255\242", nullptr));
        label_5->setText(QCoreApplication::translate("MotionWidget", "\345\234\260\350\275\250", nullptr));
        rail_label_status->setText(QString());
        label_11->setText(QCoreApplication::translate("MotionWidget", "\345\267\245\344\275\234\347\212\266\346\200\201", nullptr));
        rail_label_state->setText(QCoreApplication::translate("MotionWidget", "TextLabel", nullptr));
        label_13->setText(QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2600", nullptr));
        plt0_label_Xstatus->setText(QString());
        plt0_label_Ystatus->setText(QString());
        plt0_label_Zstatus->setText(QString());
        label_19->setText(QCoreApplication::translate("MotionWidget", "\345\267\245\344\275\234\347\212\266\346\200\201", nullptr));
        plt0_label_state->setText(QCoreApplication::translate("MotionWidget", "TextLabel", nullptr));
        label_21->setText(QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2601", nullptr));
        plt1_label_Xstatus->setText(QString());
        plt1_label_Ystatus->setText(QString());
        plt1_label_Zstatus->setText(QString());
        label_27->setText(QCoreApplication::translate("MotionWidget", "\345\267\245\344\275\234\347\212\266\346\200\201", nullptr));
        plt1_label_state->setText(QCoreApplication::translate("MotionWidget", "TextLabel", nullptr));
        label_29->setText(QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2602", nullptr));
        plt2_label_Xstatus->setText(QString());
        plt2_label_Ystatus->setText(QString());
        plt2_label_Zstatus->setText(QString());
        label_35->setText(QCoreApplication::translate("MotionWidget", "\345\267\245\344\275\234\347\212\266\346\200\201", nullptr));
        plt2_label_state->setText(QCoreApplication::translate("MotionWidget", "TextLabel", nullptr));
        label_37->setText(QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2603", nullptr));
        plt3_label_Xstatus->setText(QString());
        plt3_label_Ystatus->setText(QString());
        plt3_label_Zstatus->setText(QString());
        label_43->setText(QCoreApplication::translate("MotionWidget", "\345\267\245\344\275\234\347\212\266\346\200\201", nullptr));
        plt3_label_state->setText(QCoreApplication::translate("MotionWidget", "TextLabel", nullptr));
        label_45->setText(QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2604", nullptr));
        plt4_label_Xstatus->setText(QString());
        plt4_label_Ystatus->setText(QString());
        plt4_label_Zstatus->setText(QString());
        label_51->setText(QCoreApplication::translate("MotionWidget", "\345\267\245\344\275\234\347\212\266\346\200\201", nullptr));
        plt4_label_state->setText(QCoreApplication::translate("MotionWidget", "TextLabel", nullptr));
        label_69->setText(QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2605", nullptr));
        plt5_label_Xstatus->setText(QString());
        plt5_label_Ystatus->setText(QString());
        plt5_label_Zstatus->setText(QString());
        label_75->setText(QCoreApplication::translate("MotionWidget", "\345\267\245\344\275\234\347\212\266\346\200\201", nullptr));
        plt5_label_state->setText(QCoreApplication::translate("MotionWidget", "TextLabel", nullptr));
        label_77->setText(QCoreApplication::translate("MotionWidget", "\345\257\271\344\275\215\345\271\263\345\217\2606", nullptr));
        plt6_label_Xstatus->setText(QString());
        plt6_label_Ystatus->setText(QString());
        plt6_label_Zstatus->setText(QString());
        label_83->setText(QCoreApplication::translate("MotionWidget", "\345\267\245\344\275\234\347\212\266\346\200\201", nullptr));
        plt6_label_state->setText(QCoreApplication::translate("MotionWidget", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MotionWidget: public Ui_MotionWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MOTION_WIDGET_H
