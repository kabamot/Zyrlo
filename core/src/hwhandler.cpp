#include "hwhandler.h"

#include <QtConcurrent>
#include <QDebug>
#include <opencv2/opencv.hpp>

HWHandler::HWHandler(QObject *parent) : QObject(parent)
{
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
    int dummyCounter = 0;
    while(!m_stop) {
        // Main stuff here
        QThread::msleep(300);
        qDebug() << "This is inside HWHandler main thread" << ++dummyCounter;

        // Image captured from the camera
        if (false) {
            cv::Mat img;
            emit imageReceived(img);
        }

        // Button event received
        if (false) {
            Button button = Button::Down;
            emit buttonReceived(button);
        }
    }
}

