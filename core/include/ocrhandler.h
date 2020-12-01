/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Nov 2020
 *
 ****************************************************************************/

#pragma once

#include <QObject>

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

signals:

private:
    explicit OcrHandler();

private:
    int m_languageCode {1};
};

