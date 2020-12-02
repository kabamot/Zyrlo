/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Nov 2020
 *
 ****************************************************************************/

#include "ocrhandler.h"

#include <QString>
#include <QDebug>
#include <QThread>

#include <ZyrloOcr.h>

constexpr auto DATA_DIR = "/opt/zyrlo/Distrib";
constexpr int STATUS_MAX_SIZE = 32;

OcrHandler::OcrHandler()
{
    const auto retCode = zyrlo_proc_init(DATA_DIR, m_languageCode);
    if (retCode != 0) {
        qWarning() << "Error in zyrlo_proc_init()" << retCode;
    }

    while (!isIdle()) {
        QThread::msleep(1);
    }
}

OcrHandler::~OcrHandler()
{
    zyrlo_proc_end();
}

bool OcrHandler::setLanguage(int languageCode)
{
    m_languageCode = languageCode;
    const auto retCode = zyrlo_proc_set_ocr_language(m_languageCode);
    if (retCode != 0) {
        qWarning() << "Error in zyrlo_proc_set_ocr_language()" << retCode;
    }
    return retCode == 0;
}

bool OcrHandler::startProcess(const cv::Mat &image)
{
    cancelProcess();

    const auto retCode = zyrlo_proc_start_with_bayer(image);
    if (retCode != 0) {
        qWarning() << "Error in zyrlo_proc_start_with_bayer()" << retCode;
    }
    return retCode == 0;
}

bool OcrHandler::cancelProcess()
{
    if (!isIdle()) {
        zyrlo_proc_cancel();
        while (!isIdle()) {
            QThread::msleep(1);
        }
    }
    return true;
}

bool OcrHandler::isIdle() const
{
    return getStatus() == QStringLiteral("Idle");
}

bool OcrHandler::isOcring() const
{
    return getStatus() == QStringLiteral("Ocring");
}

QString OcrHandler::getStatus() const
{
    char buffer[STATUS_MAX_SIZE];
    zyrlo_proc_get_status(buffer);
    return QString(buffer);
}
