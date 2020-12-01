/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Nov 2020
 *
 ****************************************************************************/

#include "ocrhandler.h"

#include <QDebug>

#include <ZyrloOcr.h>

constexpr auto DATA_DIR = "/opt/zyrlo/Distrib";

OcrHandler::OcrHandler()
{
    const auto retCode = zyrlo_proc_init(DATA_DIR, m_languageCode);
    if (retCode != 0) {
        qWarning() << "Error in zyrlo_proc_init()" << retCode;
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
    const auto retCode = zyrlo_proc_start_with_bayer(image);
    if (retCode != 0) {
        qWarning() << "Error in zyrlo_proc_start_with_bayer()" << retCode;
    }
    return retCode == 0;
}
