/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
      , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::start);
    connect(&m_controller, &MainController::formattedTextUpdated,
            ui->textBrowser, &QTextBrowser::setHtml);

    ui->fileNameLineEdit->setText("/home/dilshodm/work/proj/upwork/leon/Zyrlo/tests/data/RawFull_000.jpg");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::start()
{
    m_controller.start(ui->fileNameLineEdit->text());
}
