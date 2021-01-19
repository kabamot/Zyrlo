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
#include "textpage.h"

constexpr auto DATA_DIR = "/opt/zyrlo/Distrib";
constexpr int STATUS_MAX_SIZE = 64;

OcrHandler::OcrHandler()
{
    const auto retCode = zyrlo_proc_init(DATA_DIR, m_languageCode);
    if (retCode != 0) {
        qWarning() << "Error in zyrlo_proc_init()" << retCode;
    }

    while (!isIdle()) {
        QThread::msleep(1);
    }

    m_timer.setInterval(10);
    connect(&m_timer, &QTimer::timeout, this, &OcrHandler::checkProcess);
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
    stopProcess();

    if (image.empty()) {
        qWarning() << "Image is empty";
        emit finished();
        return false;
    }
    imwrite("OrcrImg.bmp", image);
    const auto retCode = zyrlo_proc_start_with_bayer(image);
    if (retCode == 0) {
        m_timer.start();
    } else {
        qWarning() << "Error in zyrlo_proc_start_with_bayer()" << retCode;
    }

    return retCode == 0;
}

bool OcrHandler::stopProcess()
{
    if (!isIdle()) {
        zyrlo_proc_cancel();
        while (!isIdle()) {
            QThread::msleep(1);
        }
    }

    m_timer.stop();
    destroyTextPage();
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

int OcrHandler::processingParagraphNum() const
{
    return m_processingParagraphNum;
}

const TextPage *OcrHandler::textPage() const
{
    return m_page;
}

void OcrHandler::checkProcess()
{
    if (isIdle()) {
        m_timer.stop();
        emit finished();
    }

    if (isOcring()) {
        if (m_page == nullptr) {
            createTextPage();
        }

        if (getOcrResults()) {
            emit lineAdded();
        }
    }
}

QString OcrHandler::getStatus() const
{
    char buffer[STATUS_MAX_SIZE];
    zyrlo_proc_get_status(buffer);
    return QString(buffer);
}

void OcrHandler::createTextPage()
{
    if (m_page) {
        destroyTextPage();
    }

    const auto numParagraphs = zyrlo_proc_get_num_paragraphs();
    m_page = new TextPage(numParagraphs);

    for (int i = 0; i < numParagraphs; ++i) {
        const auto numLines = zyrlo_proc_get_num_lines(i);
        if (numLines <= 0)
            continue;
        Q_ASSERT(numLines > 0);
        m_page->paragraph(i).setId(i);
        m_page->paragraph(i).setNumLines(numLines);
    }
}

void OcrHandler::destroyTextPage()
{
    if (m_page) {
        delete m_page;
        m_page = nullptr;
    }
}

bool OcrHandler::getOcrResults()
{
    bool hasNewResult = false;
    text_line textLine;
    int resultsCode = 0;
    while (resultsCode == 0) {
        resultsCode = zyrlo_proc_get_result(&textLine);
        if (resultsCode == 0) {
            m_page->paragraph(textLine.nParagraphId).addLine(textLine.sText);
            hasNewResult = true;
            m_processingParagraphNum = textLine.nParagraphId;
        }
    }

    return hasNewResult;
}
