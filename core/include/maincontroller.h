/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Nov 2020
 *
 ****************************************************************************/

#pragma once

#include <QObject>
#include <QSound>
#include <QFuture>
#include "translator.h"
#include "textposition.h"

class OcrHandler;
class Paragraph;
class CerenceTTS;
class HWHandler;

namespace cv {
    class Mat;
}

class MainController : public QObject
{
    Q_OBJECT

    enum class State {
        Stopped,
        SpeakingPage,
        SpeakingText,   // speaking arbitrary short/auxiliary text retaining current place in page
        Paused,
    };

public:
    MainController();

    void start(const QString &filename);
    void snapImage();
    void flashLed();
    void setLed(bool bOn);
    bool toggleAudioSink();
    void onToggleVoice();

signals:
    void textUpdated(const QString &text);
    void formattedTextUpdated(const QString &text);
    void finished();
    void wordPositionChanged(const TextPosition &position);
    void previewUpdated(const cv::Mat & img);
    void toggleAudioOutput();
    void spellCurrentWord();
    void resetDevice();
    void toggleGestures();
    void toggleVoice();
    void readHelp();

public slots:
    void pauseResume();
    void backWord();
    void nextWord();
    void backSentence();
    void nextSentence();
    void sayText(const QString &text);

private:
    OcrHandler &ocr();
    const OcrHandler &ocr() const;
    void startSpeaking(int delayMs = 0);
    const Paragraph &paragraph() const;
    void startBeeping();
    void stopBeeping();
    void startLongPressTimer(void (MainController::*action)(void), int nDelay);
    void stopLongPressTimer();
    void setCurrentWordPosition(const TextPosition &textPosition);
    bool isPageValid() const;
    void SaveImage(int indx);
    void ReadImage(int indx);
    void onReadHelp();
    void onSpellCurrentWord();
    void changeVoiceSpeed(int nStep);

private slots:
    void onNewTextExtracted();
    void onSpeakingFinished();
    void setCurrentWord(int wordPosition, int wordLength);
    void previewImgUpdate(const cv::Mat & prevImg);
    void readerReady();
    void targetNotFound();
    void onBtButton(int nButton, bool bDown);
    void onButton(int nButton, bool bDown);
    void onBtBattery(int nVal);
    void onToggleAudioSink();
    void onResetDevice();
    void onToggleGestures();
    void onToggleSingleColumn();

private:
    CerenceTTS *m_ttsEngine {nullptr};
    HWHandler  *m_hwhandler {nullptr};
    int         m_ttsStartPositionInParagraph {0};
    int         m_currentParagraphNum {-1};
    QString     m_currentText;
    TextPosition m_currentWordPosition;
    State       m_prevState {State::Stopped};
    State       m_state {State::Stopped};
    QSound *m_shutterSound {nullptr}, *m_beepSound{nullptr};
    Translator m_translator, m_help;
    bool m_bKeepBeeping = false;
    QFuture<void> m_beepingThread, m_longPressTimerThread;
    bool m_ignoreRelease = false;
    unsigned int m_deviceButtonsMask = 0, m_keypadButtonMask = 0;
    int m_nLongPressCount = -1;
};

