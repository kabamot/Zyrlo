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
#include "BaseComm.h"


using namespace cv;

#define SHUTER_SOUND_WAVE_FILE "/opt/zyrlo/Distrib/Data/camera-shutter-click-01.wav"
#define BEEP_SOUND_WAVE_FILE "/opt/zyrlo/Distrib/Data/beep-08b.wav"
#define TRANSLATION_FILE "/opt/zyrlo/Distrib/Data/ZyrloTranslate.xml"
#define HELP_FILE "/opt/zyrlo/Distrib/Data/ZyrloHelp.xml"

constexpr int DELAY_ON_NAVIGATION = 1000; // ms, delay before starting TTS

MainController::MainController()
{
    connect(&ocr(), &OcrHandler::lineAdded, this, [this](){
        emit textUpdated(ocr().textPage()->text());
        emit formattedTextUpdated(ocr().textPage()->formattedText());
    });

    connect(&ocr(), &OcrHandler::lineAdded, this, &MainController::onNewTextExtracted);
    connect(this, &MainController::toggleAudioOutput, this, &MainController::onToggleAudioSink);
    connect(this, &MainController::spellCurrentWord, this, &MainController::onSpellCurrentWord);
    connect(this, &MainController::resetDevice, this, &MainController::onResetDevice);
    connect(this, &MainController::toggleGestures, this, &MainController::onToggleGestures);
    connect(this, &MainController::toggleVoice, this, &MainController::onToggleVoice);
    connect(this, &MainController::readHelp, this, &MainController::onReadHelp);

    m_ttsEngine = new CerenceTTS(this);
    connect(m_ttsEngine, &CerenceTTS::wordNotify, this, &MainController::setCurrentWord);
    connect(m_ttsEngine, &CerenceTTS::sayFinished, this, &MainController::onSpeakingFinished);

    m_hwhandler = new HWHandler(this);
    connect(m_hwhandler, &HWHandler::imageReceived, this, [this](const Mat &image){
        qDebug() << "imageReceived 0\n";
        m_ttsEngine->stop();

        m_ttsStartPositionInParagraph = 0;
        m_currentParagraphNum = 0;

        qDebug() << "imageReceived 2\n";
        if(m_shutterSound)
            m_shutterSound->play();
        startBeeping();
        ocr().startProcess(image);
        m_state = State::SpeakingPage;
    }, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::buttonReceived, this, [](Button button){
        qDebug() << "received" << (int)button;
    }, Qt::QueuedConnection);

    connect(m_hwhandler, &HWHandler::previewImgUpdate, this, &MainController::previewImgUpdate, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::readerReady, this, &MainController::readerReady, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::targetNotFound, this, &MainController::targetNotFound, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::onBtButton, this, &MainController::onBtButton, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::onButton, this, &MainController::onButton, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::onBtBattery, this, &MainController::onBtBattery, Qt::QueuedConnection);

    m_translator.Init(TRANSLATION_FILE);
    m_help.Init(HELP_FILE);
    m_shutterSound = new QSound(SHUTER_SOUND_WAVE_FILE, this);
    m_beepSound = new QSound(BEEP_SOUND_WAVE_FILE, this);

    m_hwhandler->start();
}

