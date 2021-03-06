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
#include <deque>

class OcrHandler;
class Paragraph;
class ZyrloTts;
class HWHandler;
class TtsAudioLayer;

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
    bool ChangeCameraExposure(int delta);
    bool ChangeCameraGain(int delta);
    bool ChangeCameraExposureStep(int delta);

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
    void getListOfAboutItems(QStringList & list) const;
    bool isMenuOpen() const { return m_bMenuOpen; }
    void getListOfOptions(QStringList & list) const;
    void toggleOption(int nIndx);
    void convertTextToAudio(const QString & sText, const QString & sAudioFileName);
    bool saveScannedImage(const cv::Mat & img);
    bool isSpeaking();
    bool isPlayingSound();
    bool ProcessScannedImage(const std::string & path);
    bool StartProcessScannedImages();
    bool saveScannedText() const;
    bool ConvertTextToAudio(const std::string & sPath);
    bool ProcessNextScannedImg();
    void onReaderReady();
    void SetLocalLightFreqTest(bool bOn);
    int getCurrentExposure() const;
    int getCurrentGain() const;
    int getExposureStep() const;
    bool setSpeakerSetting(int nSetting);
    void changeVoiceVolume(int nStep);
    bool tryProcessScannedImages();
    void SetTTsEngine(int nIndx);
    void setFullResPreview(bool bOn);
    void startBatteryTest();


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
    void saveMainBatterylevel();

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

    void sayText(QString text, bool bAfter = false);
    void sayTranslationTag(const QString &tag, bool bAfter = false);
    void spellText(const QString &text);
    void speechRateUp();
    void speechRateDown();
    void nextVoice();
    void onToggleAudioSink();
    void onSaveMainBatterylevel() const;

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
    //void populateVoices();
    void SetCurrentTts(const QString & lang);
    void SetDefaultTts();
     void InitTtsEngines();
    void ReleaseTtsEngines();
    bool setAudioSink(int indx);
    QString GetCharName(QChar c) const;
    int numOfParagraphs() const;

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
    void onSavingAudioDone(QString sFileName);
    void onUsbKeyInsert(bool bInserted);
    void onUsbKpConnect(bool bConnected);
    void onBtKpRegistered();


private:
    QVector<ZyrloTts *>m_ttsEnginesList;
    ZyrloTts *m_ttsEngine {nullptr};
    int         m_currentTTSIndex {0};
    HWHandler  *m_hwhandler {nullptr};
    int         m_ttsStartPositionInParagraph {0};
    int         m_currentParagraphNum {-1};
    QString     m_currentText;
    TextPosition m_currentWordPosition;
    State       m_prevState {State::Stopped};
    State       m_state {State::Stopped};
    QSound *m_shutterSound {nullptr}, *m_beepSound{nullptr}, *m_armOpenSound{nullptr}, *m_armClosedSound{nullptr};
    Translator m_translator;//, m_help;
    bool        m_wordNavigationWithDelay {false};  // Determines if it's need to do delay before continue page reading
    bool        m_isContinueAfterSpeakingFinished {true};
    bool m_bKeepBeeping = false;
    QFuture<void> m_beepingThread, m_longPressTimerThread, m_batteryTestThread;
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
    int m_nBuiltInSink = 0, m_nActiveSink = 0;
    std::string m_sCurrentBookDir;
    bool m_bUsbKeyInserted = false;
    std::string m_sCurrentImgPath;
    std::deque<std::string> m_vScannedImagesQue;
    int m_nImagesToConvert = 0;
    TtsAudioLayer *m_pTtsAudioLayer {nullptr};
};

