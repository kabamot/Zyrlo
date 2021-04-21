/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#include "cerencetts.h"

#include <algorithm>

#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <QDebug>
#include <QThread>
#include <QtConcurrent>

void WriteWaveHeader(FILE *fp, int nSampleRate, int nBitsPerSample, int nChannels, int nBuffSize);

static const char *INSTALL_PATHS[] = {
    "/opt/zyrlo/ve/languages",
};

static NUAN_ERROR vout_Write(VE_HINSTANCE hTtsInst, void *pUserData,
                             VE_CALLBACKMSG *pcbMessage)
{
    Q_UNUSED(hTtsInst);

    VE_OUTDATA *pTtsOutData = nullptr;
    CerenceTTS *p = reinterpret_cast<CerenceTTS *>(pUserData);
    NUAN_ERROR ret = NUAN_OK;

    switch (pcbMessage->eMessage) {
    case VE_MSG_BEGINPROCESS:
        qDebug() << "Text-to-speech process has started";
        break;

    case VE_MSG_ENDPROCESS:
        qDebug() << "Text-to-speech process has ended";
        if(p->outputToFile()) {
            p->writeToWave(p->getAudioOutFileName().toStdString().c_str());
        }
        break;

    case VE_MSG_OUTBUFREQ:
        pTtsOutData = static_cast<VE_OUTDATA *>(pcbMessage->pParam);
        if (pTtsOutData) {
            // a new buffer is requested
            pTtsOutData->pOutPcmBuf = p->buffer();
            pTtsOutData->cntPcmBufLen = p->bufferSize();
            pTtsOutData->pMrkList = p->markBuffer();
            pTtsOutData->cntMrkListLen = p->markBufferSize();
        }
        break;

    case VE_MSG_OUTBUFDONE:
        pTtsOutData = (VE_OUTDATA *)pcbMessage->pParam;
        if (pTtsOutData) {
            if (pTtsOutData->cntPcmBufLen != 0) {
                p->bufferDone(pTtsOutData->cntPcmBufLen, pTtsOutData->cntMrkListLen);
            }
        }
        break;

    default:
        // other messages are left unhandled.
        break;
    }

    return ret; // returning an error will cause the ve to stop processing.
}

CerenceTTS::CerenceTTS(const QString &voice, QObject *parent)
    : QObject(parent)
{
    initTTS(voice);
    initAudio();

    qDebug() << __func__;
}

CerenceTTS::~CerenceTTS()
{
    stop();

    if (m_hTtsInst.pHandleData != nullptr) {
        ve_ttsClose(m_hTtsInst);
    }

    if (m_hSpeech.pHandleData != nullptr) {
        ve_ttsUnInitialize(m_hSpeech);
    }

    vplatform_ReleaseInterfaces(&m_stInstall);
}

void CerenceTTS::say(const QString &text, int delayMs)
{
    stop();

    m_currentWord = -1;
    m_wordMarks.clear();
    m_audioIO->buffer().clear();
    m_audioIO->reset();

    m_positionMapper.setText(text);

    m_ttsFuture = QtConcurrent::run([this, text](){
        auto textBytes = text.toUtf8();

        VE_INTEXT inText;
        inText.eTextFormat = VE_NORM_TEXT;
        inText.szInText = textBytes.data();
        // Caveat: ve_ttsProcessText2Speech expects the number of bytes as cntTextLength
        inText.cntTextLength = static_cast<NUAN_U32>(textBytes.size());

        QElapsedTimer timer;
        timer.start();
        ve_ttsProcessText2Speech(m_hTtsInst, &inText);
        qDebug() << "TTS processing finished in" << timer.elapsed() << "ms";
    });

    m_speakingStartTimer.start(delayMs);
}

void CerenceTTS::stop()
{
    m_speakingStartTimer.stop();
    m_audioOutput->stop();

    ve_ttsStop(m_hTtsInst);
    m_ttsFuture.waitForFinished();
}

void CerenceTTS::pause()
{
    m_audioOutput->suspend();
}

void CerenceTTS::resume()
{
    m_audioOutput->resume();
}

bool CerenceTTS::isSpeaking() const
{
    return m_audioOutput->state() == QAudio::ActiveState;
}

bool CerenceTTS::isPaused() const
{
    return m_audioOutput->state() == QAudio::SuspendedState;
}

bool CerenceTTS::isStoppedSpeaking() const
{
    return m_audioOutput->state() == QAudio::StoppedState;
}

