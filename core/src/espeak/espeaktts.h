#ifndef ESPEAKTTS_H
#define ESPEAKTTS_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QBuffer>
#include <QFuture>
#include <QVector>
#include <QTimer>
#include <QMap>
#include "../cerence/positionmapper.h"
#include <deque>
#include <espeak/speak_lib.h>
#include "../zyrlotts.h"

class QAudioOutput;
class TtsAudioLayer;

class espeaktts : public ZyrloTts
{
    Q_OBJECT

public:
    espeaktts(const QString &lang, const QString &voice, QObject *parent, TtsAudioLayer *pTtsAudioLayer);
    ~espeaktts();

    void say(const QString &text, int delayMs = 0);
    void stop();

    void setSpeechRate(int nRate); //Range 50 - 400
    int getSpeechRate();
    void onBufferReady(short *buf, size_t buffSize);


    void setVolume(int nVolume);
    int getVolume();
    bool isFinished() const;
    void onWordMarksAdded(const espeak_EVENT & ev);
    //void connectToAudioLayer();


signals:
    void savingAudioDone(QString sfilename);
    void usbKeyInsert(bool bInserted);

private:
    void initTTS(const QString &lang, const QString &voice);
    void stopAudio();

private:
    espeak_POSITION_TYPE m_position_type;
    espeak_AUDIO_OUTPUT m_output;
    int m_Buflength = 5000, m_Options=0;
 };

#endif // ESPEAKTTS_H