void MainController::start(const QString &filename)
{
    m_ttsEngine->stop();

    m_ttsStartPositionInParagraph = 0;
    m_currentParagraphNum = 0;
    m_currentWordPosition.clear();
    m_wordNavigationWithDelay = false;
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
        if (!m_ttsEngine->isSpeaking()) {
            m_state = State::SpeakingPage;
            startSpeaking();
        }
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

    bool isPageBoundary = false;
    auto position = paragraph().prevWordPosition(m_currentWordPosition.parPos());
    while (!position.isValid()) {
        if (--m_currentParagraphNum >= 0) {
            // Go the the previous paragraph
            position = paragraph().lastWordPosition();
        } else {
            // Go to the beginning of the page
            isPageBoundary = true;
            m_currentParagraphNum = 0;
            position = paragraph().firstWordPosition();
        }
    }

    setCurrentWordPosition(position);
    m_ttsStartPositionInParagraph = position.parPos();

    if (isPageBoundary) {
        sayTranslationTag("TOP_OF_PAGE");
    } else {
        m_wordNavigationWithDelay = m_state == State::SpeakingPage;
        sayText(paragraph().text().mid(position.parPos(), position.length()));
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
        } else if (ocr().isIdle()) {
            // Page finished
            sayTranslationTag("END_OF_TEXT");
            return;
        }
    }

    setCurrentWordPosition(position);
    m_ttsStartPositionInParagraph = position.parPos();

    m_wordNavigationWithDelay = m_state == State::SpeakingPage;
    sayText(paragraph().text().mid(position.parPos(), position.length()));
}

void MainController::backSentence()
{
    if (!isPageValid())
        return;

    bool isPageBoundary = false;
    auto position = paragraph().prevSentencePosition(m_currentWordPosition.parPos());
    while (!position.isValid()) {
        if (--m_currentParagraphNum >= 0) {
            // Go the the previous paragraph
            position = paragraph().lastSentencePosition();
        } else {
            // Go to the beginning of the page
            isPageBoundary = true;
            m_currentParagraphNum = 0;
            position = paragraph().firstSentencePosition();
        }
    }

    setCurrentWordPosition(position);
    m_ttsStartPositionInParagraph = position.parPos();

    if (isPageBoundary) {
        sayTranslationTag("TOP_OF_PAGE");
    } else if (m_state == State::SpeakingPage) {
        startSpeaking();
    } else {
        sayText(paragraph().text().mid(position.parPos(), position.length()));
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
        } else if (ocr().isIdle()) {
            // Page finished
            sayTranslationTag("END_OF_TEXT");
            return;
        }
    }

    setCurrentWordPosition(position);
    m_ttsStartPositionInParagraph = position.parPos();

    if (m_state == State::SpeakingPage) {
        startSpeaking();
    } else {
        sayText(paragraph().text().mid(position.parPos(), position.length()));
    }
}

void MainController::sayText(QString text)
{
    if (m_ttsEngine) {
        if (m_state != State::SpeakingText) {
            qDebug() << __func__ << "saving current state" << (int)m_state
                     << "and speaking text:" << text;
            m_prevState = m_state;
            m_state = State::SpeakingText;
        }
        m_ttsEngine->say(text);
    } else {
        qDebug() << "TTS engine is not created";
    }
}

void MainController::sayTranslationTag(const QString &tag)
{
    sayText(m_translator.GetString(tag.toStdString()).c_str());
}

void MainController::spellText(const QString &text)
{
    sayText(QStringLiteral("\x1b\\tn=spell\\%1").arg(text));
}

void MainController::speechRateUp()
{
    changeVoiceSpeed(20);
}

void MainController::speechRateDown()
{
    changeVoiceSpeed(-20);
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

void MainController::onToggleAudioSink() {
      toggleAudioSink();
      m_beepSound->play();
}

void MainController::readerReady() {
    stopBeeping();
    sayTranslationTag("PLACE_DOC");
    m_currentParagraphNum = -1;
    ocr().stopProcess();
}

void MainController::targetNotFound()
{
    sayTranslationTag("CLEAR_SURF");
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
        qDebug() << __func__ << "current paragraph" << m_currentParagraphNum
                 << "position in paragraph" << m_ttsStartPositionInParagraph;
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
        } else if (ocr().isIdle()) {
            // Page finished
            qDebug() << "Page finished";
            m_state = State::Stopped;
            sayTranslationTag("END_OF_TEXT");
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
    bool isAdvance = true;
    if (m_state == State::SpeakingText) {
        qDebug() << __func__ << "changing state to" << (int)m_prevState;
        m_state = m_prevState;
        isAdvance = false;
    }

    qDebug() << __func__ << "state" << (int)m_state;
    if (m_state == State::SpeakingPage) {
        m_ttsStartPositionInParagraph = m_currentWordPosition.parPos();
        if (isAdvance) {
            // Continue with the next word
            m_ttsStartPositionInParagraph += m_currentWordPosition.length();
            qDebug() << "advancing text to" << m_currentWordPosition.length();
        }
        qDebug() << __func__ << "position in paragraph" << m_ttsStartPositionInParagraph;
        startSpeaking(m_wordNavigationWithDelay ? DELAY_ON_NAVIGATION : 0);
    }

    m_wordNavigationWithDelay = false;
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
                    emit (this->*action)();
                    m_ignoreRelease = true;
                    break;
                }
            }
        });
}

