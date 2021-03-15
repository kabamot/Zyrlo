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
#include "ZyrloOcr.h"
#include <regex>

using namespace cv;
using namespace std;

#define SHUTER_SOUND_WAVE_FILE "/opt/zyrlo/Distrib/Data/camera-shutter-click-01.wav"
#define BEEP_SOUND_WAVE_FILE "/opt/zyrlo/Distrib/Data/beep-08b.wav"
#define ARMOPEN_SOUND_FILE "/opt/zyrlo/Distrib/Data/button-09.wav"
#define ARMCLOSED_SOUND_FILE "/opt/zyrlo/Distrib/Data/button-10.wav"
#define TRANSLATION_FILE "/opt/zyrlo/Distrib/Data/ZyrloTranslate.xml"
#define HELP_FILE "/opt/zyrlo/Distrib/Data/ZyrloHelp.xml"

constexpr int DELAY_ON_NAVIGATION = 1000; // ms, delay before starting TTS
constexpr int LONG_PRESS_DELAY = 1500;

struct LangVoice {
    QString lang;
    QString voice;
};

struct LangVoiceComb {
    string m_sDescription;
    vector<string> m_vlangs;
    unsigned long long m_uLang_mask;
    vector<int> m_ttsEngIndxs;
};

const vector<LangVoiceComb> g_vLangVoiceSettings {
    {"Malcolm", {"eng"}, ZRL_ENGLISH_US, {0}},
    //{"Nora", {"nor"}, ZRL_NORWEGIAN, {1}},
    {"Henrik", {"nor"}, ZRL_NORWEGIAN, {2}},
    {"og Engelsk", {"nor", "eng"}, ZRL_NORWEGIAN|ZRL_ENGLISH_US, {2, 0}}
};

const QVector<LangVoice> LANGUAGES = {
    { "eng", "Malcolm" },
    { "nor", "nora" },
    { "nor", "henrik" },
};

MainController::MainController()
{
    connect(&ocr(), &OcrHandler::lineAdded, this, [this](){
        emit textUpdated(ocr().textPage()->text());
    });

    connect(&ocr(), &OcrHandler::lineAdded, this, &MainController::onNewTextExtracted);
    connect(this, &MainController::toggleAudioOutput, this, &MainController::onToggleAudioSink);
    connect(this, &MainController::spellCurrentWord, this, &MainController::onSpellCurrentWord);
    connect(this, &MainController::resetDevice, this, &MainController::onResetDevice);
    connect(this, &MainController::toggleGestures, this, &MainController::onToggleGestures);
    connect(this, &MainController::toggleVoice, this, &MainController::onToggleVoice);
    connect(this, &MainController::readHelp, this, &MainController::onReadHelp);

    m_hwhandler = new HWHandler(this, read_keypad_config());
    connect(m_hwhandler, &HWHandler::imageReceived, this, [this](const Mat &image, bool bPlayShutterSound){
        qDebug() << "imageReceived 0\n";
        if(bPlayShutterSound)
            m_ttsEngine->stop();

        qDebug() << "imageReceived 2\n";
        if(m_shutterSound && bPlayShutterSound)
            m_shutterSound->play();
        startBeeping();
        startImage(image);
    }, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::buttonReceived, this, [](Button button){
        qDebug() << "received" << (int)button;
    }, Qt::QueuedConnection);

    // Create TTS engines
    for (const auto &language : LANGUAGES) {
        auto ttsEngine = new CerenceTTS(language.voice, this);
        connect(ttsEngine, &CerenceTTS::wordNotify, this, &MainController::setCurrentWord);
        connect(ttsEngine, &CerenceTTS::sayFinished, this, &MainController::onSpeakingFinished);
        connect(ttsEngine, &CerenceTTS::sayStarted, m_hwhandler, &HWHandler::onSpeakingStarted);
        m_ttsEnginesList.append(ttsEngine);
    }

    m_ttsEngine = m_ttsEnginesList.front();
    populateVoices();

    connect(m_hwhandler, &HWHandler::previewImgUpdate, this, &MainController::previewImgUpdate, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::readerReady, this, &MainController::readerReady, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::targetNotFound, this, &MainController::targetNotFound, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::onBtButton, this, &MainController::onBtButton, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::onButton, this, &MainController::onButton, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::onBtBattery, this, &MainController::onBtBattery, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::onGesture, this, &MainController::onGesture, Qt::QueuedConnection);

    m_translator.Init(TRANSLATION_FILE);
    m_help.Init(HELP_FILE);
    m_shutterSound = new QSound(SHUTER_SOUND_WAVE_FILE, this);
    m_beepSound = new QSound(BEEP_SOUND_WAVE_FILE, this);
    m_armOpenSound = new QSound(ARMOPEN_SOUND_FILE, this);
    m_armClosedSound = new QSound(ARMCLOSED_SOUND_FILE, this);
    m_hwhandler->start();
}

