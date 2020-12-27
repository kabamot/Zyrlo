/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#include "cerencettsprivate.h"

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
    CerenceTTSPrivate *p = reinterpret_cast<CerenceTTSPrivate *>(pUserData);
    NUAN_ERROR ret = NUAN_OK;

    switch (pcbMessage->eMessage) {
    case VE_MSG_BEGINPROCESS:
        qDebug() << "Text-to-speech process has started";
        break;

    case VE_MSG_ENDPROCESS:
        qDebug() << "Text-to-speech process has ended";
        break;

    case VE_MSG_OUTBUFREQ:
        qDebug() << "Text-to-speech a new buffer is requested";
        // a new buffer is requested
        pTtsOutData = static_cast<VE_OUTDATA *>(pcbMessage->pParam);
        pTtsOutData->pOutPcmBuf = p->buffer();
        pTtsOutData->cntPcmBufLen = p->bufferSize();
        break;

    case VE_MSG_OUTBUFDONE:
        pTtsOutData = (VE_OUTDATA *)pcbMessage->pParam;
        qDebug() << "Text-to-speech buffer done, size" << pTtsOutData->cntPcmBufLen;
        if (pTtsOutData->cntPcmBufLen != 0) {
            p->bufferDone(pTtsOutData->cntPcmBufLen);
        }
        break;

    default:
        // other messages are left unhandled.
        break;
    }

    return ret; // returning an error will cause the ve to stop processing.
}

CerenceTTSPrivate::CerenceTTSPrivate(CerenceTTS *p)
    : m_parent(p)
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

    nErrcode = ve_ttsSetParamList(m_hTtsInst, &m_ttsParam[0], 2);
    if (nErrcode != NUAN_OK) {
        qWarning() << __func__ << __LINE__ << "error:" << ve_ttsGetErrorString(nErrcode);
        return;
    }

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

    m_audioOut = new QAudioOutput(format, p);
    m_audioOut->setBufferSize(4096);
    QObject::connect(m_audioOut, &QAudioOutput::stateChanged, p, [this](QAudio::State state){
        qDebug() << "state" << state;
        switch (state) {
        case QAudio::StoppedState:
            if (m_audioOut->error() != QAudio::NoError) {
                // Error handling
                qWarning() << __func__ << __LINE__ << m_audioOut->error();
            }
            break;

        case QAudio::IdleState:
            stop();
            break;

        default:
            break;
        }
    });

    m_audioIO = new QBuffer(p);
    m_audioIO->open(QIODevice::ReadOnly);

    qDebug() << __func__;
}

CerenceTTSPrivate::~CerenceTTSPrivate()
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

void CerenceTTSPrivate::say(const QString &text)
{
    stop();

    m_audioIO->buffer().clear();
    m_audioIO->reset();
    m_audioOut->start(m_audioIO);

    m_ttsFuture = QtConcurrent::run([this, text](){
        qDebug() << __func__ << m_audioOut->bufferSize() << m_audioOut->format() ;
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

void CerenceTTSPrivate::stop()
{
    m_audioOut->stop();
    ve_ttsStop(m_hTtsInst);
    m_ttsFuture.waitForFinished();
}

char *CerenceTTSPrivate::buffer()
{
    return m_ttsBuffer.data();
}

size_t CerenceTTSPrivate::bufferSize()
{
    return m_ttsBuffer.size();
}

void CerenceTTSPrivate::bufferDone(size_t size)
{
    m_audioIO->buffer().append(m_ttsBuffer.data(), size);
    qDebug() << __func__ << size;
}
