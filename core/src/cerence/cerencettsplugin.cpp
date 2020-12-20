/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#include "cerencettsplugin.h"
#include "cerencettsengine.h"

QTextToSpeechEngine *
CerenceTTSPlugin::createTextToSpeechEngine(const QVariantMap &parameters,
                                           QObject *parent,
                                           QString *errorString) const
{
    auto cerence = new CerenceTTSEngine();
    return cerence;
}
