/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QtGui>

using namespace cv;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
      , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_pLabelPreview = new QLabel();
    ui->gridLayout->addWidget(m_pLabelPreview, 1, 0, Qt::AlignCenter);
    m_pLabelPreview->setVisible(false);
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::start);
    connect(&m_controller, &MainController::formattedTextUpdated,
            ui->textBrowser, &QTextBrowser::setHtml);
    connect(&m_controller, &MainController::previewUpdated, this, &MainWindow::updatePreview);

//    ui->fileNameLineEdit->setText("/home/dilshodm/work/proj/upwork/leon/Zyrlo/tests/data/RawFull_000.jpg");
    ui->fileNameLineEdit->setText("/home/pi/zyrlo/Images/RawFull_001.bmp");
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_pLabelPreview;
}

void MainWindow::start()
{
    m_controller.start(ui->fileNameLineEdit->text());
}

void MainWindow::updatePreview(const Mat &img) {
    cvtColor(img, m_prevImg, CV_GRAY2RGB);
    rotate(m_prevImg, m_prevImg, ROTATE_180);
    m_pLabelPreview->setPixmap(QPixmap::fromImage(QImage(m_prevImg.data, m_prevImg.cols, m_prevImg.rows, m_prevImg.step, QImage::Format_RGB888)));
    if(m_bSavePreviewImage) {
        m_bSavePreviewImage = false;
        imwrite("PreviewImage.bmp", img);
    }
}

void MainWindow::keyPressEvent(QKeyEvent *ev) {
    switch(ev->key()) {
    case Qt::Key_S:
        if(ev->modifiers() & Qt::CTRL) {
            m_pLabelPreview->setVisible(true);
            qDebug() << "Preview ON\n";
        }
        else {
            m_bSavePreviewImage = true;
            qDebug() << "Saving preview image\n";
        }
        break;
    case Qt::Key_H:
        m_pLabelPreview->setVisible(false);
        qDebug() << "Preview OFF\n";
        break;
    case Qt::Key_F:
        m_controller.flashLed();
        qDebug() << "Flash Led\n";
        break;
    case Qt::Key_T:
        m_controller.snapImage();
        qDebug() << "Flash Led\n";
        break;
     }
}
