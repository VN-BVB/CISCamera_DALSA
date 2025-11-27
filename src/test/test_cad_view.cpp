
#include "test_cad_view.h"
#include "ui_test_cad_view.h"

#include <QVBoxLayout>
// #include <RDocument.h>
// #include <RMemoryStorage.h>
// #include <RSpatialIndexNavel.h>
// #include <RDocumentInterface.h>
// #include <RGraphicsViewQt.h>
// #include <RDxfImporter.h>

TestCADView::TestCADView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TestCADView)
{
    ui->setupUi(this);

    // cadView = new RGraphicsViewQt(this);
    // cadView->setMinimumSize(400, 300);

    // auto layout = new QVBoxLayout(this);
    // layout->setContentsMargins(0,0,0,0);
    // layout->addWidget(cadView);
}

TestCADView::~TestCADView()
{
    delete ui;
}
