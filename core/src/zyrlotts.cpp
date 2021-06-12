#include "zyrlotts.h"
#include "ttsaudiolayer.h"

ZyrloTts::ZyrloTts(QObject *parent, TtsAudioLayer *pTtsAudioLayer)
    : QObject(parent)
    , m_pTtsAudioLayer(pTtsAudioLayer)
{

}

void ZyrloTts::pause() {
    m_pTtsAudioLayer->suspend();
}

void ZyrloTts::resume() {
    m_pTtsAudioLayer->resume();
}

bool ZyrloTts::isSpeaking() const {
    return m_pTtsAudioLayer->state() == QAudio::ActiveState;
}

bool ZyrloTts::isPaused() const {
    return m_pTtsAudioLayer->state() == QAudio::SuspendedState;
}

bool ZyrloTts::isStoppedSpeaking() const {
    return m_pTtsAudioLayer->state() == QAudio::StoppedState;
}

void ZyrloTts::disconnectFromAudioLayer() {
    disconnect(m_connectNotify);
    disconnect(m_connectChanged);
}

void ZyrloTts::sayAfter(const QString &text) {
    m_messageQueMutex.lock();
    if(isSpeaking() || !m_messageQue.empty())
        m_messageQue.push_back(text);
    else
        say(text);
    m_messageQueMutex.unlock();
}

void ZyrloTts::connectToAudioLayer() {
    m_connectNotify = connect(m_pTtsAudioLayer, &TtsAudioLayer::notify, this, [this](){
        const auto elapsedSamples = m_pTtsAudioLayer->processedUSecs() *
                m_pTtsAudioLayer->format().sampleRate() / 1'000'000;
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

    m_connectChanged = connect(m_pTtsAudioLayer, &QAudioOutput::stateChanged, this, [this](QAudio::State state) {
        //qDebug() << "state" << state;
        switch (state) {
        case QAudio::ActiveState:
            emit sayStarted();
            break;

        case QAudio::StoppedState:
            if (m_pTtsAudioLayer->error() != QAudio::NoError) {
                // Error handling
                qWarning() << __func__ << __LINE__ << m_pTtsAudioLayer->error();
            }
            break;

        case QAudio::IdleState:
            if (m_ttsFuture.isFinished()) {
                m_pTtsAudioLayer->stop();
                emit sayFinished();
                m_messageQueMutex.lock();
                if(m_messageQue.empty()) {
                    m_messageQueMutex.unlock();
                }
                else {
                    QString text = m_messageQue.front();
                    m_messageQue.pop_front();
                    m_messageQueMutex.unlock();
                    say(text);
                }

            }
            break;

        default:
            break;
        }
    });
}

bool ZyrloTts::writeToWave(const char *sFileName) {
    bool bret = m_pTtsAudioLayer->writeToWave(sFileName);
    m_bOutputToFile = false;
    return bret;
}

bool ZyrloTts::writeToMp3(const char *sFileName) {
    bool bret = m_pTtsAudioLayer->writeToMp3(sFileName);
    m_bOutputToFile = false;
    return bret;
}

void ZyrloTts::convertTextToAudio(const QString & sText, const QString & sAudioFileName) {
    m_bOutputToFile = true;
    m_audioOutFileName = sAudioFileName;
    say(sText);
}
