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

class QAudioOutput;
class QIODevice;

class CerenceTTS : public QObject
{
    Q_OBJECT
public:
    CerenceTTS(const QString &voice, QObject *parent);
    ~CerenceTTS();

    void say(const QString &text, int delayMs = 0);
    void stop();
    void pause();
    void resume();

    bool isSpeaking() const;
    bool isPaused() const;
    bool isStoppedSpeaking() const;

    char *buffer();
    size_t bufferSize();
    VE_MARKINFO *markBuffer();
    size_t markBufferSize();
    void bufferDone(size_t sizePcm, size_t sizeMarks);
    void resetAudio();
    void setSpeechRate(int nRate); //Range 50 - 400
    int getSpeechRate();

    QStringList availableLanguages() const;
    QStringList availableVoices(const QString &language) const;
    const QMap<QString, QStringList> &voicesMap() const;
    const QMap<QString, QString> &languageNames() const;
    bool writeToWave(const char *sFileName);
    bool outputToFile() const { return m_bOutputToFile; }
    void convertTextToWave(const QString & sText, const QString & sWaveFileName);
    const QString & getAudioOutFileName() const { return m_audioOutFileName; }

signals:
    void sayStarted();
    void sayFinished();
    void wordMarksAdded();
    void wordNotify(int wordPosition, int wordLength);
    void convertTextToWaveDone(QString sfilename);
    void usbKeyInsert(bool bInserted);

private:
    void initTTS(const QString &voice);
    void initAudio();
    void stopAudio();
    void queryLanguagesVoicesInfo();

private:
    VE_INSTALL              m_stInstall;
    VPLATFORM_RESOURCES     m_stResources;
    VE_HSPEECH              m_hSpeech;
    VE_HINSTANCE            m_hTtsInst;

    // array of parameters for the vocalizer
    VE_OUTDEVINFO           m_stOutDevInfo;

    QByteArray              m_ttsBuffer {100 * 1024, 0};
    QVector<VE_MARKINFO>    m_ttsMarkBuffer {100};
    QFuture<void>           m_ttsFuture;

    QAudioOutput           *m_audioOutput {nullptr};
    QBuffer                *m_audioIO {nullptr};
    QVector<VE_MARKINFO>    m_wordMarks;
    int                     m_currentWord {-1};

    QMutex                  m_wordMarksMutex;
    QTimer                  m_speakingStartTimer;
    PositionMapper          m_positionMapper;
    QMap<QString, QString>  m_languageNames;
    QMap<QString, QStringList>  m_voicesMap;
    bool m_bOutputToFile = false;
    QString m_audioOutFileName;
};

