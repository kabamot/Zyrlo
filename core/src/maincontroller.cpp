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
#include <QtConcurrent>
#include "BTComm.h"


using namespace cv;

#define SHUTER_SOUND_WAVE_FILE "/opt/zyrlo/Distrib/Data/camera-shutter-click-01.wav"
#define BEEP_SOUND_WAVE_FILE "/opt/zyrlo/Distrib/Data/beep-08b.wav"

constexpr int DELAY_ON_NAVIGATION = 1000; // ms, delay before starting TTS

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
        m_ttsEngine->stop();

        m_ttsStartPositionInParagraph = 0;
        m_currentParagraphNum = 0;

        ocr().startProcess(image);
        qDebug() << "imageReceived 2\n";
        if(m_shutterSound)
            m_shutterSound->play();
        startBeeping();

    }, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::buttonReceived, this, [](Button button){
        qDebug() << "received" << (int)button;
    }, Qt::QueuedConnection);

    connect(m_hwhandler, &HWHandler::previewImgUpdate, this, &MainController::previewImgUpdate, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::readerReady, this, &MainController::readerReady, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::targetNotFound, this, &MainController::targetNotFound, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::onBtButton, this, &MainController::onBtButton, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::onBtBattery, this, &MainController::onBtBattery, Qt::QueuedConnection);

    m_translator.Init();
    m_shutterSound = new QSound(SHUTER_SOUND_WAVE_FILE, this);
    m_beepSound = new QSound(BEEP_SOUND_WAVE_FILE, this);

    m_hwhandler->start();
}

void MainController::start(const QString &filename)
{
    m_ttsEngine->stop();

    m_ttsStartPositionInParagraph = 0;
    m_currentParagraphNum = 0;
    cv::Mat image = cv::imread(filename.toStdString(), cv::IMREAD_GRAYSCALE);
    ocr().startProcess(image);
    m_state = State::SpeakingPage;
}

void MainController::pauseResume()
{
    switch (m_state) {
    case State::Stopped:
        m_state = State::SpeakingPage;
        startSpeaking();
        break;

    case State::SpeakingPage:
        m_state = State::Paused;
        if (m_ttsEngine->isSpeaking()) {
            m_ttsEngine->pause();
        }
        break;

    case State::SpeakingText:
        // Don't react to pause/resume in this state
        break;

    case State::Paused:
        m_state = State::SpeakingPage;
        if (m_ttsEngine->isPaused()) {
            m_ttsEngine->resume();
        } else {
            startSpeaking();
        }
        break;
    }
}

void MainController::backWord()
{
    if (!isPageValid())
        return;

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

    if (m_state == State::SpeakingPage) {
        startSpeaking(DELAY_ON_NAVIGATION);
    } else {
        m_ttsEngine->stop();
    }
}

void MainController::nextWord()
{
    if (!isPageValid())
        return;

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

    if (m_state == State::SpeakingPage) {
        startSpeaking(DELAY_ON_NAVIGATION);
    } else {
        m_ttsEngine->stop();
    }
}

void MainController::backSentence()
{
    if (!isPageValid())
        return;

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

    if (m_state == State::SpeakingPage) {
        startSpeaking(DELAY_ON_NAVIGATION);
    } else {
        m_ttsEngine->stop();
    }
}

void MainController::nextSentence()
{
    if (!isPageValid())
        return;

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

    if (m_state == State::SpeakingPage) {
        startSpeaking(DELAY_ON_NAVIGATION);
    } else {
        m_ttsEngine->stop();
    }
}

void MainController::sayText(const QString &text)
{
    if (m_ttsEngine) {
        m_prevState = m_state;
        m_state = State::SpeakingText;
        m_ttsEngine->say(text);
    } else {
        qDebug() << "TTS engine is not created";
    }
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
        return false;
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
    if(!bret)
        return false;
    if(m_shutterSound) {
        delete m_shutterSound;
        m_shutterSound = new QSound(SHUTER_SOUND_WAVE_FILE, this);
     }
    if(m_beepSound) {
        delete m_beepSound;
        m_beepSound = new QSound(BEEP_SOUND_WAVE_FILE, this);
     }
    if(m_ttsEngine) {
        m_ttsEngine->resetAudio();
    }
    return bret;
}

void MainController::toggleAudioSinkVoid() {
      //toggleAudioSink();
      m_beepSound->play();
}

void MainController::readerReady() {
    stopBeeping();
    if(m_ttsEngine)
        m_ttsEngine->say(m_translator.GetString("PLACE_DOC").c_str());
    m_currentParagraphNum = -1;
    ocr().stopProcess();
}

void MainController::targetNotFound()
{
    sayText(m_translator.GetString("CLEAR_SURF").c_str());
}

const OcrHandler &MainController::ocr() const
{
    return OcrHandler::instance();
}