void MainController::startFile(const QString &filename)
{
    cv::Mat image = cv::imread(filename.toStdString(), cv::IMREAD_GRAYSCALE);
    startImage(image);
}

void MainController::startImage(const Mat &image)
{
    m_ttsEngine->stop();

    m_ttsStartPositionInParagraph = 0;
    m_currentParagraphNum = 0;
    m_currentWordPosition.clear();
    m_wordNavigationWithDelay = false;
    m_isContinueAfterSpeakingFinished = true;
    ocr().setForceSingleColumn(m_bForceSingleColumn);
    m_bForceSingleColumn = false;
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
            m_hwhandler->UnlockBtConnect();
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
        m_isContinueAfterSpeakingFinished = true;
        if (m_ttsEngine->isPaused()) {
            m_ttsEngine->resume();
        } else {
            m_ttsStartPositionInParagraph = m_currentWordPosition.parPos();
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
        sayText(prepareTextToSpeak(paragraph().text().mid(position.parPos(), position.length())));
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
        } else if (ocr().textPage()->isComplete()) {
            // Page finished
            sayTranslationTag("END_OF_TEXT");
            return;
        }
    }

    setCurrentWordPosition(position);
    m_ttsStartPositionInParagraph = position.parPos();

    m_wordNavigationWithDelay = m_state == State::SpeakingPage;
    sayText(prepareTextToSpeak(paragraph().text().mid(position.parPos(), position.length())));
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
            position = paragraph().firstWordPosition();
            m_state = State::Paused;
        }
    }

    setCurrentWordPosition(position);
    m_ttsStartPositionInParagraph = position.parPos();

    if (isPageBoundary) {
        sayTranslationTag("TOP_OF_PAGE");
    } else if (m_state == State::SpeakingPage && m_isContinueAfterSpeakingFinished) {
        startSpeaking();
    } else {
        m_isContinueAfterSpeakingFinished = false;
        m_state = State::SpeakingPage;
        m_ttsEngine->say(prepareTextToSpeak(paragraph().text().mid(position.parPos(), position.length())));
    }
}

void MainController::nextSentence()
{
    if (!isPageValid())
        return;

    auto position = paragraph().nextSentencePosition(std::max(0, m_currentWordPosition.parPos() - 1));
    if (!position.isValid()) {
        if (m_currentParagraphNum + 1 <= ocr().processingParagraphNum()) {
            // Go the the next paragraph
            ++m_currentParagraphNum;
            position = paragraph().firstSentencePosition();
        } else if (ocr().textPage()->isComplete()) {
            // Page finished
            sayTranslationTag("END_OF_TEXT");
            return;
        }
    }

    setCurrentWordPosition(position);
    m_ttsStartPositionInParagraph = position.parPos();

    if (m_state == State::SpeakingPage && m_isContinueAfterSpeakingFinished) {
        startSpeaking();
    } else {
        m_isContinueAfterSpeakingFinished = false;
        m_state = State::SpeakingPage;
        m_ttsEngine->say(prepareTextToSpeak(paragraph().text().mid(position.parPos(), position.length())));
    }
}

void MainController::sayText(QString text)
{
    SetDefaultTts();
    if (m_ttsEngine) {
        if (m_state != State::SpeakingText) {
            qDebug() << __func__ << "saving current state" << (int)m_state
                     << "and speaking text:" << text;
            m_prevState = m_state;
            m_state = State::SpeakingText;
        }
        m_ttsEngine->say(text);
    } else {
        qWarning() << "TTS engine is not created, can't say text:" << text;
    }
}

