#include <QVBoxLayout>
#include "test_cad_view.h"

// QCAD核心头文件
#include <RDocument.h>
#include <RMemoryStorage.h>
#include <RSpatialIndexNavel.h>
#include <RDocumentInterface.h>
#include <RGraphicsViewQt.h>
#include <RDxfImporter.h>

TestCADView::TestCADView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TestCADView)
{
    ui->setupUi(this);

    // 1. 创建QCAD文档对象
    document = new RDocument(new RMemoryStorage(), new RSpatialIndexNavel());

    // 2. 创建文档接口
    documentInterface = new RDocumentInterface(*document);

    // 3. 导入一个DXF文件
    RDxfImporter importer(*document, "test.dxf");
    importer.importFile();

    // 4. 创建视图窗口
    cadView = new RGraphicsViewQt(documentInterface, ui->cadWidget);
    cadView->setMinimumSize(400, 300); // 你可以自定义尺寸

    // 5. 替换（或布局）到ui界面中
    auto layout = new QVBoxLayout(ui->cadWidget);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(cadView);
}

TestCADView::~TestCADView()
{
    delete ui;
}
