/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#pragma once

#include <QTextToSpeechEngine>

class CerenceTTSEngine : public QTextToSpeechEngine
{
    Q_OBJECT
public:
    CerenceTTSEngine(QObject *parent = nullptr);

    QVector<QLocale> availableLocales() const override;
    QVector<QVoice> availableVoices() const override;
    QLocale locale() const override;
    void pause() override;
    double pitch() const override;
    double rate() const override;
    void resume() override;
    void say(const QString &text) override;
    bool setLocale(const QLocale &locale) override;
    bool setPitch(double pitch) override;
    bool setRate(double rate) override;
    bool setVoice(const QVoice &voice) override;
    bool setVolume(double volume) override;
    QTextToSpeech::State state() const override;
    void stop() override;
    QVoice voice() const override;
    double volume() const override;
};
