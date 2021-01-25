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

using namespace cv;

#define SHUTER_SOUND_WAVE_FILE "/opt/zyrlo/Distrib/Data/camera-shutter-click-01.wav"
#define BEEP_SOUND_WAVE_FILE "/opt/zyrlo/Distrib/Data/beep-08b.wav"

MainController::MainController()
{
    connect(&ocr(), &OcrHandler::lineAdded, this, [this](){
        emit textUpdated(ocr().textPage()->text());
        emit formattedTextUpdated(ocr().textPage()->formattedText());
    });

    connect(&ocr(), &OcrHandler::lineAdded, this, &MainController::onNewTextExtracted);

    m_ttsEngine = new CerenceTTS(this);
    connect(m_ttsEngine, &CerenceTTS::wordNotify, this, &MainController::setCurrentWord);
    connect(m_ttsEngine, &CerenceTTS::sayFinished, this, &MainController::onSpeakingFinished);

    m_hwhandler = new HWHandler(this);
    connect(m_hwhandler, &HWHandler::imageReceived, this, [this](const Mat &image){
        qDebug() << "imageReceived 0\n";
        ocr().startProcess(image);
        qDebug() << "imageReceived 2\n";
        if(m_shutterSound)
            m_shutterSound->play();

    }, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::buttonReceived, this, [](Button button){
        qDebug() << "received" << (int)button;
    }, Qt::QueuedConnection);

    connect(m_hwhandler, &HWHandler::previewImgUpdate, this, &MainController::previewImgUpdate, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::readerReady, this, &MainController::readerReady, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::targetNotFound, this, &MainController::targetNotFound, Qt::QueuedConnection);

    m_translator.Init();
    m_shutterSound = new QSound(SHUTER_SOUND_WAVE_FILE, this);
    m_hwhandler->start();
}

void MainController::start(const QString &filename)
{
    m_ttsEngine->stop();

    m_ttsStartPositionInParagraph = 0;
    m_currentParagraphNum = 0;
    cv::Mat image = cv::imread(filename.toStdString(), cv::IMREAD_GRAYSCALE);
    ocr().startProcess(image);
}

void MainController::pauseResume()
{
    if (!m_ttsEngine->pauseResume()) {
        startSpeaking();
    }
}

void MainController::backWord()
{
    auto position = paragraph().prevWordPosition(m_currentWordPosition.parPos());
    while (!position.isValid()) {
        if (--m_currentParagraphNum >= 0) {
            // Go the the previous paragraph
            position = paragraph().lastWordPosition();
        } else {
            // Go to the beginning of the page
            m_currentParagraphNum = 0;
            position = paragraph().firstWordPosition();
        }
    }

    setCurrentWordPosition(position);
    m_ttsStartPositionInParagraph = position.parPos();
    startSpeaking();
}

void MainController::nextWord()
{
    auto position = paragraph().nextWordPosition(m_currentWordPosition.parPos());
    if (!position.isValid()) {
        if (m_currentParagraphNum + 1 <= ocr().processingParagraphNum()) {
            // Go the the next paragraph
            ++m_currentParagraphNum;
            position = paragraph().firstWordPosition();
        } else {
            // Page finished
            m_ttsEngine->stop();
            return;
        }
    }

    setCurrentWordPosition(position);
    m_ttsStartPositionInParagraph = position.parPos();
    startSpeaking();
}

void MainController::backSentence()
{
    auto position = paragraph().prevSentencePosition(m_currentWordPosition.parPos());
    while (!position.isValid()) {
        if (--m_currentParagraphNum >= 0) {
            // Go the the previous paragraph
            position = paragraph().lastSentencePosition();
        } else {
            // Go to the beginning of the page
            m_currentParagraphNum = 0;
            position = paragraph().firstSentencePosition();
        }
    }

    setCurrentWordPosition(position);
    m_ttsStartPositionInParagraph = position.parPos();
    startSpeaking();
}

void MainController::nextSentence()
{
    auto position = paragraph().nextSentencePosition(m_currentWordPosition.parPos());
    if (!position.isValid()) {
        if (m_currentParagraphNum + 1 <= ocr().processingParagraphNum()) {
            // Go the the next paragraph
            ++m_currentParagraphNum;
            position = paragraph().firstSentencePosition();
        } else {
            // Page finished
            m_ttsEngine->stop();
            return;
        }
    }

    setCurrentWordPosition(position);
    m_ttsStartPositionInParagraph = position.parPos();
    startSpeaking();
}

OcrHandler &MainController::ocr()
{
    return OcrHandler::instance();
}

