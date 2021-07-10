#include "usbcomm.h"

#include <stdio.h>
#include <wiringSerial.h>
#include <QDebug>

using namespace std;

std::string USBComm::DEVICE_NAME;

USBComm::USBComm() {

}

bool USBComm::Init() {
    m_handle = serialOpen (DEVICE_NAME.c_str(), 115200);
    if (m_handle < 0)
        return false;
    return true;
}

USBComm::~USBComm() {
    if(m_handle > 0)
        serialClose(m_handle);
}

void USBComm::FillSerialNumber(const char *buff) {
    for(int i = 0; i != 6; ++i) {
        int num = (buff[i * 3] - '0') * 100 + (buff[i * 3 + 1] - '0') * 10 + buff[i * 3 + 2] - '0';
        sprintf(m_serial + i * 3, "%02x", num);
        if(i != 5)
            sprintf(m_serial + i * 3 + 2, ":");
    }
}

int USBComm::Receive(int & nVal) {
    int ch = serialGetchar(m_handle) - '0', i;
    if(ch < 0)
        return -1;
    char vals[20];
    //qDebug() << ch;
    switch(ch) {
    case KP_SERIAL_INFO:
        for(i = 0; i != 20; ++i)
            vals[i] = serialGetchar(m_handle);
        if(m_serial[0] == 0)
            FillSerialNumber(vals);
        if(vals[19] != '\n')
            qDebug() << "serial mismatch";
        return ch;
    case KP_VERSION_INFO:
        for(i = 0; i != 5; ++i)
            m_version[i] = serialGetchar(m_handle);
        if(m_version[4] != '\n')
            qDebug() << "key serial mismatch";
        return ch;
    case KP_BATTERY_INFO:
        for(i = 0; i != 5; ++i)
            n_batteryInfo[i] = serialGetchar(m_handle);
        if(n_batteryInfo[4] != '\n')
            qDebug() << "key serial mismatch";
        return ch;
    default:
        for(i = 0; i != 5; ++i)
            vals[i] = serialGetchar(m_handle);
        nVal = (vals[1] - '0') * 10 + (vals[2] - '0');
        if(vals[4] != '\n')
            qDebug() << "key serial mismatch";
        return ch;
    }
    return 0;
}
