/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#include "cerencetts.h"

#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <QDebug>
#include <QThread>
#include <QtConcurrent>

#include "cerencetts.h"

static const char *INSTALL_PATHS[] = {
    "/opt/zyrlo/ve/languages",
};

static const auto VOICE_NAME = "ava";

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
        break;

    case VE_MSG_OUTBUFREQ:
        pTtsOutData = static_cast<VE_OUTDATA *>(pcbMessage->pParam);
        if (pTtsOutData) {
            // a new buffer is requested
            qDebug() << "Text-to-speech a new buffer is requested";
            pTtsOutData->pOutPcmBuf = p->buffer();
            pTtsOutData->cntPcmBufLen = p->bufferSize();
            pTtsOutData->pMrkList = p->markBuffer();
            pTtsOutData->cntMrkListLen = p->markBufferSize();
        }
        break;

    case VE_MSG_OUTBUFDONE:
        pTtsOutData = (VE_OUTDATA *)pcbMessage->pParam;
        if (pTtsOutData) {
            qDebug() << "Text-to-speech buffer done, size" << pTtsOutData->cntPcmBufLen;
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

CerenceTTS::CerenceTTS(QObject *parent)
    : QObject(parent)
{
    initTTS();
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

void CerenceTTS::say(const QString &text)
{
    stop();

    m_marks.clear();
    m_audioIO->buffer().clear();
    m_audioIO->reset();
    m_audioOutput->start(m_audioIO);

    m_ttsFuture = QtConcurrent::run([this, text](){
        qDebug() << __func__ << m_audioOutput->bufferSize() << m_audioOutput->format() ;
        auto textBytes = text.toUtf8();

        VE_INTEXT inText;
        inText.eTextFormat = VE_NORM_TEXT;
        inText.szInText = textBytes.data();
        // Caveat: ve_ttsProcessText2Speech expects the number of bytes as cntTextLength
        inText.cntTextLength = static_cast<NUAN_U32>(textBytes.size());

        QElapsedTimer timer;
        timer.start();
        ve_ttsProcessText2Speech(m_hTtsInst, &inText);
        qDebug() << __func__ << "finished in" << timer.elapsed() << "ms";
    });
}

void CerenceTTS::stop()
{
    m_audioOutput->stop();
    ve_ttsStop(m_hTtsInst);
    m_ttsFuture.waitForFinished();
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
    qDebug() << __func__ << sizePcm << sizeMarks;
    if (sizePcm > 0) {
        m_audioIO->buffer().append(m_ttsBuffer.data(), sizePcm);
    }

    if (sizeMarks > 0) {
        for (size_t i = 0; i < sizeMarks; ++i) {
            const auto &mark = m_ttsMarkBuffer.at(i);
            if (mark.eMrkType == VE_MRK_WORD) {
                qDebug() << mark.eMrkType << mark.cntSrcPos << mark.cntSrcTextLen << mark.cntDestPos;
                m_marks.append(mark);
            }
        }
    }
}

void CerenceTTS::initTTS()
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

    // set up params: Specify the voice that we want to use.
    m_ttsParam[0].eID = VE_PARAM_VOICE;
    strncpy(m_ttsParam[0].uValue.szStringValue, VOICE_NAME, VE_MAX_STRING_LENGTH);
    // and use UTF-8 as input text
    m_ttsParam[1].eID = VE_PARAM_TYPE_OF_CHAR;
    m_ttsParam[1].uValue.usValue = VE_TYPE_OF_CHAR_UTF8;

    m_ttsParam[2].eID = VE_PARAM_MARKER_MODE;
    m_ttsParam[2].uValue.usValue = (NUAN_U16) VE_MRK_ON;

    nErrcode = ve_ttsSetParamList(m_hTtsInst, &m_ttsParam[0], 3);
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

//    const auto deviceInfos = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
//    for (const QAudioDeviceInfo &deviceInfo : deviceInfos)
//        qDebug() << "Device name: " << deviceInfo.deviceName();

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
    m_audioOutput->setBufferSize(4096);
    QObject::connect(m_audioOutput, &QAudioOutput::stateChanged,
                     this, [this](QAudio::State state)
    {
        qDebug() << "state" << state;
        switch (state) {
        case QAudio::StoppedState:
            if (m_audioOutput->error() != QAudio::NoError) {
                // Error handling
                qWarning() << __func__ << __LINE__ << m_audioOutput->error();
            }
            break;

        case QAudio::IdleState:
//            stop();
            break;

        default:
            break;
        }
    });

    m_audioIO = new QBuffer(this);
    m_audioIO->open(QIODevice::ReadOnly);
}
