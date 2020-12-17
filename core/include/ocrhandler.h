/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Nov 2020
 *
 ****************************************************************************/

#pragma once

#include <string_view>
#include <QObject>
#include <QTimer>

class TextPage;

namespace cv {
class Mat;
}

// Sinlgleton instance
class OcrHandler : public QObject
{
    Q_OBJECT
public:
    static OcrHandler &instance() {
        static OcrHandler ocr;
        return ocr;
    }
    OcrHandler(OcrHandler const&) = delete;
    void operator=(OcrHandler const&) = delete;
    ~OcrHandler();

    bool setLanguage(int languageCode);

    bool startProcess(const cv::Mat &image);
    bool stopProcess();

    bool isIdle() const;
    bool isOcring() const;

    const TextPage *textPage() const;

signals:
    void lineAdded();
    void finished();

private slots:
    void checkProcess();

private:
    explicit OcrHandler();
    QString getStatus() const;

    void createTextPage();
    void destroyTextPage();
    bool getOcrResults();

private:
    int m_languageCode {1};
    TextPage *m_page {nullptr};
    QTimer m_timer;
};

