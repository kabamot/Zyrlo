/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#pragma once

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QBuffer>
#include <QFuture>
#include <QVector>
#include <QTimer>
#include <QMap>

#include <ve_ttsapi.h>
#include <ve_platform.h>
#include <vplatform.h>

#include "cerencetts_const.h"
#include "positionmapper.h"
#include "../zyrlotts.h"

class QAudioOutput;
class QIODevice;

#include <deque>

class CerenceTTS : public ZyrloTts
{
    Q_OBJECT

public:
    CerenceTTS(const QString &voice, QObject *parent, TtsAudioLayer **ppTtsAudioLayer);
    ~CerenceTTS();

    void say(const QString &text, int delayMs = 0);
    void stop();

    void bufferDone(size_t sizePcm, size_t sizeMarks);
    void setSpeechRate(int nRate); //Range 50 - 400
    int getSpeechRate();

    void setVolume(int nVolume);
    int getVolume();
    virtual char *buffer();
    virtual size_t bufferSize();
    VE_MARKINFO *markBuffer();
    virtual size_t markBufferSize();

signals:
    void savingAudioDone(QString sfilename);
    void usbKeyInsert(bool bInserted);

private:
    void initTTS(const QString &voice);
    void initAudio();
    //void queryLanguagesVoicesInfo();

private:
    QByteArray              m_ttsBuffer {100 * 1024, 0};
    QVector<VE_MARKINFO>    m_ttsMarkBuffer {100};

    VE_INSTALL              m_stInstall;
    VPLATFORM_RESOURCES     m_stResources;
    VE_HSPEECH              m_hSpeech;
    VE_HINSTANCE            m_hTtsInst;

    // array of parameters for the vocalizer
    VE_OUTDEVINFO           m_stOutDevInfo;

//    QMap<QString, QString>  m_languageNames;
//    QMap<QString, QStringList>  m_voicesMap;

};

