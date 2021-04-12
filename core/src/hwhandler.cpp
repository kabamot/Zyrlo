#include "hwhandler.h"

#include <QtConcurrent>
#include <QDebug>
#include "BaseComm.h"
#include "BTComm.h"
#include <QAudioDeviceInfo>
#include <QMediaPlayer>
#include <QAudioOutputSelectorControl>
#include <QMediaService>

// This is important to receive cv::Mat from another thread
Q_DECLARE_METATYPE(cv::Mat);
Q_DECLARE_METATYPE(Button);

HWHandler::HWHandler(QObject *parent, bool btKeyboardFound)
    : QObject(parent)
    , m_btKeyboardFound(btKeyboardFound)
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
//    const auto deviceInfos = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
//    for (const QAudioDeviceInfo &deviceInfo : deviceInfos)
//        qDebug() << "Device name: " << deviceInfo.deviceName();

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
    if (m_btKeyboardFound && !m_buttonBtThread.isRunning())
        m_buttonBtThread = QtConcurrent::run([this](){ buttonBtThreadRun(); });
 }

void HWHandler::stop()
{
    m_stop = true;
}

void HWHandler::run()
{
    m_zcam.initCamera();
    while(!m_stop) {
        switch(m_zcam.AcquireFrameStep()) {
        case ZyrloCamera::eCameraArmClosed:
            QThread::msleep(100);
            break;
        case ZyrloCamera::eShowPreviewImge:
            emit previewImgUpdate(m_zcam.GetPreviewImg());
            break;
        case ZyrloCamera::eStartOcr:
            emit imageReceived(m_zcam.GetImageForOcr(), true);
            break;
        case ZyrloCamera::eReaderReady:
            emit readerReady();
            break;
        case ZyrloCamera::eTargetNotFound:
            emit targetNotFound();
            break;
        case ZyrloCamera::eGestBackSentence:
            emit onGesture(1);
            break;
        case ZyrloCamera::eGestPauseResume:
            emit onGesture(2);
            break;
        }
        QThread::msleep(1);
    }
}

void HWHandler::onButtonsDown(byte down_val) {
    qDebug() << "onButtonsDown " << down_val << '\n';
    emit onButton(down_val, true);
}

void HWHandler::onButtonsUp(byte up_val) {
    qDebug() << "onButtonsUp " << up_val << '\n';
    emit onButton(up_val, false);
}

void HWHandler::buttonBtThreadRun() {
     int nVal;
    m_btc.init();
    for(;!m_stop; QThread::msleep(100)) {
        m_btc.btConnect(m_stop);
        bool bCont = true;
        for(; !m_stop && bCont; QThread::msleep(20)) {

            switch(m_btc.receiveLoopStep(nVal)) {
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
    byte reply[2], xor_val, up_val, down_val;
    BaseComm bc;
    int nBatteryCheck = 40, nBatteryCheckCount = 0;
    bc.init();
    bc.sendCommand(I2C_COMMAND_OTHER_BOOT_COMPLETE | I2C_COMMAND_OTHER, reply);
    for(; !m_stop; QThread::msleep(50)) {
        if(bc.sendCommand(I2C_COMMAND_GET_KEY_STATUS, reply) != 0) {
            qDebug() << "BaseComm error" << reply[0] << reply[1] << Qt::endl;
            continue;
        }
        m_nButtonMask = reply[0];
        if((SWITCH_FOLDED_MASK | m_nButtonMask) != 0)
            onButtonsUp(SWITCH_FOLDED_MASK_UP);
        break;
    }
    for(; !m_stop; QThread::msleep(50), --nBatteryCheckCount) {
        if(nBatteryCheckCount <= 0 && bc.sendCommand(I2C_COMMAND_GET_BATTERY, reply, false) == 0) {
            m_battery =  (m_battery < 0) ? float(reply[1]) : m_battery * 0.9f + float(reply[1]) * 0.1f;
            qDebug() << "Battery" << m_battery <<Qt::endl;
            nBatteryCheckCount = nBatteryCheck;
        }
        if(bc.sendCommand(I2C_COMMAND_GET_KEY_STATUS, reply) != 0) {
            qDebug() << "BaseComm error\n";
            continue;
        }
        xor_val = m_nButtonMask ^ reply[0];
        if(xor_val != 0) {
            down_val = reply[1] & xor_val;
            if(down_val != 0)
                onButtonsDown(down_val);
            up_val = m_nButtonMask & xor_val;
            if(up_val != 0)
                onButtonsUp(up_val);
            m_nButtonMask = reply[0];
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

static string GetSavedImgePath(int indx) {
    char sPath[256];
    sprintf(sPath, "/home/pi/RawImage_%d.bmp", indx);
   return sPath;
}

void HWHandler::saveImage(int indx) {
     imwrite(GetSavedImgePath(indx), m_zcam.GetFullResRawImg(0));
}

bool HWHandler::recallSavedImage(int indx) {
    m_recallImg = imread(GetSavedImgePath(indx), cv::IMREAD_GRAYSCALE);
    return !m_recallImg.empty();
}

void HWHandler::readRecallImage() {
    emit imageReceived(m_recallImg, false);
}

const cv::Mat & HWHandler::getRecallImg() const {
    return m_recallImg;
}

bool HWHandler::gesturesOn() const {
    return m_zcam.gesturesOn();
}

void HWHandler::setGesturesUi(bool bOn) {
    m_zcam.setGesturesUi(bOn);
}

void HWHandler::setCameraArmPosition(bool bOpen) {
    m_zcam.setArmPosition(bOpen);
}

void HWHandler::onSpeakingStarted() {
    qDebug() << "HWHandler::onSpeakingStarted\n";
    m_btc.ConnectLock();
}

void HWHandler::UnlockBtConnect() {
    qDebug() << "HWHandler::onSpeakingFinished\n";
    m_btc.ConnectUlnock();
}
