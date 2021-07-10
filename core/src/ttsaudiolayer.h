#ifndef TTSAUDIOLAYER_H
#define TTSAUDIOLAYER_H

#include <QAudioOutput>
#include <QBuffer>
#include <QTimer>
#include <QByteArray>

class TtsAudioLayer : public QAudioOutput {

    QBuffer *m_audioIO {nullptr};

    QTimer m_speakingStartTimer;

    static TtsAudioLayer *m_pTtsAudioLayer;

    bool m_bOutputToFile =false;

    TtsAudioLayer(const QAudioFormat &format, QObject *parent);

public:
    virtual ~TtsAudioLayer();
    static TtsAudioLayer *instance(QObject *parent = NULL);
    static TtsAudioLayer *reset();
    void clear();
    void startTimer(int delayMs);
    void stop();
    void appendSample(const char *pSample, size_t size);
    bool writeToWave(const char *sFileName);
    bool writeToMp3(const char *sFileName);
    void setOutputToFile(bool bOutputToFile) { m_bOutputToFile = bOutputToFile; }
 };

#endif // TTSAUDIOLAYER_H

