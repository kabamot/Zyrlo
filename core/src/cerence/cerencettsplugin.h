/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#pragma once

#include <QObject>
#include <QTextToSpeechPlugin>

#include <QTextToSpeechEngine>

QT_BEGIN_NAMESPACE

class CerenceTTSPlugin : public QObject, public QTextToSpeechPlugin
{
    Q_OBJECT
    Q_INTERFACES(QTextToSpeechPlugin)
    Q_PLUGIN_METADATA(IID "org.qt-project.qt.speech.tts.plugin/5.0"
                      FILE "cerence_plugin.json")
public:
    QTextToSpeechEngine *createTextToSpeechEngine(const QVariantMap &parameters,
                                                  QObject *parent,
                                                  QString *errorString) const override;
};

QT_END_NAMESPACE
