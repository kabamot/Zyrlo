#ifndef ZYRLOTTS_H
#define ZYRLOTTS_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QBuffer>
#include <QFuture>
#include <QVector>
#include <QTimer>
#include <QMap>
#include <QAudioOutput>
#include <QMetaObject>
#include <deque>

#include <ve_ttsapi.h>

#include "cerence/positionmapper.h"

class TtsAudioLayer;

class ZyrloTts : public QObject
{
    Q_OBJECT

protected:
    bool m_bOutputToFile = false;
    QString m_audioOutFileName;

public:
    ZyrloTts(QObject *parent = nullptr, TtsAudioLayer *pTtsAudioLayer = nullptr);

    virtual void say(const QString &text, int delayMs = 0) = 0;
    virtual void sayAfter(const QString &text);
    virtual void stop() = 0;
    virtual void pause();
    virtual void resume();

    virtual bool isSpeaking() const;
    virtual bool isPaused() const;
    virtual bool isStoppedSpeaking() const;

    virtual void setSpeechRate(int nRate) = 0; //Range 50 - 400
    virtual int getSpeechRate() = 0;

    virtual bool writeToWave(const char *sFileName);
    virtual bool outputToFile() const { return m_bOutputToFile; }
    virtual void convertTextToAudio(const QString & sText, const QString & sAudioFileName);
    virtual const QString & getAudioOutFileName() const { return m_audioOutFileName; }
    virtual bool writeToMp3(const char *sFileName);
    virtual void setVolume(int nVolume) = 0;
    virtual int getVolume() = 0;
    virtual void connectToAudioLayer();
    virtual void disconnectFromAudioLayer();

signals:
    void sayStarted();
    void sayFinished();
    void wordMarksAdded();
    void wordNotify(int wordPosition, int wordLength);

protected:
    QFuture<void>           m_ttsFuture;

    QVector<VE_MARKINFO>    m_wordMarks;
    int                     m_currentWord {-1};

    QMutex                  m_wordMarksMutex;
    PositionMapper          m_positionMapper;
    QMutex m_messageQueMutex;
    std::deque<QString> m_messageQue;
    TtsAudioLayer *m_pTtsAudioLayer {nullptr};
    QMetaObject::Connection m_connectNotify, m_connectChanged;
};

#endif // ZYRLOTTS_H