void MainController::startSpeaking(int delayMs)
{
    if (!isPageValid() || m_currentParagraphNum < 0)
        return;

    while (true) {
        qDebug() << __func__ << "current paragraph" << m_currentParagraphNum;
        m_currentText = ocr().textPage()->getText(m_currentParagraphNum, m_ttsStartPositionInParagraph);

        if (!m_currentText.isEmpty()) {
            // Continue speaking if there is more text in the current paragraph
            qDebug() << __func__ << m_currentText;
            m_ttsEngine->say(m_currentText, delayMs);
        } else if (m_currentParagraphNum + 1 <= ocr().processingParagraphNum()) {
            // Advance to the next paragraph if the current one is completed and
            // all text pronounced
            ++m_currentParagraphNum;
            m_ttsStartPositionInParagraph = 0;
            continue;
        } else {
            // Page finished
            m_state = State::Stopped;
            emit finished();
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

bool MainController::isPageValid() const
{
    return ocr().textPage() != nullptr;
}

void MainController::onNewTextExtracted()
{
    stopBeeping();
    if (m_state == State::SpeakingPage && m_ttsEngine->isStoppedSpeaking()) {
        qDebug() << __func__ << m_currentParagraphNum;
        // If TTS stopped and there is more text extracted, then continue speaking
        startSpeaking();
    }
}

void MainController::onSpeakingFinished()
{
    if (m_state == State::SpeakingText) {
        m_state = m_prevState;
    }

    if (m_state == State::SpeakingPage) {
        m_ttsStartPositionInParagraph = m_currentWordPosition.parPos() + m_currentWordPosition.length();
        qDebug() << __func__ << m_ttsStartPositionInParagraph;
        startSpeaking();
    }
}

void MainController::setCurrentWord(int wordPosition, int wordLength)
{
    if(m_state == State::SpeakingText || m_currentParagraphNum < 0)
        return;

    TextPosition wordPos{m_ttsStartPositionInParagraph + wordPosition,
                         wordLength,
                         paragraph().paragraphPosition()};

    setCurrentWordPosition(wordPos);
}

void MainController::previewImgUpdate(const Mat & prevImg) {
    emit previewUpdated(prevImg);
}

void MainController::startBeeping() {
     if(!m_beepSound)
        return;
    if (!m_beepingThread.isRunning())
        m_beepingThread = QtConcurrent::run([this]() {
           for(m_bKeepBeeping = true, QThread::msleep(1000); m_bKeepBeeping; QThread::msleep(1000)) {
               m_beepSound->play();
           }
        });
}

void MainController::startLongPressTimer(void (MainController::*action)(void), int nDelay) {
    const int intvl = 100;
    m_nLongPressCount = nDelay / intvl;
    if (!m_longPressTimerThread.isRunning())
        m_longPressTimerThread = QtConcurrent::run([this, action]() {
            for(; m_nLongPressCount >= 0; --m_nLongPressCount, QThread::msleep(intvl)) {
                if(m_nLongPressCount == 0) {
                    (this->*action)();
                    break;
                }
            }
        });
}

void MainController::stopLongPressTimer() {
    m_nLongPressCount = -1;
}

void MainController::stopBeeping() {
    qDebug() << __func__ << "beep\n";
    m_bKeepBeeping = false;
}

//#define KP_BUTTON_PRESSED     0x01
//#define KP_BUTTON_RELEASED    0x02
//#define KP_BATTERY_INFO       0x03
//#define KP_POWERING_DOWN      0x04


//#define KP_BUTTON_CENTER      0x01
//#define KP_BUTTON_UP          0x02
//#define KP_BUTTON_DOWN        0x03
//#define KP_BUTTON_LEFT        0x04
//#define KP_BUTTON_RIGHT       0x05
//#define KP_BUTTON_HELP        0x06
//#define KP_BUTTON_ROUND_L     0x07
//#define KP_BUTTON_ROUND_R     0x08
//#define KP_BUTTON_SQUARE_L    0x09
void MainController::onBtButton(int nButton, bool bDown) {
    if(bDown) {
        switch(nButton) {
        case KP_BUTTON_CENTER   :
            if(m_squareLeftDown) {
                //SaveImage(1);
                break;
            }
            if(m_squareRightDown) {
                //ReadImage(1);
                break;
            }
            pauseResume();
            break;
        case KP_BUTTON_UP       :
            m_buttonUpRessed = true;
            if(m_voiceDown) {
                m_ignoreVoice = true;
                startLongPressTimer(&MainController::toggleAudioSinkVoid, 3000);
                break;
            }
            if(m_squareLeftDown) {
                //SaveImage(2);
                break;
            }
            if(m_squareRightDown) {
                //ReadImage(2);
                break;
            }
            backSentence();
            break;
        case KP_BUTTON_DOWN     :
            if(m_squareLeftDown) {
                break;
            }
            nextSentence();
            break;
        case KP_BUTTON_LEFT     :
            if(m_squareLeftDown) {
                break;
            }
            backWord();
            break;
        case KP_BUTTON_RIGHT    :
            nextWord();
            break;
        case KP_BUTTON_HELP     :
            break;
        case KP_BUTTON_ROUND_L  :
            m_voiceDown = true;
            if(m_buttonUpRessed) {
                m_ignoreVoice = true;
                startLongPressTimer(&MainController::toggleAudioSinkVoid, 3000);
                break;
            }
            break;
        case KP_BUTTON_ROUND_R  :
            //Spell
            break;
        case KP_BUTTON_SQUARE_L :
            m_squareLeftDown = true;
            break;
        case KP_BUTTON_SQUARE_R :
            m_squareRightDown = true;
            break;
         }
    }
    else {
        stopLongPressTimer();
        switch(nButton) {
        case KP_BUTTON_UP       :
            m_buttonUpRessed = false;
            break;
        case KP_BUTTON_ROUND_L  :
            m_voiceDown = false;
            if(m_ignoreVoice) {
                m_ignoreVoice = false;
            }
            else
            //SwitchVoice();
            break;
        case KP_BUTTON_SQUARE_L :
            m_squareLeftDown = false;
            break;
        case KP_BUTTON_SQUARE_R :
            m_squareRightDown = false;
            break;
          }
    }
}

void MainController::onBtBattery(int nVal) {

}
