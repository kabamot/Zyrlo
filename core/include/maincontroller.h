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
    ~MainController();

    void startFile(const QString &filename);
    void startImage(const cv::Mat &image);
    void snapImage();
    void flashLed();
    void setLed(bool bOn);
    bool toggleAudioSink();
    void onToggleVoice();
    void SaveImage(int indx);
    void ReadImage(int indx);
    void onReadHelp();
    void onSpellCurrentWord();


signals:
    void textUpdated(const QString &text);
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
    void sayText(QString text);
    void sayTranslationTag(const QString &tag);
    void spellText(const QString &text);
    void speechRateUp();
    void speechRateDown();
    void nextVoice();

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
    void changeVoiceSpeed(int nStep);
    QString prepareTextToSpeak(QString text);
    void populateVoices();
    void SetCurrentTts(const QString & lang);
    void SetDefaultTts();

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
    void onGesture(int nGest);

private:
    QVector<CerenceTTS *>m_ttsEnginesList;
    CerenceTTS *m_ttsEngine {nullptr};
    int         m_currentTTSIndex {0};
    QStringList m_voices;
    HWHandler  *m_hwhandler {nullptr};
    int         m_ttsStartPositionInParagraph {0};
    int         m_currentParagraphNum {-1};
    QString     m_currentText;
    TextPosition m_currentWordPosition;
    State       m_prevState {State::Stopped};
    State       m_state {State::Stopped};
    QSound *m_shutterSound {nullptr}, *m_beepSound{nullptr}, *m_armOpenSound{nullptr}, *m_armClosedSound{nullptr};
    Translator m_translator, m_help;
    bool        m_wordNavigationWithDelay {false};  // Determines if it's need to do delay before continue page reading
    bool        m_isContinueAfterSpeakingFinished {true};
    bool m_bKeepBeeping = false;
    QFuture<void> m_beepingThread, m_longPressTimerThread;
    unsigned int m_deviceButtonsMask = 0, m_keypadButtonMask = 0;
    int m_nLongPressCount = -1;
    void (MainController::*m_longPressAction)(void);
    int m_nCurrentLangaugeSettingIndx = 0;
    bool m_bForceSingleColumn = false;
};

