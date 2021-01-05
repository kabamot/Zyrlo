#include "hwhandler.h"

#include <QtConcurrent>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include "BaseComm.h"

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
    m_buttonThread.waitForFinished();
}

void HWHandler::start()
{
    m_stop = false;
    if (!m_future.isRunning())
        m_future = QtConcurrent::run([this](){ run(); });
    if (!m_buttonThread.isRunning())
        m_buttonThread = QtConcurrent::run([this](){ buttonThreadRun(); });
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
        case 0:
            emit previewImgUpdate(m_zcam.GetPreviewImg());
            break;
        case 1:
            emit imageReceived(m_zcam.GetFullResRawImg());
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

void HWHandler::flashLed() {
    m_zcam.flashLed();
}

void HWHandler::setLed(bool bOn) {
     m_zcam.setLed(bOn);
}
