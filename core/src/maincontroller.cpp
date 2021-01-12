/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Nov 2020
 *
 ****************************************************************************/

#include "maincontroller.h"
#include "ocrhandler.h"
#include "textpage.h"
#include "hwhandler.h"
#include "cerence/cerencetts.h"
#include <opencv2/imgcodecs.hpp>

#include <QDebug>
#include <QTextToSpeech>
#include <QPluginLoader>

MainController::MainController()
{
    connect(&ocr(), &OcrHandler::lineAdded, this, [this](){
        emit textUpdated(ocr().textPage()->text());
        emit formattedTextUpdated(ocr().textPage()->formattedText());
    });

    connect(&ocr(), &OcrHandler::lineAdded, this, &MainController::onNewTextExtracted);

    m_ttsEngine = new CerenceTTS(this);
    connect(m_ttsEngine, &CerenceTTS::wordNotify, this, &MainController::wordNotify);
    connect(m_ttsEngine, &CerenceTTS::wordNotify, this, [this](int wordPosition, int wordLength){
        m_wordPosition = wordPosition;
        m_wordLength = wordLength;
    });
    connect(m_ttsEngine, &CerenceTTS::sayFinished, this, &MainController::onSpeakingFinished);

    m_hwhandler = new HWHandler(this);
//    connect(m_hwhandler, &HWHandler::imageReceived, this, [](const cv::Mat &image){
//    }, Qt::QueuedConnection);
//    connect(m_hwhandler, &HWHandler::buttonReceived, this, [](Button button){
//    }, Qt::QueuedConnection);

    m_hwhandler->start();
}

void MainController::start(const QString &filename)
{
    m_positionInParagraph = 0;
    m_currentParagraphNum = -1;
    cv::Mat image = cv::imread(filename.toStdString(), cv::IMREAD_GRAYSCALE);
    ocr().startProcess(image);
}

OcrHandler &MainController::ocr()
{
    return OcrHandler::instance();
}

void MainController::startSpeaking()
{
    m_wordPosition = 0;
    m_wordLength = 0;
    while (true) {
        qDebug() << __func__ << m_currentParagraphNum;
        m_currentText = ocr().textPage()->getText(m_currentParagraphNum, m_positionInParagraph);
        if (!m_currentText.isEmpty()) {
            qDebug() << __func__ << m_currentText;
            m_ttsEngine->say(m_currentText);
        } else if (ocr().textPage()->paragraph(m_currentParagraphNum).isComplete() &&
                   ++m_currentParagraphNum < ocr().textPage()->numParagraphs()) {
           m_positionInParagraph = 0;
           continue;
        }

        break;
    }
}

void MainController::onNewTextExtracted()
{
    if (m_currentParagraphNum < 0) {
        // Initialize current paragraph on the first text added
        m_currentParagraphNum = 0;
        startSpeaking();
    } else if (m_ttsEngine->isStoppedSpeaking()) {
        startSpeaking();
    }
}

void MainController::onSpeakingFinished()
{
    m_positionInParagraph += m_currentText.size();
    qDebug() << __func__ << m_positionInParagraph;
    startSpeaking();
}
