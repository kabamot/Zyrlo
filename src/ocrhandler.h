/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Nov 2020
 *
 ****************************************************************************/

#pragma once

#include <QObject>

class OcrHandler : public QObject
{
    Q_OBJECT
public:
    explicit OcrHandler(QObject *parent = nullptr);

signals:

private:
};