void MainController::snapImage() {
    m_hwhandler->snapImage();
}

void MainController::flashLed() {
    m_hwhandler->flashLed(1000);
}

void MainController::setLed(bool bOn) {
     m_hwhandler->setLed(bOn);
}

static int getCurrentPulseSinkIndex() {
    FILE *fp;
    char path[1035];
    int indx = -1;
    fp = popen("pacmd info | grep \"* index:\"", "r");
    if (fp == NULL) {
        qDebug() << "Failed to run command\n";
        exit(1);
    }

    /* Read the output a line at a time - output it. */
    while (fgets(path, sizeof(path), fp) != NULL) {
        qDebug() << path;
        if(strstr(path, "index: 1")) {
            indx = 1;
            break;
        }
        if(strstr(path, "index: 0")) {
            indx = 0;
            break;
        }
     }

    /* close */
    pclose(fp);
    return indx;
}

bool MainController::toggleAudioSink() {
    int indx = getCurrentPulseSinkIndex();
    if(indx < 0)
        return false;
    bool bret = true;
    FILE *fp;
    char path[1035], cmd[256];
    sprintf(cmd, "pacmd set-default-sink %d", 1 - indx);
    fp = popen(cmd, "r");
    if (fp == NULL) {
        qDebug() << "Failed to run command\n";
        bret = false;
    }
    else {
        /* Read the output a line at a time - output it. */
        while (fgets(path, sizeof(path), fp) != NULL) {
            bret = false;
            qDebug() << path;
        }
    }
    /* close */
    pclose(fp);
    if(m_shutterSound) {
        delete m_shutterSound;
        m_shutterSound = new QSound(SHUTER_SOUND_WAVE_FILE, this);
     }
    if(m_beepSound) {
        delete m_beepSound;
        m_beepSound = new QSound(BEEP_SOUND_WAVE_FILE, this);
     }
    if(bret && m_ttsEngine) {
        m_ttsEngine->resetAudio();
    }
    return bret;
}

const OcrHandler &MainController::ocr() const
{
    return OcrHandler::instance();
}

void MainController::startSpeaking()
{
    while (true) {
        qDebug() << __func__ << "current paragraph" << m_currentParagraphNum;
        if(m_currentParagraphNum < 0)
            break;
        setCurrentWord(0, 0);
        const TextPage *pTp = ocr().textPage();
        if(!pTp)
            break;
        m_currentText = ocr().textPage()->getText(m_currentParagraphNum, m_ttsStartPositionInParagraph);

        if (!m_currentText.isEmpty()) {
            // Continue speaking if there is more text in the current paragraph
            qDebug() << __func__ << m_currentText;
            m_ttsEngine->say(m_currentText);
        } else if (m_currentParagraphNum + 1 <= ocr().processingParagraphNum()) {
            // Advance to the next paragraph if the current one is completed and
            // all text pronounced
            ++m_currentParagraphNum;
            m_ttsStartPositionInParagraph = 0;
            continue;
        }

        break;
    }
}

const Paragraph &MainController::paragraph() const
{
    return ocr().textPage()->paragraph(m_currentParagraphNum);
}

void MainController::setCurrentWordPosition(const TextPosition &textPosition)
{
    m_currentWordPosition = textPosition;
    emit wordPositionChanged(m_currentWordPosition);
}

void MainController::onNewTextExtracted()
{
    if (m_ttsEngine->isStoppedSpeaking()) {
        qDebug() << __func__ << m_currentParagraphNum;
        // If TTS stopped and there is more text extracted, then continue speaking
        startSpeaking();
    }
}

void MainController::onSpeakingFinished()
{
    m_ttsStartPositionInParagraph = m_currentWordPosition.parPos() + m_currentWordPosition.length();
    qDebug() << __func__ << m_ttsStartPositionInParagraph;
    startSpeaking();
}

void MainController::setCurrentWord(int wordPosition, int wordLength)
{
    TextPosition wordPos{m_ttsStartPositionInParagraph + wordPosition,
                         wordLength,
                         paragraph().paragraphPosition()};

    setCurrentWordPosition(wordPos);
}

void MainController::previewImgUpdate(const Mat & prevImg) {
    emit previewUpdated(prevImg);
}

void MainController::readerReady() {
    if(m_ttsEngine)
        m_ttsEngine->say(m_translator.GetString("PLACE_DOC").c_str());
}

void MainController::targetNotFound() {
    if(m_ttsEngine)
        m_ttsEngine->say(m_translator.GetString("CLEAR_SURF").c_str());
}
