/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#include "cerencetts.h"
#include "cerencettsprivate.h"

CerenceTTS::CerenceTTS(QObject *parent)
    : QObject(parent)
    , p{new CerenceTTSPrivate(this)}
{
}

CerenceTTS::~CerenceTTS()
{
    delete p;
}

void CerenceTTS::say(const QString &text)
{
    p->say(text);
}

void CerenceTTS::stop()
{
    p->stop();
}
