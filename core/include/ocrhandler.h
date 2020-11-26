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

class OcrHandler : public QObject
{
    Q_OBJECT
public:
    explicit OcrHandler(QObject *parent = nullptr);
    ~OcrHandler();

    bool setLanguage(int languageCode);
    bool startProcess(const cv::Mat &image);

signals:

private:
    int m_languageCode {1};
};

