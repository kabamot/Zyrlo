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

signals:
    void imageReceived(const cv::Mat &image);
    void buttonReceived(Button button);
    void previewImgUpdate(const cv::Mat &prevImg);

private:
    std::atomic_bool    m_stop {false};
    QFuture<void>       m_future;
    ZyrloCamera m_zcam;
};
