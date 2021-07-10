#ifndef USBCOMM_H
#define USBCOMM_H

#include "Buttons.h"
#include <string>

class USBComm
{
    static std::string DEVICE_NAME;

    int m_handle = -1;
    char m_serial[18] = {0}, m_version[5] = {0}, n_batteryInfo[5] = {0};
    void FillSerialNumber(const char *buff);

public:
    USBComm();
    ~USBComm();
    bool Init();
    int Receive(int & nval);
    std::string getSerial() const {return m_serial;}
    static void setDeviceName(const std::string & deviceName) {DEVICE_NAME = deviceName;}
};

#endif // USBCOMM_H
