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

#include <ve_ttsapi.h>
#include <ve_platform.h>
#include <vplatform.h>

class QAudioOutput;
class QIODevice;

class CerenceTTS : public QObject
{
    Q_OBJECT
public:
    CerenceTTS(QObject *parent);
    ~CerenceTTS();

    void say(const QString &text);
    void stop();

    char *buffer();
    size_t bufferSize();
    VE_MARKINFO *markBuffer();
    size_t markBufferSize();
    void bufferDone(size_t sizePcm, size_t sizeMarks);

signals:
    void sayStarted();
    void sayFinished();
    void wordMarksAdded();
    void wordNotify(int wordPosition, int wordLength);

private:
    void initTTS();
    void initAudio();
    void stopAudio();

private:
    VE_INSTALL              m_stInstall;
    VPLATFORM_RESOURCES     m_stResources;
    VE_HSPEECH              m_hSpeech;
    VE_HINSTANCE            m_hTtsInst;

    // array of parameters for the vocalizer
    VE_PARAM                m_ttsParam[16];
    VE_OUTDEVINFO           m_stOutDevInfo;

    QByteArray              m_ttsBuffer {100 * 1024, 0};
    QVector<VE_MARKINFO>    m_ttsMarkBuffer {100};
    QFuture<void>           m_ttsFuture;

    QAudioOutput           *m_audioOutput {nullptr};
    QBuffer                *m_audioIO {nullptr};
    QVector<VE_MARKINFO>    m_wordMarks;
    int                     m_currentWord {-1};

    QMutex                  m_wordMarksMutex;
};

