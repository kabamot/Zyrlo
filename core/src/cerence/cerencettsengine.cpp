/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#include "cerencettsengine.h"

CerenceTTSEngine::CerenceTTSEngine(QObject *parent)
    : QTextToSpeechEngine(parent)
{

}

QVector<QLocale> CerenceTTSEngine::availableLocales() const
{

}

QVector<QVoice> CerenceTTSEngine::availableVoices() const
{

}

QLocale CerenceTTSEngine::locale() const
{

}

void CerenceTTSEngine::pause()
{

}

double CerenceTTSEngine::pitch() const
{

}

double CerenceTTSEngine::rate() const
{

}

void CerenceTTSEngine::resume()
{

}

void CerenceTTSEngine::say(const QString &text)
{

}

bool CerenceTTSEngine::setLocale(const QLocale &locale)
{

}

bool CerenceTTSEngine::setPitch(double pitch)
{

}

bool CerenceTTSEngine::setRate(double rate)
{

}

bool CerenceTTSEngine::setVoice(const QVoice &voice)
{

}

bool CerenceTTSEngine::setVolume(double volume)
{

}

QTextToSpeech::State CerenceTTSEngine::state() const
{

}

void CerenceTTSEngine::stop()
{

}

QVoice CerenceTTSEngine::voice() const
{

}

double CerenceTTSEngine::volume() const
{

}
