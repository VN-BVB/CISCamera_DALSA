/********************************************************************************
** Form generated from reading UI file 'rail_widget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RAIL_WIDGET_H
#define UI_RAIL_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_RailWidget
{
public:
    QGridLayout *gridLayout;
    QGridLayout *gridLayout_2;
    QTextEdit *edit_MotionState;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_X_CurrentPosition;
    QLabel *label_7;
    QLabel *label_6;
    QWidget *widget;
    QGridLayout *gridLayout_4;
    QPushButton *btn_regressOrigin;
    QPushButton *btn_X_AbsPositionCommand;
    QPushButton *btn_X_JogReverse;
    QPushButton *btn_X_JogForward;
    QLabel *label_13;
    QLabel *label_17;
    QLabel *label_16;
    QLineEdit *edit_X_AbsSpeed;
    QSlider *horizontalSlider_X_AbsSpeed;
    QLabel *label_14;
    QLineEdit *edit_X_AbsPosition;
    QLabel *label_X_CurrentSpeed;
    QTextEdit *edit_AxisState;
    QSlider *horizontalSlider_X_AbsPosition;
    QVBoxLayout *verticalLayout_2;
    QWidget *widget_2;
    QGridLayout *gridLayout_5;
    QPushButton *btn_chk_Rest;
    QPushButton *btn_contectRail;
    QPushButton *btn_discontectRail;
    QHBoxLayout *horizontalLayout;
    QCheckBox *chk_ServoEnable;
    QCheckBox *chk_Stop;
    QCheckBox *chk_ImmediateStop;
    QTextEdit *textEdit;

    void setupUi(QWidget *RailWidget)
    {
        if (RailWidget->objectName().isEmpty())
            RailWidget->setObjectName(QString::fromUtf8("RailWidget"));
        RailWidget->resize(957, 648);
        QFont font;
        font.setPointSize(12);
        RailWidget->setFont(font);
        gridLayout = new QGridLayout(RailWidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setSizeConstraint(QLayout::SetNoConstraint);
        edit_MotionState = new QTextEdit(RailWidget);
        edit_MotionState->setObjectName(QString::fromUtf8("edit_MotionState"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(edit_MotionState->sizePolicy().hasHeightForWidth());
        edit_MotionState->setSizePolicy(sizePolicy);
        edit_MotionState->setMinimumSize(QSize(0, 0));
        edit_MotionState->setMaximumSize(QSize(16777215, 50));
        QFont font1;
        font1.setPointSize(10);
        font1.setBold(true);
        edit_MotionState->setFont(font1);
        edit_MotionState->setAutoFillBackground(true);
        edit_MotionState->setReadOnly(true);

        gridLayout_2->addWidget(edit_MotionState, 1, 1, 1, 2);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));

        gridLayout_2->addLayout(horizontalLayout_2, 9, 0, 2, 3);

        label_X_CurrentPosition = new QLabel(RailWidget);
        label_X_CurrentPosition->setObjectName(QString::fromUtf8("label_X_CurrentPosition"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_X_CurrentPosition->sizePolicy().hasHeightForWidth());
        label_X_CurrentPosition->setSizePolicy(sizePolicy1);
        label_X_CurrentPosition->setMinimumSize(QSize(0, 0));
        label_X_CurrentPosition->setMaximumSize(QSize(16777215, 50));

        gridLayout_2->addWidget(label_X_CurrentPosition, 6, 1, 1, 2);

        label_7 = new QLabel(RailWidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        gridLayout_2->addWidget(label_7, 4, 0, 1, 1);

        label_6 = new QLabel(RailWidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        sizePolicy1.setHeightForWidth(label_6->sizePolicy().hasHeightForWidth());
        label_6->setSizePolicy(sizePolicy1);

        gridLayout_2->addWidget(label_6, 2, 0, 1, 1);

        widget = new QWidget(RailWidget);
        widget->setObjectName(QString::fromUtf8("widget"));
        gridLayout_4 = new QGridLayout(widget);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        btn_regressOrigin = new QPushButton(widget);
        btn_regressOrigin->setObjectName(QString::fromUtf8("btn_regressOrigin"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(btn_regressOrigin->sizePolicy().hasHeightForWidth());
        btn_regressOrigin->setSizePolicy(sizePolicy2);
        btn_regressOrigin->setMinimumSize(QSize(0, 40));

        gridLayout_4->addWidget(btn_regressOrigin, 0, 1, 1, 1);

        btn_X_AbsPositionCommand = new QPushButton(widget);
        btn_X_AbsPositionCommand->setObjectName(QString::fromUtf8("btn_X_AbsPositionCommand"));
        sizePolicy2.setHeightForWidth(btn_X_AbsPositionCommand->sizePolicy().hasHeightForWidth());
        btn_X_AbsPositionCommand->setSizePolicy(sizePolicy2);
        btn_X_AbsPositionCommand->setMinimumSize(QSize(0, 40));

        gridLayout_4->addWidget(btn_X_AbsPositionCommand, 0, 0, 1, 1);

        btn_X_JogReverse = new QPushButton(widget);
        btn_X_JogReverse->setObjectName(QString::fromUtf8("btn_X_JogReverse"));
        sizePolicy2.setHeightForWidth(btn_X_JogReverse->sizePolicy().hasHeightForWidth());
        btn_X_JogReverse->setSizePolicy(sizePolicy2);
        btn_X_JogReverse->setMinimumSize(QSize(0, 40));

        gridLayout_4->addWidget(btn_X_JogReverse, 1, 1, 1, 1);

        btn_X_JogForward = new QPushButton(widget);
        btn_X_JogForward->setObjectName(QString::fromUtf8("btn_X_JogForward"));
        sizePolicy2.setHeightForWidth(btn_X_JogForward->sizePolicy().hasHeightForWidth());
        btn_X_JogForward->setSizePolicy(sizePolicy2);
        btn_X_JogForward->setMinimumSize(QSize(0, 40));

        gridLayout_4->addWidget(btn_X_JogForward, 1, 0, 1, 1);


        gridLayout_2->addWidget(widget, 8, 0, 1, 2);

        label_13 = new QLabel(RailWidget);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        gridLayout_2->addWidget(label_13, 7, 0, 1, 1);

        label_17 = new QLabel(RailWidget);
        label_17->setObjectName(QString::fromUtf8("label_17"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(label_17->sizePolicy().hasHeightForWidth());
        label_17->setSizePolicy(sizePolicy3);
        label_17->setMaximumSize(QSize(50, 16777215));

        gridLayout_2->addWidget(label_17, 0, 0, 1, 1);

        label_16 = new QLabel(RailWidget);
        label_16->setObjectName(QString::fromUtf8("label_16"));
        sizePolicy1.setHeightForWidth(label_16->sizePolicy().hasHeightForWidth());
        label_16->setSizePolicy(sizePolicy1);

        gridLayout_2->addWidget(label_16, 1, 0, 1, 1);

        edit_X_AbsSpeed = new QLineEdit(RailWidget);
        edit_X_AbsSpeed->setObjectName(QString::fromUtf8("edit_X_AbsSpeed"));
        sizePolicy3.setHeightForWidth(edit_X_AbsSpeed->sizePolicy().hasHeightForWidth());
        edit_X_AbsSpeed->setSizePolicy(sizePolicy3);
        edit_X_AbsSpeed->setMaximumSize(QSize(16777215, 50));

        gridLayout_2->addWidget(edit_X_AbsSpeed, 4, 1, 1, 2);

        horizontalSlider_X_AbsSpeed = new QSlider(RailWidget);
        horizontalSlider_X_AbsSpeed->setObjectName(QString::fromUtf8("horizontalSlider_X_AbsSpeed"));
        horizontalSlider_X_AbsSpeed->setMinimum(1);
        horizontalSlider_X_AbsSpeed->setMaximum(180);
        horizontalSlider_X_AbsSpeed->setValue(25);
        horizontalSlider_X_AbsSpeed->setOrientation(Qt::Horizontal);
        horizontalSlider_X_AbsSpeed->setInvertedControls(false);

        gridLayout_2->addWidget(horizontalSlider_X_AbsSpeed, 5, 1, 1, 1);

        label_14 = new QLabel(RailWidget);
        label_14->setObjectName(QString::fromUtf8("label_14"));

        gridLayout_2->addWidget(label_14, 6, 0, 1, 1);

        edit_X_AbsPosition = new QLineEdit(RailWidget);
        edit_X_AbsPosition->setObjectName(QString::fromUtf8("edit_X_AbsPosition"));
        sizePolicy3.setHeightForWidth(edit_X_AbsPosition->sizePolicy().hasHeightForWidth());
        edit_X_AbsPosition->setSizePolicy(sizePolicy3);
        edit_X_AbsPosition->setMaximumSize(QSize(16777215, 50));

        gridLayout_2->addWidget(edit_X_AbsPosition, 2, 1, 1, 2);

        label_X_CurrentSpeed = new QLabel(RailWidget);
        label_X_CurrentSpeed->setObjectName(QString::fromUtf8("label_X_CurrentSpeed"));
        sizePolicy1.setHeightForWidth(label_X_CurrentSpeed->sizePolicy().hasHeightForWidth());
        label_X_CurrentSpeed->setSizePolicy(sizePolicy1);
        label_X_CurrentSpeed->setMinimumSize(QSize(0, 0));
        label_X_CurrentSpeed->setMaximumSize(QSize(16777215, 50));

        gridLayout_2->addWidget(label_X_CurrentSpeed, 7, 1, 1, 2);

        edit_AxisState = new QTextEdit(RailWidget);
        edit_AxisState->setObjectName(QString::fromUtf8("edit_AxisState"));
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Minimum);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(1);
        sizePolicy4.setHeightForWidth(edit_AxisState->sizePolicy().hasHeightForWidth());
        edit_AxisState->setSizePolicy(sizePolicy4);
        edit_AxisState->setMaximumSize(QSize(16777215, 50));
        edit_AxisState->setFont(font1);
        edit_AxisState->setAutoFillBackground(true);
        edit_AxisState->setReadOnly(true);

        gridLayout_2->addWidget(edit_AxisState, 0, 1, 1, 2);

        horizontalSlider_X_AbsPosition = new QSlider(RailWidget);
        horizontalSlider_X_AbsPosition->setObjectName(QString::fromUtf8("horizontalSlider_X_AbsPosition"));
        horizontalSlider_X_AbsPosition->setMaximum(1000);
        horizontalSlider_X_AbsPosition->setSingleStep(1);
        horizontalSlider_X_AbsPosition->setValue(5);
        horizontalSlider_X_AbsPosition->setOrientation(Qt::Horizontal);
        horizontalSlider_X_AbsPosition->setInvertedAppearance(false);
        horizontalSlider_X_AbsPosition->setInvertedControls(false);

        gridLayout_2->addWidget(horizontalSlider_X_AbsPosition, 3, 1, 1, 1);


        gridLayout->addLayout(gridLayout_2, 0, 1, 1, 1);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        widget_2 = new QWidget(RailWidget);
        widget_2->setObjectName(QString::fromUtf8("widget_2"));
        widget_2->setMinimumSize(QSize(0, 10));
        gridLayout_5 = new QGridLayout(widget_2);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        btn_chk_Rest = new QPushButton(widget_2);
        btn_chk_Rest->setObjectName(QString::fromUtf8("btn_chk_Rest"));
        btn_chk_Rest->setMinimumSize(QSize(0, 40));

        gridLayout_5->addWidget(btn_chk_Rest, 1, 0, 1, 2);

        btn_contectRail = new QPushButton(widget_2);
        btn_contectRail->setObjectName(QString::fromUtf8("btn_contectRail"));

        gridLayout_5->addWidget(btn_contectRail, 0, 0, 1, 1);

        btn_discontectRail = new QPushButton(widget_2);
        btn_discontectRail->setObjectName(QString::fromUtf8("btn_discontectRail"));

        gridLayout_5->addWidget(btn_discontectRail, 0, 1, 1, 1);


        verticalLayout_2->addWidget(widget_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        chk_ServoEnable = new QCheckBox(RailWidget);
        chk_ServoEnable->setObjectName(QString::fromUtf8("chk_ServoEnable"));
        chk_ServoEnable->setEnabled(false);
        chk_ServoEnable->setLayoutDirection(Qt::LeftToRight);

        horizontalLayout->addWidget(chk_ServoEnable);

        chk_Stop = new QCheckBox(RailWidget);
        chk_Stop->setObjectName(QString::fromUtf8("chk_Stop"));

        horizontalLayout->addWidget(chk_Stop);

        chk_ImmediateStop = new QCheckBox(RailWidget);
        chk_ImmediateStop->setObjectName(QString::fromUtf8("chk_ImmediateStop"));

        horizontalLayout->addWidget(chk_ImmediateStop);


        verticalLayout_2->addLayout(horizontalLayout);

        textEdit = new QTextEdit(RailWidget);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));
        QSizePolicy sizePolicy5(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(textEdit->sizePolicy().hasHeightForWidth());
        textEdit->setSizePolicy(sizePolicy5);

        verticalLayout_2->addWidget(textEdit);


        gridLayout->addLayout(verticalLayout_2, 0, 0, 1, 1);


        retranslateUi(RailWidget);

        QMetaObject::connectSlotsByName(RailWidget);
    } // setupUi

    void retranslateUi(QWidget *RailWidget)
    {
        RailWidget->setWindowTitle(QCoreApplication::translate("RailWidget", "RailWidget", nullptr));
        edit_MotionState->setHtml(QCoreApplication::translate("RailWidget", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'Microsoft YaHei UI'; font-size:10pt; font-weight:700; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">\346\226\255\345\274\200\350\277\236\346\216\245</p></body></html>", nullptr));
        label_X_CurrentPosition->setText(QString());
        label_7->setText(QCoreApplication::translate("RailWidget", "\350\277\220\345\212\250\351\200\237\345\272\246", nullptr));
        label_6->setText(QCoreApplication::translate("RailWidget", "\350\277\220\345\212\250\344\275\215\347\275\256", nullptr));
        btn_regressOrigin->setText(QCoreApplication::translate("RailWidget", "\345\216\237\347\202\271\345\233\236\345\275\222", nullptr));
        btn_X_AbsPositionCommand->setText(QCoreApplication::translate("RailWidget", "\347\273\235\345\257\271\345\256\232\344\275\215\346\211\247\350\241\214", nullptr));
        btn_X_JogReverse->setText(QCoreApplication::translate("RailWidget", "\345\217\215\345\220\221\347\202\271\345\212\250", nullptr));
        btn_X_JogForward->setText(QCoreApplication::translate("RailWidget", "\346\255\243\345\220\221\347\202\271\345\212\250", nullptr));
        label_13->setText(QCoreApplication::translate("RailWidget", "\345\275\223\345\211\215\351\200\237\345\272\246", nullptr));
        label_17->setText(QCoreApplication::translate("RailWidget", "\350\275\264\347\212\266\346\200\201", nullptr));
        label_16->setText(QCoreApplication::translate("RailWidget", "\350\277\220\345\212\250\347\212\266\346\200\201", nullptr));
        edit_X_AbsSpeed->setText(QCoreApplication::translate("RailWidget", "30", nullptr));
        label_14->setText(QCoreApplication::translate("RailWidget", "\345\275\223\345\211\215\344\275\215\347\275\256", nullptr));
        edit_X_AbsPosition->setText(QCoreApplication::translate("RailWidget", "5", nullptr));
        label_X_CurrentSpeed->setText(QString());
        edit_AxisState->setHtml(QCoreApplication::translate("RailWidget", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'Microsoft YaHei UI'; font-size:10pt; font-weight:700; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">\346\226\255\345\274\200\350\277\236\346\216\245</p></body></html>", nullptr));
        btn_chk_Rest->setText(QCoreApplication::translate("RailWidget", "\345\244\215\344\275\215", nullptr));
        btn_contectRail->setText(QCoreApplication::translate("RailWidget", "\350\277\236\346\216\245\345\234\260\350\275\250", nullptr));
        btn_discontectRail->setText(QCoreApplication::translate("RailWidget", "\346\226\255\345\274\200\345\234\260\350\275\250", nullptr));
        chk_ServoEnable->setText(QCoreApplication::translate("RailWidget", "\344\274\272\346\234\215\344\275\277\350\203\275", nullptr));
        chk_Stop->setText(QCoreApplication::translate("RailWidget", "\345\201\234\346\255\242\350\277\220\345\212\250", nullptr));
        chk_ImmediateStop->setText(QCoreApplication::translate("RailWidget", "\346\200\245\345\201\234", nullptr));
    } // retranslateUi

};

namespace Ui {
    class RailWidget: public Ui_RailWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RAIL_WIDGET_H
