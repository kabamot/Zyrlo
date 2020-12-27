/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Nov 2020
 *
 ****************************************************************************/

#pragma once

#include <QObject>

class OcrHandler;
class Paragraph;
class CerenceTTS;

class MainController : public QObject
{
    Q_OBJECT
public:
    MainController();

    void start(const QString &filename);

signals:
    void textUpdated(const QString &text);
    void formattedTextUpdated(const QString &text);
//    void paragraphUpdated(const Paragrah &paragraph);
    void finished();

private:
    OcrHandler &ocr();
    CerenceTTS *m_ttsEngine {nullptr};
};

