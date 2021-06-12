#include "espeaktts.h"

#include <algorithm>

#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <QDebug>
#include <QThread>
#include <QtConcurrent>
#include <QMutexLocker>
#include <lame/lame.h>
#include "../ttsaudiolayer.h"

using namespace std;

espeaktts::espeaktts(const QString &lang, const QString &voice, QObject *parent, TtsAudioLayer *pTtsAudioLayer)
    : ZyrloTts(parent, pTtsAudioLayer)
{
    initTTS(lang, voice);
    qDebug() << __func__;
}

espeaktts::~espeaktts() {

}

string LangToEspeakLang(const QString & lang) {
    if(lang.compare("eng") == 0)
        return "en";
    if(lang.compare("ita") == 0)
        return "it";
    if(lang.compare("fre") == 0)
        return "fr";
    return "";
}

int callBack(short *buffer, int n, espeak_EVENT* ev) {
    espeaktts *pEspeaktts = NULL;
    if(ev[0].type != 0) {
        pEspeaktts = ((espeaktts*)ev[0].user_data);
        pEspeaktts->onBufferReady(buffer, n);
    }
    int i = 0;
    size_t sizeMarks = 0;
    for(; ev[i].type != 0; ++i) {
        qDebug() << ev[i].type;
        switch(ev[i].type) {
        case espeakEVENT_END:
        case espeakEVENT_MSG_TERMINATED:
            break;
        case espeakEVENT_WORD:
            if(ev[i].length > 0) {
                pEspeaktts->onWordMarksAdded(ev[i]);
                ++sizeMarks;
            }
            break;
        default:
            break;
        }
    }
    return 0;
}

void espeaktts::onWordMarksAdded(const espeak_EVENT & ev) {
    VE_MARKINFO mark;
    memset(&mark, 0, sizeof(VE_MARKINFO));
    mark.eMrkType = VE_MRK_WORD;
    mark.cntSrcPos = ev.text_position;
    mark.cntDestPos = ev.audio_position * m_pTtsAudioLayer->format().sampleRate() / 1000;
    mark.cntSrcTextLen = ev.length;
    m_wordMarksMutex.lock();
    m_wordMarks.append(mark);
    m_wordMarksMutex.unlock();
    emit wordMarksAdded();
}

void espeaktts::initTTS(const QString &lang, const QString &voice) {
    m_output = AUDIO_OUTPUT_RETRIEVAL;
    espeak_Initialize(m_output, m_Buflength, NULL, m_Options);
    espeak_SetSynthCallback(callBack);
    setlocale(LC_ALL, "C");

    espeak_VOICE esvoice;
    memset(&esvoice, 0, sizeof(espeak_VOICE)); // Zero out the voice first
    esvoice.languages = LangToEspeakLang(lang).c_str();
    esvoice.name = voice.toStdString().c_str();
    esvoice.variant = 0;
    esvoice.gender = 1;
    espeak_SetVoiceByProperties(&esvoice);
}

void espeaktts::say(const QString &text, int delayMs) {
    stop();

    m_currentWord = -1;
    m_wordMarks.clear();
    m_pTtsAudioLayer->clear();

    m_positionMapper.setText(text);

    m_ttsFuture = QtConcurrent::run([this, text]() {
        auto textBytes = text.toUtf8();

        espeak_POSITION_TYPE position_type = POS_WORD;
        unsigned int Size, position=0, end_position=0, flags=espeakCHARS_UTF8, unique_identifier;

        Size = strlen(textBytes) + 1;
        espeak_Synth( textBytes, textBytes.size() + 1, position, position_type, end_position, flags, &unique_identifier, this );
        espeak_Synchronize();
     });

    m_pTtsAudioLayer->startTimer(delayMs);
}

void espeaktts::stop() {
    m_pTtsAudioLayer->stop();
    espeak_Cancel();
    m_ttsFuture.waitForFinished();
}

void espeaktts::onBufferReady(short *buf, size_t buffSize) {
    m_pTtsAudioLayer->appendSample((char*)buf, buffSize * 2);
}

bool espeaktts::isFinished() const {
    return m_ttsFuture.isFinished();
}

void espeaktts::setSpeechRate(int nRate) { //Range 50 - 400
    espeak_SetParameter(espeakRATE, nRate, 0);
}

int espeaktts::getSpeechRate() {
    return espeak_GetParameter(espeakRATE, 1);
}

void espeaktts::setVolume(int nVolume) {
    espeak_SetParameter(espeakVOLUME, nVolume, 0);
}

int espeaktts::getVolume() {
    return espeak_GetParameter(espeakVOLUME, 1);
}
