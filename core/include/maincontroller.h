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
        Speaking,
        Paused,
    };

public:
    MainController();

    void start(const QString &filename);
    void snapImage();
    void flashLed();
    void setLed(bool bOn);
    bool toggleAudioSink();
    void toggleAudioSinkVoid();

signals:
    void textUpdated(const QString &text);
    void formattedTextUpdated(const QString &text);
    void finished();
    void wordPositionChanged(const TextPosition &position);
    void previewUpdated(const cv::Mat & img);

public slots:
    void pauseResume();
    void backWord();
    void nextWord();
    void backSentence();
    void nextSentence();

private:
    OcrHandler &ocr();
    const OcrHandler &ocr() const;
    void startSpeaking();
    const Paragraph &paragraph() const;
    void startBeeping();
    void stopBeeping();
    void startLongPressTimer(void (MainController::*action)(void), int nDelay);
    void stopLongPressTimer();
    void setCurrentWordPosition(const TextPosition &textPosition);
    bool isPageValid() const;

private slots:
    void onNewTextExtracted();
    void onSpeakingFinished();
    void setCurrentWord(int wordPosition, int wordLength);
    void previewImgUpdate(const cv::Mat & prevImg);
    void readerReady();
    void targetNotFound();
    void onBtButton(int nButton, bool bDown);
    void onBtBattery(int nVal);

private:
    CerenceTTS *m_ttsEngine {nullptr};
    HWHandler  *m_hwhandler {nullptr};
    int         m_ttsStartPositionInParagraph {0};
    int         m_currentParagraphNum {-1};
    QString     m_currentText;
    TextPosition m_currentWordPosition;
    State       m_state {State::Stopped};
    QSound *m_shutterSound {nullptr}, *m_beepSound{nullptr};
    Translator m_translator;
    bool m_bKeepBeeping = false;
    QFuture<void> m_beepingThread, m_longPressTimerThread;
    bool m_squareLeftDown = false, m_squareRightDown = false, m_buttonUpRessed = false, m_voiceDown = false,  m_ignoreVoice = false;
    int m_nLongPressCount = -1;

};