void MainController::sayTranslationTag(const QString &tag)
{
    sayText(m_translator.GetString(tag.toStdString()).c_str());
}

void MainController::spellText(const QString &text)
{
    sayText(QStringLiteral(CERENCE_ESC R"(\tn=spell\%1)").arg(text));
}

void MainController::speechRateUp()
{
    changeVoiceSpeed(20);
}

void MainController::speechRateDown()
{
    changeVoiceSpeed(-20);
}

void MainController::nextVoice()
{
    if (++m_currentTTSIndex >= m_ttsEnginesList.size())
        m_currentTTSIndex = 0;

    m_ttsEngine->stop();

//    sayText(m_voices[m_currentVoiceNum]);
//    QString voice = m_voices[m_currentVoiceNum].split(',').back().trimmed();

    const auto langVoice = LANGUAGES[m_currentTTSIndex];
    m_ttsEngine = m_ttsEnginesList[m_currentTTSIndex];

    m_translator.SetLanguage(langVoice.lang.toStdString());
    QString voiceText = QStringLiteral(R"(%1, %2 %3\pause=%4\)")
                            .arg(m_translator.GetString("VOICE_SET_TO").c_str(), langVoice.voice, CERENCE_ESC)
                            .arg(m_state == State::SpeakingPage ? 500 : 0);
    sayText(voiceText);
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
    if(m_armOpenSound) {
        delete m_armOpenSound;
        m_armOpenSound = new QSound(ARMOPEN_SOUND_FILE, this);
    }
    if(m_armClosedSound) {
        delete m_armClosedSound;
        m_armClosedSound = new QSound(ARMOPEN_SOUND_FILE, this);
    }
    for (auto ttsEngine : m_ttsEnginesList) {
        ttsEngine->resetAudio();
    }
    return bret;
}

void MainController::onToggleAudioSink() {
      toggleAudioSink();
      m_beepSound->play();
}

void MainController::readerReady() {
    stopBeeping();
    m_state = State::Paused;
    if (m_ttsEngine->isSpeaking()) {
        m_ttsEngine->pause();
    }
    sayTranslationTag("PLACE_DOC");
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
        auto currText = ocr().textPage()->getText(m_currentParagraphNum, m_ttsStartPositionInParagraph);

        if (!currText.second.isEmpty()) {
            m_currentText = currText.second;
            SetCurrentTts(currText.first);
            // Continue speaking if there is more text in the current paragraph
            qDebug() << __func__ << m_currentText;
            m_ttsEngine->say(prepareTextToSpeak(m_currentText), delayMs);
        } else if (m_currentParagraphNum + 1 <= ocr().processingParagraphNum()) {
            // Advance to the next paragraph if the current one is completed and
            // all text pronounced
            ++m_currentParagraphNum;
            m_ttsStartPositionInParagraph = 0;
            continue;
        } else if (ocr().textPage()->isComplete()) {
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
    auto tp = ocr().textPage();
    return tp != nullptr && m_currentParagraphNum >= 0 && m_currentParagraphNum <  tp->numParagraphs();
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
    const bool isSpeakingTextState = m_state == State::SpeakingText;
    if (isSpeakingTextState) {
        qDebug() << __func__ << "changing state to" << (int)m_prevState;
        m_state = m_prevState;
    }

    qDebug() << __func__ << "state" << (int)m_state;
    if (m_state == State::SpeakingPage && m_isContinueAfterSpeakingFinished) {
        m_ttsStartPositionInParagraph = m_currentWordPosition.parPos();
        if (!isSpeakingTextState) {
            // Continue with the next word
            m_ttsStartPositionInParagraph += m_currentWordPosition.length();
            qDebug() << "advancing text to" << m_currentWordPosition.length();
        }
        qDebug() << __func__ << "position in paragraph" << m_ttsStartPositionInParagraph;
        startSpeaking(m_wordNavigationWithDelay ? DELAY_ON_NAVIGATION : 0);
    }
    else
        m_hwhandler->UnlockBtConnect();
    if (!isSpeakingTextState && !m_isContinueAfterSpeakingFinished) {
        // The normal mode (not service text speak) with stop after sentence is finished,
        // so set continue mode back to true
        m_state = State::Paused;
        m_currentWordPosition = TextPosition(m_currentWordPosition.parPos() + m_currentWordPosition.length(),
                                             m_currentWordPosition.length(),
                                             paragraph().paragraphPosition());
    }

    m_wordNavigationWithDelay = false;
 }

void MainController::setCurrentWord(int wordPosition, int wordLength)
{
    if(!isPageValid() || m_state == State::SpeakingText || m_currentParagraphNum < 0)
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
            for(m_bKeepBeeping = true, QThread::msleep(2000); m_bKeepBeeping; QThread::msleep(1000)) {
                if(!m_ttsEngine->isSpeaking())
                    m_beepSound->play();
            }
        });
}

