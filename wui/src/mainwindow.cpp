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
#include <QTextBlock>
#include <QInputDialog>
#include <regex>

using namespace cv;
using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_pLabelPreview = new QLabel();
    ui->gridLayout->addWidget(m_pLabelPreview, 1, 0, Qt::AlignCenter);
    m_pLabelPreview->setVisible(false);
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::start);
    connect(&m_controller, &MainController::textUpdated, this, &MainWindow::updateText);
    connect(&m_controller, &MainController::wordPositionChanged, this, &MainWindow::highlighWord);
    connect(&m_controller, &MainController::previewUpdated, this, &MainWindow::updatePreview);

    connect(ui->pauseButton, &QPushButton::clicked, &m_controller, &MainController::pauseResume);
    connect(ui->nextWordButton, &QPushButton::clicked, &m_controller, &MainController::nextWord);
    connect(ui->backWordButton, &QPushButton::clicked, &m_controller, &MainController::backWord);
    connect(ui->nextSentenceButton, &QPushButton::clicked, &m_controller, &MainController::nextSentence);
    connect(ui->backSentenceButton, &QPushButton::clicked, &m_controller, &MainController::backSentence);
    connect(ui->spellWordButton, &QPushButton::clicked, &m_controller, &MainController::spellCurrentWord);
    connect(ui->rateUpButton, &QPushButton::clicked, &m_controller, &MainController::speechRateUp);
    connect(ui->rateDownButton, &QPushButton::clicked, &m_controller, &MainController::speechRateDown);
    connect(ui->nextVoiceButton, &QPushButton::clicked, &m_controller, &MainController::nextVoice);

    ui->fileNameLineEdit->setText("/opt/zyrlo/RawFull_000.bmp");
    m_controller.setLed(true);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_pLabelPreview;
    m_controller.setLed(false);
}

void MainWindow::keyPressEvent(QKeyEvent *ev) {
    switch(ev->key()) {
    case Qt::Key_S:
        if(ev->modifiers() & Qt::CTRL) {
            m_bPreviewOn = true;
            m_pLabelPreview->setVisible(true);
            qDebug() << "Preview ON\n";
        }
        else {
            m_bSavePreviewImage = true;
            qDebug() << "Saving preview image\n";
        }
        break;
    case Qt::Key_H:
        m_bPreviewOn = false;
        m_pLabelPreview->setVisible(false);
        qDebug() << "Preview OFF\n";
        break;
    case Qt::Key_F:
        m_controller.flashLed();
        qDebug() << "Flash Led\n";
        break;
    case Qt::Key_T:
        m_controller.snapImage();
        qDebug() << "SnapShot\n";
        break;
    case Qt::Key_L:
        if(ev->modifiers() & Qt::SHIFT) {
            m_controller.setLed(false);
            qDebug() << "Light OFF\n";
        }
        else {
            m_controller.setLed(true);
            qDebug() << "Light ON\n";
        }
        break;
    case Qt::Key_P:
        m_controller.toggleAudioSink();
        break;
    case Qt::Key_F1:
        if(ev->modifiers() & Qt::SHIFT)
            m_controller.SaveImage(1);
        else
            m_controller.ReadImage(1);
        break;
    case Qt::Key_F2:
        if(ev->modifiers() & Qt::SHIFT)
            m_controller.SaveImage(2);
        else
            m_controller.ReadImage(2);
        break;
    case Qt::Key_F3:
        if(ev->modifiers() & Qt::SHIFT)
            m_controller.SaveImage(3);
        else
            m_controller.ReadImage(3);
        break;
    case Qt::Key_F4:
        if(ev->modifiers() & Qt::SHIFT)
            m_controller.SaveImage(4);
        else
            m_controller.ReadImage(4);
        break;
    case Qt::Key_F5:
    {
        m_controller.sayText("Type the serial number");
        bool ok;
        QString text = QInputDialog::getText(this, tr("Keypad MAC"), tr(""), QLineEdit::Normal, "", &ok);
        if (ok && !text.isEmpty())
            m_controller.write_keypad_config(text.toStdString());
    }
        break;
    case Qt::Key_F6:
        m_controller.SaySN();
        break;
    }
}

void MainWindow::start()
{
    m_prevPosition.clear();
    m_controller.startFile(ui->fileNameLineEdit->text());
}

void MainWindow::updateText(QString text)
{
    const auto currentText = ui->textBrowser->toPlainText();
    if (text.left(currentText.size()) == currentText) {
        // Remove the currentText from the text
        text.remove(0, currentText.size());
    } else {
        // text is not according to the current text, will clear and replace it
        ui->textBrowser->clear();
    }

    QTextCharFormat fmt;
    fmt.setFontPointSize(14);

    QTextCursor cursor(ui->textBrowser->document());
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(text, fmt);
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

void MainWindow::updatePreview(const Mat &img) {
    if(m_bPreviewOn) {
        cvtColor(img, m_prevImg, CV_GRAY2RGB);
        rotate(m_prevImg, m_prevImg, ROTATE_180);
        m_pLabelPreview->setPixmap(QPixmap::fromImage(QImage(m_prevImg.data, m_prevImg.cols, m_prevImg.rows, m_prevImg.step, QImage::Format_RGB888)));
    }
    if(m_bSavePreviewImage) {
        m_bSavePreviewImage = false;
        imwrite("PreviewImage.bmp", img);
    }
}
