#include "hwhandler.h"

#include <QtConcurrent>
#include <QDebug>
#include <opencv2/opencv.hpp>

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
}

void HWHandler::start()
{
    m_stop = false;

    m_future = QtConcurrent::run([this](){ run(); });
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

        m_zcam.AcquireImage();
        emit previewImgUpdate(m_zcam.GetPreviewImg());
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
