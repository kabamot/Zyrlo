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
#include "kbdinputinjector.h"

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

    typedef enum  {
        BY_SYMBOL = 0,
        BY_WORD,
        BY_SENTENCE,
        BY_PARAGRAPH,
        NUM_OF_NAV_MODES
    } NavigationMode;

public:
    MainController();
    ~MainController();

    void waitForSayTextFinished();

    void startFile(const QString &filename);
    void startImage(const cv::Mat &image);
    void snapImage();
    void flashLed();
    void setLed(bool bOn);
    bool toggleAudioSink();
    bool switchToBuiltInSink();
    void onToggleVoice();
    void SaveImage(int indx);
    void ReadImage(int indx);
    void onReadHelp();
    void onSpellCurrentWord();
    bool write_keypad_config(const std::string & text);
    void SaySN();
    QString translateTag(const QString &tag) const;

    void resetAudio();
    void getListOfLanguges(QStringList & list) const;
    void toggleVoiceEnabled(int nIndx);
    void saveVoiceSettings();
    void setMenuOpen(bool bMenuOpen);
    void readSettings();
    void writeSettings() const;
    void toggleNavigationMode(bool bForward);
    void onLeftArrow();
    void onRightArrow();

signals:
    void textUpdated(const QString &text);
    void finished();
    void wordPositionChanged(const TextPosition &position);
    void previewUpdated(const cv::Mat & img);
    void toggleAudioOutput();
    void spellCurrentWord();
    void toggleGestures();
    void toggleVoice();
    void readHelp();
    void openMainMenu();
    void sayBatteryStatus();

public slots:
    void pauseResume();
    void pause();
    void resume();
    void backWord();
    void nextWord();
    void backSentence();
    void nextSentence();

    void nextParagraph();
    void backParagraph();
    void nextSymbol();
    void backSymbol();

    void sayText(QString text);
    void sayTranslationTag(const QString &tag);
    void spellText(const QString &text);
    void speechRateUp();
    void speechRateDown();
    void nextVoice();
    void onToggleAudioSink();

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
    bool read_keypad_config();
    void InitTtsEngines();
    void ReleaseTtsEngines();
    bool setAutoSink(int indx);
    QString GetCharName(QChar c) const;

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
    void onToggleGestures();
    void onToggleSingleColumn();
    void onGesture(int nGest);
    void onSayBatteryStatus();

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
    char m_btKbdMac[64] = {0};
    bool m_bVoiceSettingsChanged = false;
    KbdInputInjector m_kbdInjctr;
    bool m_bMenuOpen = false;
    NavigationMode m_navigationMode = BY_WORD;
    int m_nCurrNavPos = -1;
    bool m_bUseCameraFlash = true;
};

