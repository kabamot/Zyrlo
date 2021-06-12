#include "ttsaudiolayer.h"
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QDebug>
#include "zyrlotts.h"
#include <lame/lame.h>

TtsAudioLayer *TtsAudioLayer::m_pTtsAudioLayer = NULL;

TtsAudioLayer *TtsAudioLayer::instance(QObject *parent) {
    if(!m_pTtsAudioLayer) {
        if(!parent)
            return NULL;
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
        m_pTtsAudioLayer = new TtsAudioLayer(format, parent);
    }
    return m_pTtsAudioLayer;
}

TtsAudioLayer::TtsAudioLayer(const QAudioFormat &format, QObject *parent)
    : QAudioOutput(format, parent)
{
    setBufferSize(4096 * 4); // Give some buffer to remove stutter
    setNotifyInterval(50);
    m_audioIO = new QBuffer(this);
    m_audioIO->open(QIODevice::ReadOnly);

    m_speakingStartTimer.setSingleShot(true);
    connect(&m_speakingStartTimer, &QTimer::timeout, this, [this](){
            start(m_audioIO);
    });
}

void TtsAudioLayer::clear() {
    m_audioIO->buffer().clear();
    m_audioIO->reset();
}

void TtsAudioLayer::startTimer(int delayMs) {
    m_speakingStartTimer.start(delayMs);
}

void TtsAudioLayer::stop() {
    m_speakingStartTimer.stop();
    QAudioOutput::stop();
}

void TtsAudioLayer::appendSample(const char *pSample, size_t size) {
    m_audioIO->buffer().append(pSample, size);
}

TtsAudioLayer *TtsAudioLayer::reset() {
    QObject *parent = m_pTtsAudioLayer->parent();
    delete m_pTtsAudioLayer;
    m_pTtsAudioLayer = NULL;
    m_pTtsAudioLayer = instance(parent);
    return m_pTtsAudioLayer;
}

TtsAudioLayer::~TtsAudioLayer() {
    if(m_audioIO)
        delete m_audioIO;
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

bool TtsAudioLayer::writeToWave(const char *sFileName) {
    int nBuffSize = m_audioIO->buffer().size();
    if(nBuffSize < 1)
        return false;
    FILE *fp = fopen(sFileName, "wb");
    if(!fp)
        return false;
    WriteWaveHeader(fp, 22050, 16, 1, nBuffSize);
    int nWritten = fwrite(m_audioIO->buffer().constData(), 2, nBuffSize, fp);
    fclose(fp);
    return nWritten == nBuffSize;
}

bool TtsAudioLayer::writeToMp3(const char *sFileName) {
    int nBuffSize = m_audioIO->buffer().size();
    if(nBuffSize < 1)
        return false;
    FILE *fp = fopen(sFileName, "wb");
    if(!fp)
        return false;
    const int PCM_SIZE = 8192;
    const int MP3_SIZE = 1.25 * PCM_SIZE * 0.5  + 7200;
    unsigned char mp3_buffer[MP3_SIZE];
    lame_t lame = lame_init();
    lame_set_in_samplerate(lame, 22050);
    lame_set_VBR(lame, vbr_default);
    lame_set_mode(lame, MONO);
    lame_set_num_channels(lame, 1);
    lame_init_params(lame);
    int nWritten = 0;
    for(int i = 0, chunk = std::min(PCM_SIZE, nBuffSize); chunk > 0; nBuffSize -=  PCM_SIZE, chunk = std::min(PCM_SIZE, nBuffSize), ++i) {
        nWritten = lame_encode_buffer(lame, (short int*)(m_audioIO->buffer().constData() + i * PCM_SIZE), NULL, chunk / 2, mp3_buffer, MP3_SIZE);
        fwrite(mp3_buffer, nWritten, 1, fp);
    }
    nWritten = lame_encode_flush(lame, mp3_buffer, MP3_SIZE);
    fwrite(mp3_buffer, nWritten, 1, fp);
    lame_close(lame);
    fclose(fp);
    return true;
}
