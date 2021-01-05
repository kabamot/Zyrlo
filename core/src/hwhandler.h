#pragma once

#include <QObject>
#include <QFuture>
#include <atomic>
#include "zyrlocamera.h"

namespace cv {
    class Mat;
}

enum class Button {
    Up,
    Down,
    PgUp,
    PgDown,
};

class HWHandler : public QObject
{
    Q_OBJECT
public:
    explicit HWHandler(QObject *parent = nullptr);
    ~HWHandler() override;

    void start();
    void stop();

    void run();
    void buttonThreadRun();
    void snapImage();
    void flashLed();
    void setLed(bool bOn);
    void onButtonsDown(unsigned char down_val);
    void onButtonsUp(unsigned char up_val);

signals:
    void imageReceived(const cv::Mat &image);
    void buttonReceived(Button button);
    void previewImgUpdate(const cv::Mat &prevImg);

private:
    std::atomic_bool    m_stop {false};
    QFuture<void>       m_future, m_buttonThread;
    ZyrloCamera m_zcam;
    unsigned char m_nButtonMask = 0x40;
};