void MainController::stopLongPressTimer() {
    m_nLongPressCount = -1;
}

void MainController::stopBeeping() {
//    qDebug() << __func__ << "beep\n";
    m_bKeepBeeping = false;
}

void MainController::SaveImage(int indx) {

}

void MainController::ReadImage(int indx) {

}

void MainController::onReadHelp() {
    sayText(m_help.GetString("BUTTON_HELP").c_str());
}

void MainController::onBtButton(int nButton, bool bDown) {
    if(bDown) {
        m_keypadButtonMask |= (1 << nButton);
        switch(nButton) {
        case KP_BUTTON_CENTER   :
            pauseResume();
            break;
        case KP_BUTTON_UP       :
            if((1 << KP_BUTTON_ROUND_L) & m_keypadButtonMask) {
                 startLongPressTimer(&MainController::toggleAudioOutput, 3000);
                break;
            }
            if((1 << KP_BUTTON_SQUARE_L) & m_keypadButtonMask) {
                SaveImage(1);
                break;
            }
            if((1 << KP_BUTTON_SQUARE_R) & m_keypadButtonMask) {
                ReadImage(1);
                break;
            }
            backSentence();
            break;
        case KP_BUTTON_DOWN     :
            if((1 << KP_BUTTON_SQUARE_L) & m_keypadButtonMask) {
                SaveImage(2);
                break;
            }
            if((1 << KP_BUTTON_SQUARE_R) & m_keypadButtonMask) {
                ReadImage(2);
                break;
            }
            nextSentence();
            break;
        case KP_BUTTON_LEFT     :
            if((1 << KP_BUTTON_RIGHT) & m_keypadButtonMask) {
                onToggleSingleColumn();
                break;
            }
            if((1 << KP_BUTTON_SQUARE_L) & m_keypadButtonMask) {
                SaveImage(3);
                break;
            }
            if((1 << KP_BUTTON_SQUARE_R) & m_keypadButtonMask) {
                ReadImage(3);
                break;
            }
            backWord();
            break;
        case KP_BUTTON_RIGHT    :
            if((1 << KP_BUTTON_LEFT) & m_keypadButtonMask) {
                onToggleSingleColumn();
                break;
            }
            if((1 << KP_BUTTON_SQUARE_L) & m_keypadButtonMask) {
                SaveImage(4);
                break;
            }
            if((1 << KP_BUTTON_SQUARE_R) & m_keypadButtonMask) {
                ReadImage(4);
                break;
            }
            nextWord();
            break;
        case KP_BUTTON_HELP     :
            startLongPressTimer(&MainController::readHelp, 3000);
            break;
        case KP_BUTTON_ROUND_L  :
            if((1 << KP_BUTTON_UP) & m_keypadButtonMask) {
                startLongPressTimer(&MainController::toggleAudioOutput, 3000);
                break;
            }
            break;
        case KP_BUTTON_ROUND_R  :
            startLongPressTimer(&MainController::spellCurrentWord, 3000);
            break;
         }
    }
    else {
        m_keypadButtonMask &= ~(1 << nButton);
        stopLongPressTimer();
        if(m_keypadButtonMask == 0) {
            if(m_ignoreRelease) {
                m_ignoreRelease = false;
            }
            else {
                switch(nButton) {
                case KP_BUTTON_ROUND_L  :
                    onToggleVoice();
                    break;
                case KP_BUTTON_SQUARE_L :
                    break;
                case KP_BUTTON_SQUARE_R :
                    break;
                }
            }
        }
    }
}

