#include "CISCameraImage.h"

#include "ui_CISCameraImage.h"

CISWidget::CISWidget(QWidget *parent) : QWidget(parent), ui(new Ui::CISWidget) { ui->setupUi(this); }

CISWidget::~CISWidget() { delete ui; }
