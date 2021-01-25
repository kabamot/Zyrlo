/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Nov 2020
 *
 ****************************************************************************/

#pragma once

#include <QObject>
#include <QSound>
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
    void setCurrentWordPosition(const TextPosition &textPosition);
    bool isPageValid() const;

private slots:
    void onNewTextExtracted();
    void onSpeakingFinished();
    void setCurrentWord(int wordPosition, int wordLength);
    void previewImgUpdate(const cv::Mat & prevImg);
    void readerReady();
    void targetNotFound();

private:
    CerenceTTS *m_ttsEngine {nullptr};
    HWHandler  *m_hwhandler {nullptr};
    int         m_ttsStartPositionInParagraph {0};
    int         m_currentParagraphNum {-1};
    QString     m_currentText;
    TextPosition m_currentWordPosition;
    QSound *m_shutterSound {nullptr}, *m_beepSound{nullptr};
    Translator m_translator;
};

