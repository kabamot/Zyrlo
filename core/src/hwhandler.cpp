#include "hwhandler.h"

#include <QtConcurrent>
#include <QDebug>
#include <QAudioDeviceInfo>
#include <QMediaPlayer>
#include <QAudioOutputSelectorControl>
#include <QMediaService>
#include <sys/types.h>
#include <dirent.h>

// This is important to receive cv::Mat from another thread
Q_DECLARE_METATYPE(cv::Mat);
Q_DECLARE_METATYPE(Button);

HWHandler::HWHandler(QObject *parent)
    : QObject(parent)
    , m_btc((MainController*)parent)
{
    // This is important to receive cv::Mat from another thread
    qRegisterMetaType<cv::Mat>();
    qRegisterMetaType<Button>();
}

HWHandler::~HWHandler() {
    stop();
    m_future.waitForFinished();
    if(m_btKeyboardFound)
        m_buttonBtThread.waitForFinished();
    m_buttonThread.waitForFinished();
}

bool HWHandler::init() {
    byte reply[2];
    return m_bc.init() == 0
            && m_bc.sendCommand(I2C_COMMAND_OTHER_BOOT_COMPLETE | I2C_COMMAND_OTHER, reply) == 0;
}

void HWHandler::start() {
    QMediaPlayer player;
    QMediaService *svc = player.service();
    if (svc != nullptr) {
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
    if(!(m_btKeyboardFound = (m_btc.init() == 0)))
        return;
    QThread::sleep(5);
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
                //qDebug() << "onBattery " << nVal << '\n';
                emit onBtBattery(nVal);
                break;
            case -1:
                bCont = false;
                qDebug() << "BT Disconnected\n";
                break;
            }
        }
    }
}

void HWHandler::ReadSnAndVersion(BaseComm & bc) {
    byte reply[2];
    int res[] = {-1, -1, -1};
    unsigned char commands[] = {I2C_COMMAND_OTHER_GET_VERSION, I2C_COMMAND_OTHER_GET_SERIAL1, I2C_COMMAND_OTHER_GET_SERIAL2};

    for(int i = 0; i != 3; ++i) {
        for(; !m_stop && res[i] < 0; QThread::msleep(50)) {
            if(bc.sendCommand(I2C_COMMAND_OTHER | commands[i], reply, false) != 0) {
                qDebug() << "BaseComm error" << reply[0] << reply[1] << Qt::endl;
                continue;
            }
            res[i] = reply[1];
        }
    }
    m_nSN = res[1] * 256 + res[2];
    m_nVersion = res[0];

}

bool usbKeyInserted() {
    DIR *fd = opendir("/dev");
    if(!fd)
        return false;
    bool bret = false;
    struct dirent *dp;
    while ((dp = readdir (fd))) {
        if(strncmp(dp->d_name, "sd", 2) == 0) {
            bret = true;
            break;
        }
    }
    closedir(fd);
    return bret;
}

void HWHandler::buttonThreadRun() {
    qDebug() << "buttonThreadRun STARTED\b";
    byte reply[2], xor_val, up_val, down_val;
    int nBatteryCheck = 40, nBatteryCheckCount = 0;
    m_bc.sendCommand(I2C_COMMAND_OTHER_BOOT_COMPLETE | I2C_COMMAND_OTHER, reply); // in case it didn't stop bepping in the first time
    ReadSnAndVersion(m_bc);
    for(; !m_stop; QThread::msleep(50)) {
        if(m_bc.sendCommand(I2C_COMMAND_GET_KEY_STATUS, reply) != 0) {
            qDebug() << "BaseComm error" << reply[0] << reply[1] << Qt::endl;
            continue;
        }
        qDebug() << "BUTTTON" << reply[0] << reply[0] << Qt::endl;
        m_nButtonMask = reply[0];
        if((SWITCH_FOLDED_MASK & m_nButtonMask) != 0) {
            m_zcam.setArmPosition(true);
            break;
        }
    }
    setLed(true);
    for(; !m_stop; QThread::msleep(50), --nBatteryCheckCount) {
        if(nBatteryCheckCount <= 0 && m_bc.sendCommand(I2C_COMMAND_GET_BATTERY, reply, false) == 0) {
            m_battery =  (m_battery < 0) ? float(reply[1]) : m_battery * 0.9f + float(reply[1]) * 0.1f;
            //qDebug() << "Battery" << m_battery <<Qt::endl;
            nBatteryCheckCount = nBatteryCheck;
            bool bUsbKeyInserted = usbKeyInserted();
            if(bUsbKeyInserted != m_bUsbKeyInserted) {
                m_bUsbKeyInserted = bUsbKeyInserted;
                emit usbKeyInsert(m_bUsbKeyInserted);
            }
        }
        if(m_bc.sendCommand(I2C_COMMAND_GET_KEY_STATUS, reply) != 0) {
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

bool HWHandler::ChangeCameraExposure(int delta) {

    int nExp = m_zcam.getExposure();
    qDebug() << "Exposure = " << nExp;
    m_zcam.setAutoExposure(false);
    return m_zcam.setExposure(nExp + delta) == 0;
}

bool HWHandler::ChangeCameraExposureStep(int delta) {
    float fStep = m_zcam.getExposureStep();
    m_zcam.setExposureStep(fStep + float(delta));
    float fExp = m_zcam.getPreviewExposure();
    m_zcam.setEffectiveExposure(fExp);
    return true;
}
void HWHandler::setIgnoreCameraInputs(bool bIgnore) {
    m_zcam.setIgnoreInputs(bIgnore);
}

void HWHandler::setUseCameraFlash(bool bUseFlash) {
    m_zcam.setUseFlash(bUseFlash);
}

bool HWHandler::getUseCameraFlash() const {
    return m_zcam.getUseFlash();
}

void HWHandler::setExposureStep(float fStep) {
    m_zcam.setExposureStep(fStep);
}

float HWHandler::getExposureStep() const {
    return m_zcam.getExposureStep();
}

bool HWHandler::setSpeakerSetting(int nSetting) {
    return m_bc.setSpeakerSetting(nSetting) == 0;
}

string HWHandler::kpConfig() const {
    return m_btc.kpConfig();
}

void HWHandler::setFullResPreview(bool bOn) {
    m_zcam.setFullResPreview(bOn);
}
