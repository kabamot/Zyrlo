#include "hwhandler.h"

#include <QtConcurrent>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include "BaseComm.h"
#include "BTComm.h"
#include <QAudioDeviceInfo>
#include <QMediaPlayer>
#include <QAudioOutputSelectorControl>
#include <QMediaService>

// This is important to receive cv::Mat from another thread
Q_DECLARE_METATYPE(cv::Mat);
Q_DECLARE_METATYPE(Button);

HWHandler::HWHandler(QObject *parent) : QObject(parent)
{
    // This is important to receive cv::Mat from another thread
    qRegisterMetaType<cv::Mat>();
    qRegisterMetaType<Button>();
}

HWHandler::~HWHandler()
{
    stop();
    m_future.waitForFinished();
    m_buttonBtThread.waitForFinished();
    m_buttonThread.waitForFinished();
}

void HWHandler::start()
{
    const auto deviceInfos = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    for (const QAudioDeviceInfo &deviceInfo : deviceInfos)
        qDebug() << "Device name: " << deviceInfo.deviceName();

    QMediaPlayer player;
    QMediaService *svc = player.service();
    if (svc != nullptr)
    {
        QAudioOutputSelectorControl *out = qobject_cast<QAudioOutputSelectorControl *>
                                           (svc->requestControl(QAudioOutputSelectorControl_iid));
        if (out != nullptr)
        {
            out->setActiveOutput("pulse");
            svc->releaseControl(out);
        }
    }

    m_stop = false;
    if (!m_future.isRunning())
        m_future = QtConcurrent::run([this](){ run(); });
    if (!m_buttonThread.isRunning())
        m_buttonThread = QtConcurrent::run([this](){ buttonThreadRun(); });
    if (!m_buttonBtThread.isRunning())
        m_buttonBtThread = QtConcurrent::run([this](){ buttonBtThreadRun(); });
 }

void HWHandler::stop()
{
    m_stop = true;
}

void HWHandler::run()
{
    m_zcam.initCamera();
    int dummyCounter = 0;
    while(!m_stop) {
        // Main stuff here

        switch(m_zcam.AcquireFrameStep()) {
        case ZyrloCamera::eShowPreviewImge:
            emit previewImgUpdate(m_zcam.GetPreviewImg());
            break;
        case ZyrloCamera::eStartOcr:
            emit imageReceived(m_zcam.GetImageForOcr());
            break;
        case ZyrloCamera::eReaderReady:
            emit readerReady();
            break;
        case ZyrloCamera::eTargetNotFound:
            emit targetNotFound();
            break;
        }

        QThread::msleep(1);//300);
        //qDebug() << "This is inside HWHandler main thread" << ++dummyCounter;

        // Image captured from the camera
        //        if (dummyCounter % 10 == 0) {
        //            cv::Mat img;
        //            //emit imageReceived(img);
        //        }

        // Button event received
        //        if (dummyCounter % 11 == 0) {
        //            Button button = Button::Down;
        //            emit buttonReceived(button);
        //        }
    }
}

void HWHandler::onButtonsDown(byte down_val) {
    qDebug() << "onButtonsDown " << down_val << '\n';
}

void HWHandler::onButtonsUp(byte up_val) {
    qDebug() << "onButtonsUp " << up_val << '\n';
}

void HWHandler::buttonBtThreadRun() {
    BTComm btc;
    int nVal;
    btc.init();
    for(;!m_stop; QThread::msleep(100)) {
        btc.btConnect();
        bool bCont = true;
        for(; !m_stop && bCont; QThread::msleep(100)) {
            switch(btc.receiveLoopStep(nVal)) {
            case 0:
                break;
            case 1:
                qDebug() << "onButtonsDown " << nVal << '\n';
                emit onBtButton(nVal, true);
                break;
            case 2:
                qDebug() << "onButtonsUp " << nVal << '\n';
                emit onBtButton(nVal, false);
                break;
            case 3:
                qDebug() << "onBattery " << nVal << '\n';
                emit onBtBattery(nVal);
                break;
            case -1:
                bCont = false;
                break;
            }
        }
    }
}

void HWHandler::buttonThreadRun() {
    byte reply, xor_val, up_val, down_val;
    BaseComm bc;
    bc.init();
    bc.sendCommand(I2C_COMMAND_OTHER_BOOT_COMPLETE | I2C_COMMAND_OTHER, &reply);
    for(; !m_stop; QThread::msleep(50)) {
        if(bc.sendCommand(I2C_COMMAND_GET_KEY_STATUS, &reply) != 0) {
            qDebug() << "BaseComm error\n";
            continue;
        }
        xor_val = m_nButtonMask ^ reply;
        if(xor_val != 0) {
            down_val = reply & xor_val;
            if(down_val != 0)
                onButtonsDown(down_val);
            up_val = m_nButtonMask & xor_val;
            if(up_val != 0)
                onButtonsUp(up_val);
            m_nButtonMask = reply;
        }
    }
}

void HWHandler::snapImage() {
    m_zcam.snapImage();
}

void HWHandler::flashLed(int msecs) {
    m_zcam.flashLed(msecs);
}

void HWHandler::setLed(bool bOn) {
     m_zcam.setLed(bOn);
}
