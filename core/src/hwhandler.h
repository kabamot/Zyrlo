#pragma once

#include <QObject>
#include <QFuture>
#include <atomic>
#include "zyrlocamera.h"
#include "BTComm.h"
#include <opencv2/opencv.hpp>

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
    explicit HWHandler(QObject *parent, bool btKeyboardFound);
    ~HWHandler() override;

    void start();
    void stop();

    void run();
    void buttonThreadRun();
    void buttonBtThreadRun();
    void snapImage();
    void flashLed(int msecs);
    void setLed(bool bOn);
    void onButtonsDown(unsigned char down_val);
    void onButtonsUp(unsigned char up_val);
    void saveImage(int indx);
    bool recallSavedImage(int indx);
    void readRecallImage();
    const cv::Mat & getRecallImg() const;
    bool gesturesOn() const;
    void setGesturesUi(bool bOn);
    void setCameraArmPosition(bool bOpen);
    void UnlockBtConnect();

public slots:
    void onSpeakingStarted();

signals:
    void imageReceived(const cv::Mat &image, bool bPlayShutterSound);
    void buttonReceived(Button button);
    void previewImgUpdate(const cv::Mat &prevImg);
    void readerReady();
    void targetNotFound();
    void onBtButton(int nButton, bool bDown);
    void onButton(int nButton, bool bDown);
    void onBtBattery(int nVal);
    void onGesture(int nGesture);

private:
    std::atomic_bool    m_stop {false};
    QFuture<void>       m_future, m_buttonThread, m_buttonBtThread;
    ZyrloCamera m_zcam;
    int m_nButtonMask = -1; //0x40;
    cv::Mat m_recallImg;
    BTComm m_btc;
    bool m_btKeyboardFound = false;
};
