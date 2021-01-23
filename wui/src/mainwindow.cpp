/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDebug>
#include <QTextBlock>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::start);
    connect(&m_controller, &MainController::formattedTextUpdated,
            ui->textBrowser, &QTextBrowser::setHtml);
    connect(&m_controller, &MainController::wordPositionChanged,
            this, &MainWindow::highlighWord);

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

void MainWindow::highlighWord(const TextPosition &position)
{
    QTextCursor cursor(ui->textBrowser->document());

    if (m_prevPosition.isValid()) {
        // Restore previous position's format
        setCursorAtPosition(m_prevPosition, cursor);
        cursor.setCharFormat(m_prevFormat);
    }

    setCursorAtPosition(position, cursor);
    QTextCharFormat fmt = cursor.charFormat();

    // Save current format and position
    m_prevFormat = fmt;
    m_prevPosition = position;

    // Set new background on the new position
    fmt.setBackground(Qt::yellow);
    cursor.setCharFormat(fmt);

    // Make sure that the current highlighted word is always visible (using scrolling when needed)
    cursor.setPosition(position.absPos());
    ui->textBrowser->setTextCursor(cursor);
}

void MainWindow::setCursorAtPosition(const TextPosition &position, QTextCursor &cursor)
{
    cursor.setPosition(position.absPos(), QTextCursor::MoveAnchor);
    cursor.setPosition(position.absPos() + position.length(), QTextCursor::KeepAnchor);
}
