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
public:
    MainController();

    void start(const QString &filename);
    void snapImage();
    void flashLed();
    void setLed(bool bOn);
    bool toggleAudioSink();

signals:
    void textUpdated(const QString &text);
    void formattedTextUpdated(const QString &text);
//    void paragraphUpdated(const Paragrah &paragraph);
    void finished();
    void wordNotify(int wordPosition, int wordLength);
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
    void startLongPressTimer(void (*action)(void), int nDelay);
    void stopLongPressTimer();

private slots:
    void onNewTextExtracted();
    void onSpeakingFinished();
    void setCurrentWord(int wordPosition, int wordLength);

private:
    CerenceTTS *m_ttsEngine {nullptr};
    HWHandler  *m_hwhandler {nullptr};
    int         m_ttsStartPositionInParagraph {0};
    int         m_currentParagraphNum {-1};

    int         m_wordPosition {0};
    int         m_wordLength {0};
    QString     m_currentText;
    QSound *m_shutterSound {nullptr}, *m_beepSound{nullptr};
    Translator m_translator;
    bool m_bKeepBeeping = false;
    QFuture<void> m_beepingThread, m_longPressTimerThread;
    bool m_squareLeftDown = false, m_squareRightDown = false, m_voiceDown = false,  m_ignoreVoice = false;
    int m_nLongPressCount = -1;

 private slots:
    void previewImgUpdate(const cv::Mat & prevImg);
    void readerReady();
    void targetNotFound();
    void onBtButton(int nButton, bool bDown);
    void onBtBattery(int nVal);
 };

