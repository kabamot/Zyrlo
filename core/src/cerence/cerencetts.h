/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#pragma once

#include <QObject>

class CerenceTTSPrivate;

class CerenceTTS : public QObject
{
    Q_OBJECT
public:
    CerenceTTS(QObject *parent = nullptr);
    ~CerenceTTS() override;

public slots:
    void say(const QString &text);
    void stop();

private:
    CerenceTTSPrivate *p;
};
