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

    connect(ui->pauseButton, &QPushButton::clicked, &m_controller, &MainController::pauseResume);
    connect(ui->nextWordButton, &QPushButton::clicked, &m_controller, &MainController::nextWord);
    connect(ui->backWordButton, &QPushButton::clicked, &m_controller, &MainController::backWord);
    connect(ui->nextSentenceButton, &QPushButton::clicked, &m_controller, &MainController::nextSentence);
    connect(ui->backSentenceButton, &QPushButton::clicked, &m_controller, &MainController::backSentence);

    ui->fileNameLineEdit->setText("/opt/zyrlo/RawFull_000.bmp");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::start()
{
    m_controller.start(ui->fileNameLineEdit->text());
}