char *CerenceTTS::buffer()
{
    return m_ttsBuffer.data();
}

size_t CerenceTTS::bufferSize()
{
    return m_ttsBuffer.size();
}

VE_MARKINFO *CerenceTTS::markBuffer()
{
    return m_ttsMarkBuffer.data();
}

size_t CerenceTTS::markBufferSize()
{
    return m_ttsMarkBuffer.size() * sizeof(VE_MARKINFO);
}

void CerenceTTS::bufferDone(size_t sizePcm, size_t sizeMarks)
{
    if (sizePcm > 0) {
        auto totalSize = sizePcm;
        const auto audioBuffSize = static_cast<size_t>(m_audioOutput->bufferSize());
        if (sizePcm < audioBuffSize) {
            // Fill up to the buffer size with 0s, otherwise it will be not played
            std::fill(m_ttsBuffer.data() + sizePcm, m_ttsBuffer.data() + audioBuffSize, 0);
            totalSize = audioBuffSize;
        }
        m_audioIO->buffer().append(m_ttsBuffer.data(), totalSize);
    }

    if (sizeMarks > 0) {
        for (size_t i = 0; i < sizeMarks; ++i) {
            const auto &mark = m_ttsMarkBuffer.at(i);
            if (mark.eMrkType == VE_MRK_WORD) {
                m_wordMarksMutex.lock();
                m_wordMarks.append(mark);
                m_wordMarksMutex.unlock();

                emit wordMarksAdded();
            }
        }
    }
}

QStringList CerenceTTS::availableLanguages() const
{
    return m_languageNames.keys();
}

QStringList CerenceTTS::availableVoices(const QString &language) const
{
    return m_voicesMap.value(language);
}

const QMap<QString, QString> &CerenceTTS::languageNames() const
{
    return m_languageNames;
}

const QMap<QString, QStringList> &CerenceTTS::voicesMap() const
{
    return m_voicesMap;
}

void CerenceTTS::initTTS(const QString &voice)
{
    memset(&m_stInstall, 0, sizeof(m_stInstall));
    memset(&m_stResources, 0, sizeof(m_stResources));
    memset(&m_hSpeech, 0, sizeof(m_hSpeech));
    memset(&m_hTtsInst, 0, sizeof(m_hTtsInst));
    memset(&m_stOutDevInfo, 0, sizeof(m_stOutDevInfo));

    m_stInstall.fmtVersion = VE_CURRENT_VERSION;

    m_stResources.fmtVersion = VPLATFORM_CURRENT_VERSION;
    m_stResources.apDataInstall = const_cast<char **>(INSTALL_PATHS);
    m_stResources.u16NbrOfDataInstall = sizeof(INSTALL_PATHS) / sizeof(INSTALL_PATHS[0]);

    auto nErrcode = vplatform_GetInterfaces(&m_stInstall, &m_stResources);
    if (nErrcode != NUAN_OK) {
        qWarning() << __func__ << __LINE__ << "error:" << ve_ttsGetErrorString(nErrcode);
        return;
    }

    // Initialize the engine
    nErrcode = ve_ttsInitialize(&m_stInstall, &m_hSpeech);
    if (nErrcode != NUAN_OK) {
        qWarning() << __func__ << __LINE__ << "error:" << ve_ttsGetErrorString(nErrcode);
        return;
    }

    // Create the TTS instance
    nErrcode = ve_ttsOpen(m_hSpeech, m_stInstall.hHeap, m_stInstall.hLog, &m_hTtsInst);
    if (nErrcode != NUAN_OK) {
        qWarning() << __func__ << __LINE__ << "error:" << ve_ttsGetErrorString(nErrcode);
        return;
    }

    queryLanguagesVoicesInfo();

    VE_PARAM ttsParam[3];

    // First set voice
    ttsParam[0].eID = VE_PARAM_VOICE;
    strncpy(ttsParam[0].uValue.szStringValue, voice.toStdString().c_str(), VE_MAX_STRING_LENGTH);

    // and use UTF-8 as input text
    ttsParam[1].eID = VE_PARAM_TYPE_OF_CHAR;
    ttsParam[1].uValue.usValue = VE_TYPE_OF_CHAR_UTF8;

    ttsParam[2].eID = VE_PARAM_MARKER_MODE;
    ttsParam[2].uValue.usValue = (NUAN_U16) VE_MRK_ON;

    nErrcode = ve_ttsSetParamList(m_hTtsInst, &ttsParam[0], 3);
    if (nErrcode != NUAN_OK) {
        qWarning() << __func__ << __LINE__ << "error:" << ve_ttsGetErrorString(nErrcode);
        return;
    }
}

