#include "imageviewwindow.h"
#include "ui_imageviewwindow.h"

#include <QFileDialog>
#include <QDebug>

ImageViewWindow::ImageViewWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ImageViewWindow)
{
    ui->setupUi(this);
}

ImageViewWindow::~ImageViewWindow()
{
    delete ui;
}

void ImageViewWindow::on_pb_open_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Select Image", "", "(*.png *.jpg *.bmp)");
    if(path.isEmpty())
        return;
    qDebug() << "地址" << path;
    try
    {
        cv::Mat img = cv::imread(path.toStdString(), cv::IMREAD_GRAYSCALE);
        if (img.empty())
        {
            qDebug() << "无法加载图像";
            return;
        }
        qDebug() << "图像尺寸:" << img.cols << "x" << img.rows;

        cv::Mat cropped_img = img(cv::Rect(16000, 16000, 1900, 1900));
        cv::imwrite("D:/Cpp_Project/WeldseamMeasurement/tests/image/cropped_img.bmp", cropped_img);

        cv::Mat edge;
        cv::Canny(cropped_img, edge, 20, 40);


        // 将OpenCV的Mat转换为QImage
        QImage qimg;
        if (edge.type() == CV_8UC1)
        {
            // 灰度图像
            qimg = QImage(edge.data, edge.cols, edge.rows, edge.step, QImage::Format_Grayscale8);
        }
        else
        {
            // 如果是彩色图像，需要转换BGR到RGB
            cv::Mat img_rgb;
            cv::cvtColor(edge, img_rgb, cv::COLOR_BGR2RGB);
            qimg = QImage(img_rgb.data, img_rgb.cols, img_rgb.rows, img_rgb.step, QImage::Format_RGB888);
        }

        // 创建QGraphicsScene
        QGraphicsScene *scene = new QGraphicsScene(this);

        // 将QImage转换为QPixmap并添加到场景中
        QPixmap pixmap = QPixmap::fromImage(qimg);
        scene->addPixmap(pixmap);

        // 将场景设置到QGraphicsView中
        ui->gv_image->setScene(scene);

        // 可选：调整视图以适应图像大小
        ui->gv_image->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    }
    catch (const std::exception& e)
    {
        qDebug() << "加载图像时出错:" << e.what();
    }

}