void MainController::onButton(int nButton, bool bDown) {
    if(bDown) {
        m_deviceButtonsMask |= (1 << nButton);
        switch(nButton) {
        case BUTTON_PAUSE_MASK   :
            if((1 << BUTTON_RATE_UP_MASK) & m_keypadButtonMask) {
                startLongPressTimer(&MainController::toggleAudioOutput, 3000);
                break;
            }
            if((1 << BUTTON_RATE_DN_MASK) & m_keypadButtonMask) {
                startLongPressTimer(&MainController::toggleVoice, 3000);
                break;
            }
            startLongPressTimer(&MainController::toggleGestures, 3000);
            break;
        case BUTTON_BACK_MASK       :
            startLongPressTimer(&MainController::resetDevice, 3000);
            break;
        case BUTTON_RATE_UP_MASK     :
            if((1 << BUTTON_RATE_DN_MASK) & m_keypadButtonMask) {
                m_ignoreRelease = true;
                onToggleSingleColumn();
                break;
            }
            if((1 << BUTTON_PAUSE_MASK) & m_keypadButtonMask) {
                startLongPressTimer(&MainController::toggleAudioOutput, 3000);
                break;
            }
            break;
        case BUTTON_RATE_DN_MASK     :
            if((1 << BUTTON_RATE_UP_MASK) & m_keypadButtonMask) {
                m_ignoreRelease = true;
                onToggleSingleColumn();
                break;
            }
            break;

         }
    }
    else {
        m_deviceButtonsMask &= ~(1 << nButton);
        stopLongPressTimer();
        if(m_deviceButtonsMask == 0) {
            if(m_ignoreRelease) {
                m_ignoreRelease = false;
             }
            else {
                switch(nButton) {
                case BUTTON_PAUSE_MASK   :
                    pauseResume();
                    break;
                case BUTTON_BACK_MASK       :
                    backSentence();
                    break;
                case BUTTON_RATE_UP_MASK     :
                    speechRateUp();
                    break;
                case BUTTON_RATE_DN_MASK     :
                    speechRateDown();
                    break;

                }
            }
        }
    }
}

void MainController::onBtBattery(int nVal) {

}

void MainController::onToggleVoice() {

}

void MainController::onSpellCurrentWord()
{
    // Don't start spelling current word if it's already speaking
    if (!isPageValid() || m_state == State::SpeakingPage || m_state == State::SpeakingText)
        return;

    auto position = paragraph().currentWordPosition(m_currentWordPosition.parPos());
    if (position.isValid()) {
        spellText(paragraph().text().mid(position.parPos(), position.length()));
    }
}

void MainController::changeVoiceSpeed(int nStep) {
    if(!m_ttsEngine)
        return;
    if (m_ttsEngine->isSpeaking())
        m_ttsEngine->pause();
    int nCurrRate = m_ttsEngine->getSpeechRate();
    qDebug() << "changeVoiceSpeed" << nCurrRate << Qt::endl;
    m_ttsEngine->setSpeechRate(nCurrRate + nStep);
    sayTranslationTag((nStep > 0) ? "SPEECH_SPEED_UP" : "SPEECH_SPEED_DN");
}

void MainController::onResetDevice() {
    if(m_beepSound)
        m_beepSound->play();
    system("reboot");
}

void MainController::onToggleGestures() {
    if(m_beepSound)
        m_beepSound->play();
}

void MainController::onToggleSingleColumn() {
    if(m_beepSound)
        m_beepSound->play();
}
