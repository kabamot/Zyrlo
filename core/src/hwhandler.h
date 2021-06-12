#pragma once

#include <QObject>
#include <QFuture>
#include <atomic>
#include "zyrlocamera.h"
#include "BaseComm.h"
#include "BTComm.h"
#include <opencv2/opencv.hpp>

class BaseComm;

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
    explicit HWHandler(QObject *parent);
    ~HWHandler() override;

    bool init();

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
    int getMainBatteryPercent() const { return int(m_battery / 2.55f + 0.5f); }
    bool ChangeCameraExposure(int delta);
    bool ChangeCameraExposureStep(int delta);
    int getSN() const {return m_nSN;}
    int getVersion() const {return m_nVersion;}
    void setIgnoreCameraInputs(bool bIgnore);
    void setUseCameraFlash(bool bUseFlash);
    bool getUseCameraFlash() const;
    bool IsUsbKeyInserted() const { return m_bUsbKeyInserted; }
    bool readKpConfig() { return m_btc.readKpConfig(); }
    void setUsingMainAudioSink(bool bUsingMainAudioSink) { m_btc.setUsingMainAudioSink(bUsingMainAudioSink); }
    void SetLocalLightFreqTest(bool bOn) {m_zcam.SetLocalLightFreqTest(bOn);}
    int getCurrentExposure() const {return m_zcam.getExposure();}
    int getCurrentGain() const {return m_zcam.getGain();}
    void setCurrentGain(int nGain) {m_zcam.setGain(nGain);}
    void setExposureStep(float fStep);
    float getExposureStep() const;
    bool setSpeakerSetting(int nSetting);
    std::string kpConfig() const;

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
    void usbKeyInsert(bool bInserted);

private:
    std::atomic_bool    m_stop {false};
    QFuture<void>       m_future, m_buttonThread, m_buttonBtThread;
    ZyrloCamera m_zcam;
    int m_nButtonMask = -1; //0x40;
    cv::Mat m_recallImg;
    BaseComm m_bc;
    BTComm m_btc;
    bool m_btKeyboardFound = false;
    float m_battery = -1.0f;
    int m_nSN = -1, m_nVersion = -1;
    bool m_bUsbKeyInserted = false;
    void ReadSnAndVersion(BaseComm & bc);
};
