/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Nov 2020
 *
 ****************************************************************************/

#pragma once

#include <QObject>

class OcrHandler;
class Paragraph;
class CerenceTTS;
class HWHandler;

class MainController : public QObject
{
    Q_OBJECT
public:
    MainController();

    void start(const QString &filename);

signals:
    void textUpdated(const QString &text);
    void formattedTextUpdated(const QString &text);
//    void paragraphUpdated(const Paragrah &paragraph);
    void finished();
    void wordNotify(int wordPosition, int wordLength);

public slots:
    void pauseResume();
    void backWord();
    void nextWord();
    void backSentence();
    void nextSentence();

private:
    OcrHandler &ocr();
    void startSpeaking();

private slots:
    void onNewTextExtracted();
    void onSpeakingFinished();

private:
    CerenceTTS *m_ttsEngine {nullptr};
    HWHandler  *m_hwhandler {nullptr};

    int         m_positionInParagraph {0};
    int         m_currentParagraphNum {-1};

    int         m_wordPosition {0};
    int         m_wordLength {0};
    QString     m_currentText;
};