void MainController::startLongPressTimer(void (MainController::*action)(void), int nDelay) {
    const int intvl = 100;
    m_nLongPressCount = nDelay / intvl;
    m_longPressAction = action;
    if (!m_longPressTimerThread.isRunning())
        m_longPressTimerThread = QtConcurrent::run([this]() {
            for(; m_nLongPressCount >= 0; --m_nLongPressCount, QThread::msleep(intvl)) {
                if(m_nLongPressCount == 0) {
                    emit (this->*m_longPressAction)();
                    m_keypadButtonMask = m_deviceButtonsMask = 0;
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
    m_hwhandler->saveImage(indx);
    sayTranslationTag("PAGE_SAVED");
}

void MainController::ReadImage(int indx) {
    if(!m_hwhandler->recallSavedImage(indx)) {
        m_beepSound->play();
        return;
    }
    sayTranslationTag("PAGE_RECALL");
    m_hwhandler->readRecallImage();
}

void MainController::onReadHelp() {
    m_keypadButtonMask = (1 << KP_BUTTON_HELP);
    sayText(m_help.GetString("BUTTON_HELP").c_str());
}

static string getHelpTag(int nButton) {
    switch(nButton) {
    case KP_BUTTON_CENTER  :
        return "BUTTON_PAUSE_RESUME";
     case KP_BUTTON_UP      :
        return "BUTTON_ARROW_UP";
    case KP_BUTTON_DOWN    :
        return "BUTTON_ARROW_DOWN";
    case KP_BUTTON_LEFT    :
        return "BUTTON_ARROW_LEFT";
    case KP_BUTTON_RIGHT   :
        return "BUTTON_ARROW_RIGHT";
    case KP_BUTTON_ROUND_L :
        return "BUTTON_VOICE";
    case KP_BUTTON_ROUND_R :
        return "BUTTON_SPELL";
    case KP_BUTTON_SQUARE_L:
        return "BUTTON_SAVE";
    case KP_BUTTON_SQUARE_R:
        return "BUTTON_RECALL";
    }
    return "";
}

void MainController::onBtButton(int nButton, bool bDown) {
    if(nButton < 1 || nButton > 10)
        return;
    if(bDown) {
        m_keypadButtonMask |= (1 << nButton);
        switch(nButton) {
        case KP_BUTTON_CENTER   :
            pauseResume();
            break;
        case KP_BUTTON_UP       :
            if((1 << KP_BUTTON_ROUND_L) & m_keypadButtonMask) {
                startLongPressTimer(&MainController::toggleAudioOutput, LONG_PRESS_DELAY);
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
            startLongPressTimer(&MainController::readHelp, LONG_PRESS_DELAY);
            break;
        case KP_BUTTON_ROUND_L  :
            if((1 << KP_BUTTON_UP) & m_keypadButtonMask) {
                startLongPressTimer(&MainController::toggleAudioOutput, LONG_PRESS_DELAY);
                break;
            }
            break;
        case KP_BUTTON_ROUND_R  :
            onSpellCurrentWord();
            break;
        }
    }
    else {
        qDebug() << "m_keypadButtonMask =" << m_keypadButtonMask << ((1 << KP_BUTTON_HELP) & m_keypadButtonMask) << Qt::endl;
        stopLongPressTimer();
        if(nButton == KP_BUTTON_HELP) {
            m_ttsEngine->stop();
            m_keypadButtonMask = 0;
        }
        if((1 << KP_BUTTON_HELP) & m_keypadButtonMask) {
            m_keypadButtonMask = (1 << KP_BUTTON_HELP);
            sayText(m_help.GetString(getHelpTag(nButton)).c_str());
            return;
        }
        if(m_keypadButtonMask != 0) {
            m_keypadButtonMask = 0;
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

static int findOneOfTheButtons(int nButtons) {
    if(nButtons == 0)
        return 0;
    int n = 1;
    for(; (n & nButtons) == 0; n <<= 1);
    return n;
}

void MainController::onButton(int nButton, bool bDown) {
    if(bDown) {
        if(SWITCH_FOLDED_MASK & nButton) {
            if(m_armOpenSound)
                m_armOpenSound->play();
            m_hwhandler->setCameraArmPosition(true);
            setLed(true);
        }
        m_deviceButtonsMask |= nButton;
        switch(findOneOfTheButtons(nButton)) {
        case BUTTON_PAUSE_MASK   :
            if(BUTTON_RATE_UP_MASK & m_deviceButtonsMask) {
                startLongPressTimer(&MainController::toggleAudioOutput, LONG_PRESS_DELAY);
                break;
            }
            if(BUTTON_RATE_DN_MASK & m_deviceButtonsMask) {
                startLongPressTimer(&MainController::toggleVoice, LONG_PRESS_DELAY);
                break;
            }
            startLongPressTimer(&MainController::toggleGestures, LONG_PRESS_DELAY);
            break;
        case BUTTON_BACK_MASK       :
            startLongPressTimer(&MainController::resetDevice, LONG_PRESS_DELAY);
            break;
        case BUTTON_RATE_UP_MASK     :
            if(BUTTON_RATE_DN_MASK & m_deviceButtonsMask) {
                m_deviceButtonsMask = 0;
                onToggleSingleColumn();
                break;
            }
            if(BUTTON_PAUSE_MASK & m_deviceButtonsMask) {
                startLongPressTimer(&MainController::toggleAudioOutput, LONG_PRESS_DELAY);
                break;
            }
            break;
        case BUTTON_RATE_DN_MASK     :
            if(BUTTON_RATE_UP_MASK & m_deviceButtonsMask) {
                m_deviceButtonsMask = 0;
                onToggleSingleColumn();
                break;
            }
            if(BUTTON_PAUSE_MASK & m_deviceButtonsMask) {
                startLongPressTimer(&MainController::toggleVoice, LONG_PRESS_DELAY);
                break;
            }
            break;
         }
    }
    else {
        qDebug() << "m_deviceButtonsMask =" << m_deviceButtonsMask <<Qt::endl;
        stopLongPressTimer();
        if(nButton == SWITCH_FOLDED_MASK_UP) {
            m_hwhandler->setCameraArmPosition(true);
            setLed(true);
           return;
        }
        if(SWITCH_FOLDED_MASK & nButton) {
            m_hwhandler->setCameraArmPosition(false);
            setLed(false);
            if(m_armClosedSound)
                m_armClosedSound->play();
        }
        if(m_deviceButtonsMask != 0) {
            m_deviceButtonsMask = 0;
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

void MainController::onBtBattery(int nVal) {

}

void MainController::onToggleVoice() {
    m_beepSound->play();
    int nIndx = (m_nCurrentLangaugeSettingIndx + 1) % g_vLangVoiceSettings.size();
    if(!ocr().setLanguage(g_vLangVoiceSettings[nIndx].m_uLang_mask)) {
        ocr().stopProcess();
        return;
    }
    m_nCurrentLangaugeSettingIndx = nIndx;
    m_translator.SetLanguage(g_vLangVoiceSettings[m_nCurrentLangaugeSettingIndx].m_vlangs.front());
    m_currentTTSIndex = g_vLangVoiceSettings[m_nCurrentLangaugeSettingIndx].m_ttsEngIndxs.front();
    m_ttsEngine = m_ttsEnginesList[m_currentTTSIndex];
    string sMsg = m_translator.GetString("VOICE_SET_TO") + " " + g_vLangVoiceSettings[m_nCurrentLangaugeSettingIndx].m_sDescription;
    sayText(sMsg.c_str());
}

void MainController::SetCurrentTts(const QString & lang) {
    qDebug() << "SetCurrentTts" << lang << Qt::endl;
    const vector<string> & vlangs = g_vLangVoiceSettings[m_nCurrentLangaugeSettingIndx].m_vlangs;
    const vector<int> & vindxs = g_vLangVoiceSettings[m_nCurrentLangaugeSettingIndx].m_ttsEngIndxs;
    for(size_t i = 0; i != vlangs.size(); ++i)
        if(lang.compare(vlangs[i].c_str()) == 0) {
            if(m_currentTTSIndex == vindxs[i])
                return;
             m_currentTTSIndex = vindxs[i];
            m_ttsEngine = m_ttsEnginesList[m_currentTTSIndex];
        }
}

void MainController::SetDefaultTts() {
    qDebug() << "SetDefaultTts\n";
    if(m_currentTTSIndex == g_vLangVoiceSettings[m_nCurrentLangaugeSettingIndx].m_ttsEngIndxs[0])
        return;
    m_ttsEngine->stop();
    m_currentTTSIndex = g_vLangVoiceSettings[m_nCurrentLangaugeSettingIndx].m_ttsEngIndxs[0];
    m_ttsEngine = m_ttsEnginesList[m_currentTTSIndex];
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

QString MainController::prepareTextToSpeak(QString text)
{
    // Spell 5+ digit numbers by digits
    static const QRegularExpression re(R"((\d{5,}))");
    text.replace(re, CERENCE_ESC R"(\tn=spell\\1)" CERENCE_ESC R"(\tn=normal\)");
    return text;
}

void MainController::populateVoices()
{
    if (!m_ttsEngine)
        return;

    m_voices.clear();

    for (const auto &langCode : m_ttsEngine->availableLanguages()) {
        const auto voices = m_ttsEngine->availableVoices(langCode);
        for (const auto &voice : voices) {
            QString languageAndVoice = QStringLiteral("%1, %2").arg(m_ttsEngine->languageNames()[langCode], voice);
            m_voices.push_back(languageAndVoice);
        }
    }

    qDebug() << __func__ << m_voices;
}

void MainController::onResetDevice() {
    if(m_beepSound)
        m_beepSound->play();
    system("reboot");
}

void MainController::onToggleGestures() {
    bool bOn = !m_hwhandler->gesturesOn();
    sayTranslationTag(bOn ? "GESTURES_ON" : "GESTURES_OFF");
    m_hwhandler->setGesturesUi(bOn);
}

void MainController::onToggleSingleColumn() {
    if(m_beepSound)
        m_beepSound->play();
    m_bForceSingleColumn = !m_bForceSingleColumn;
    sayTranslationTag(m_bForceSingleColumn ? "READ_THRU_COLUMNS" : "READ_NORMAL");
}

void MainController::onGesture(int nGest) {
    switch(nGest) {
    case 1:
        backSentence();
    case 2:
        pauseResume();
    }
}

MainController::~MainController() {
    if(m_shutterSound)
        delete m_shutterSound;
    if(m_beepSound)
        delete m_beepSound;
    if(m_armOpenSound)
        delete m_armOpenSound;
    if(m_armClosedSound)
        delete m_armClosedSound;
}


bool MainController::read_keypad_config() {
    FILE *fp = fopen("/home/pi/keypad_config.txt", "r");
    if(!fp)
        return false;
    fscanf(fp, "%s", m_btKbdMac);
    fclose(fp);
    return true;
}

bool MainController::write_keypad_config(const string & text) {
    if(text.length() != 4) {
        sayText("Invalid serial number");
        return false;
    }
    string sMac = string("40:F5:20:47:") + text.substr(0, 2) + ':' + text.substr(2);
    if(regex_match(sMac, regex("^([0-9A-F]{2}[:-]){5}([0-9A-F]{2})$"))) {
        strcpy(m_btKbdMac, sMac.c_str());
        system((string("echo ") + sMac + " > /home/pi/keypad_config.txt").c_str());
        sayText("Serial number saved");
        return true;
    }
    sayText("Invalid serial number");
    return false;
}

void MainController::SaySN() {
    if(!m_btKbdMac[0]) {
        sayText("No keypad");
        return;
    }
    string sn = regex_replace( m_btKbdMac, regex(":"), " " );
    sayText(sn.substr(12).c_str());
}