void CerenceTTS::initAudio()
{
    /* Set the output device */
    m_stOutDevInfo.pUserData = this;
    m_stOutDevInfo.pfOutNotify = vout_Write;

    ve_ttsSetOutDevice(m_hTtsInst, &m_stOutDevInfo);

    // Setup audioOut
    QAudioFormat format;
    // Set up the format, eg.
    format.setSampleRate(22050);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());

    if (!info.isFormatSupported(format)) {
        format = info.nearestFormat(format);
        qWarning() << "Raw audio format not supported by backend";
        qWarning() << "Using nearest format" << format.sampleRate() << format.sampleSize() << format.codec()
                   << format.byteOrder() << format.sampleType();
    }

    m_audioOutput = new QAudioOutput(format, this);
    m_audioOutput->setBufferSize(4096 * 4); // Give some buffer to remove stutter
    m_audioOutput->setNotifyInterval(50);
    qDebug() << __func__ << m_audioOutput->bufferSize() << m_audioOutput->notifyInterval();

    connect(m_audioOutput, &QAudioOutput::notify, this, [this](){
        const auto elapsedSamples = m_audioOutput->processedUSecs() *
                m_audioOutput->format().sampleRate() / 1'000'000;
        auto newCurrentWord = m_currentWord;

        {
            // Searching if the new word pronouncing
            QMutexLocker locker(&m_wordMarksMutex);
            for (int i = m_currentWord + 1; i < m_wordMarks.size(); ++i) {
                if (elapsedSamples >= static_cast<qint64>(m_wordMarks[i].cntDestPos)) {
                    newCurrentWord = i;
                } else {
                    break;
                }
            }
        }

        if (newCurrentWord > m_currentWord) {
            m_currentWord = newCurrentWord;
            emit wordNotify(m_positionMapper.position(m_wordMarks[m_currentWord].cntSrcPos),
                            m_wordMarks[m_currentWord].cntSrcTextLen);
        }
    });

    connect(m_audioOutput, &QAudioOutput::stateChanged, this, [this](QAudio::State state) {
        qDebug() << "state" << state;
        switch (state) {
        case QAudio::ActiveState:
            emit sayStarted();
            break;

        case QAudio::StoppedState:
            if (m_audioOutput->error() != QAudio::NoError) {
                // Error handling
                qWarning() << __func__ << __LINE__ << m_audioOutput->error();
            }
            break;

        case QAudio::IdleState:
            if (m_ttsFuture.isFinished()) {
                m_audioOutput->stop();
                emit sayFinished();
            }
            break;

        default:
            break;
        }
    });

    m_audioIO = new QBuffer(this);
    m_audioIO->open(QIODevice::ReadOnly);

    m_speakingStartTimer.setSingleShot(true);
    connect(&m_speakingStartTimer, &QTimer::timeout, this, [this](){
        if(!outputToFile())
            m_audioOutput->start(m_audioIO);
    });

}

void CerenceTTS::queryLanguagesVoicesInfo()
{
    m_voicesMap.clear();
    m_languageNames.clear();

    NUAN_U16 numLanguages = 0;
    auto nErrcode = ve_ttsGetLanguageList(m_hSpeech, NULL, &numLanguages);
    if (nErrcode != NUAN_OK) {
        qWarning() << __func__ << __LINE__ << "error:" << ve_ttsGetErrorString(nErrcode);
        return;
    }

    QByteArray buffLanguages(sizeof(VE_LANGUAGE) * numLanguages, 0);
    VE_LANGUAGE *pLangList = reinterpret_cast<VE_LANGUAGE *>(buffLanguages.data());

    nErrcode = ve_ttsGetLanguageList(m_hSpeech, pLangList, &numLanguages);
    if (nErrcode != NUAN_OK) {
        qWarning() << __func__ << __LINE__ << "error:" << ve_ttsGetErrorString(nErrcode);
        return;
    }

    for (int i = 0; i < numLanguages; ++i) {
        qDebug("%d>> %s, %s, %s", i,
               pLangList[i].szLanguage,
               pLangList[i].szLanguageTLW,
               pLangList[i].szVersion);

        const QString langCode = pLangList[i].szLanguageTLW; // ex. ENU, NON, etc
        const QString language = pLangList[i].szLanguage;    // American English, Norwegian

        NUAN_U16 numVoices = 0;
        nErrcode = ve_ttsGetVoiceList(m_hSpeech, pLangList[i].szLanguage, NULL, &numVoices);
        if (nErrcode != NUAN_OK) {
            qWarning() << __func__ << __LINE__ << "error:" << ve_ttsGetErrorString(nErrcode);
            return;
        }

        QByteArray buffVoices(sizeof(VE_VOICEINFO) * numVoices, 0);
        VE_VOICEINFO *pVoiceList = reinterpret_cast<VE_VOICEINFO *>(buffVoices.data());

        nErrcode = ve_ttsGetVoiceList(m_hSpeech, pLangList[i].szLanguage, pVoiceList, &numVoices);
        if (nErrcode != NUAN_OK) {
            qWarning() << __func__ << __LINE__ << "error:" << ve_ttsGetErrorString(nErrcode);
            return;
        }

        QStringList voices;
        for (int j = 0; j < numVoices; j++) {
            qDebug("  %d>> %s, %s, %s\n", j, pVoiceList[j].szVoiceName,
                   pVoiceList[j].szVoiceAge, pVoiceList[j].szVoiceType);

            voices.append(pVoiceList[j].szVoiceName);
        }

        m_voicesMap[langCode] = voices;
        m_languageNames[langCode] = language;
    }
}

void CerenceTTS::stopAudio() {
    if(m_audioIO)
        delete m_audioIO;
    if(m_audioOutput)
        delete m_audioOutput;
}

void CerenceTTS::resetAudio() {
    stopAudio();
    initAudio();
}

void CerenceTTS::setSpeechRate(int nRate) {
    VE_PARAM prm;
    prm.eID = VE_PARAM_SPEECHRATE;
    prm.uValue.usValue = nRate;
    auto nErrcode = ve_ttsSetParamList(m_hTtsInst, &prm, 1);
    if (nErrcode != NUAN_OK)
        qWarning() << __func__ << __LINE__ << "error:" << ve_ttsGetErrorString(nErrcode);
}

int CerenceTTS::getSpeechRate() {
    VE_PARAM prm;
    prm.eID = VE_PARAM_SPEECHRATE;
    prm.uValue.usValue = 0;
    auto nErrcode = ve_ttsGetParamList(m_hTtsInst, &prm, 1);
    if (nErrcode != NUAN_OK)
        qWarning() << __func__ << __LINE__ << "error:" << ve_ttsGetErrorString(nErrcode);
    return (int) prm.uValue.usValue;
}

bool CerenceTTS::writeToWave(const char *sFileName) {
    FILE *fp = fopen(sFileName, "w");
    if(!fp)
        return false;
    WriteWaveHeader(fp, 22050, 16, 1, m_audioIO->buffer().size());
    fwrite(m_audioIO->buffer().constData(), 2, m_audioIO->buffer().size(), fp);
    fclose(fp);
    m_bOutputToFile = false;
    emit convertTextToWaveDone(sFileName);
    return true;
}

void CerenceTTS::convertTextToWave(const QString & sText, const QString & sWaveFileName) {
    m_bOutputToFile = true;
    m_audioOutFileName = sWaveFileName;
    say(sText);
}

void WriteWaveHeader(FILE *fp, int nSampleRate, int nBitsPerSample, int nChannels, int nBuffSize) {
    fwrite("RIFF", 1, 4, fp);
    int nBiteRate = nSampleRate * nBitsPerSample / 8 * nChannels;
    int nSubchunk2Size = nBuffSize;
    int nChunksize = nSubchunk2Size + 36;
    fwrite(&nChunksize, 4, 1, fp);
    fwrite("WAVE", 1, 4, fp);
    char sTmp[4] = "fmt";
    sTmp[3] = 0x20;
    fwrite(sTmp, 1, 4, fp);
    sTmp[0] = 16;
    memset(sTmp + 1, 0, 3);
    fwrite(sTmp, 1, 4, fp);
    sTmp[0] = 1;
    sTmp[1] = sTmp[3] = 0;
    sTmp[2] = nChannels;
    fwrite(sTmp, 1, 4, fp);
    fwrite(&nSampleRate, 4 , 1, fp);
    fwrite(&nBiteRate, 4, 1, fp);
    sTmp[0] = 4;
    sTmp[1] = 0;
    sTmp[2] = nBitsPerSample;
    sTmp[3] = 0;
    fwrite(sTmp, 1, 4, fp);
    fwrite("data", 1, 4, fp);
    fwrite(&nSubchunk2Size, 4, 1, fp);
}
