
#include <wiringPi.h>
#include <wiringPiI2C.h>

#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <QDebug>
#include "BaseComm.h"



int BaseComm::init()
{
    wiringPiSetup () ;
    m_fdI2C = wiringPiI2CSetup(0x50);
    if(m_fdI2C < 0) {
        LogError("wiringPiI2CSetup error %d\n", errno);
        return -1;
    }

    m_nSequence = 0;
    setSpeakerSetting(4);
    return 0;
}

int BaseComm::sendCommand(byte pCommand, byte *pReply, bool bCheckEquality)
{
    byte bufferIn[16];
    byte bufferOut[16];
    int retVal;

    m_nSequence ++;
    if(m_nSequence == 16)
        m_nSequence = 0;

    bufferOut[0] = pCommand;
    retVal = write(m_fdI2C, (const void *)bufferOut, 1);
    if(retVal != 1) {
        qDebug() << "I2C write error\n";
        return -1;
    }
    usleep(10000);
    retVal = read(m_fdI2C, (void *)bufferIn, 2);
    if(retVal < 0) {
        qDebug() << "I2C read error %d %s\n", errno, strerror(errno);
        return -1;
    }
    if(retVal < 2) {
        usleep(10000);
        qDebug() <<  "I2C read less then 2 bytes. Re-trying\n";
        int bytesLeft = 2-retVal;
        retVal = read(m_fdI2C, (void *)(&bufferIn[retVal]), bytesLeft);
        if(retVal != bytesLeft) {
            qDebug() << "I2C read error. Not enough bytes read.\n";
            return -1;
        }
    }
    //qDebug() << (int)bufferIn[1] << "    " << (int)bufferIn[0] << '\n';
    if(bCheckEquality && bufferIn[1] != bufferIn[0]) {
        LogError("I2C read data error %x %x     %x %x\n", bufferOut[0], bufferIn[0], bufferOut[0], bufferIn[0] );
        usleep(100000);
        retVal = read(m_fdI2C, (void *)bufferIn, 2);
        if(retVal < 0)
            LogError("I2C read error on buffer clean up %d %s\n", errno, strerror(errno));
        if(retVal > 0)
            LogError("I2C read clean up, %d     %x %x\n", retVal, bufferIn[0], bufferIn[1]);

        return -1;
    }

    memcpy(pReply, bufferIn, 2);
    return 0;
}

int BaseComm::flushInput()
{
    byte bufferIn[1024];
    int iCnt = 0;

    int retVal = read(m_fdI2C, (void *)bufferIn, 2);
    while(retVal > 0 && iCnt < 10) {
        printf("keep flushing retVal =%d\n", retVal);
        usleep(10);
        iCnt++;
        retVal = read(m_fdI2C, (void *)bufferIn, 2);
    }
    if(retVal < 0) {
        printf("retVal = %d\n", retVal);
        LogError("I2C read error while flushing %d %s\n", errno, strerror(errno));
        return -1;
    }

    return 0;
}

int printRegs(int fdI2C)
{
    byte bufferIn[16];
    byte bufferOut[16];
    char msg[128];
    int retVal;

    bufferOut[0] = 0x1;
    retVal = write(fdI2C, (const void *)bufferOut, 1);
    if(retVal != 1) {
        qDebug() << "I2C write error" << retVal;
        return -1;
    }

    retVal = read(fdI2C, (void *)bufferIn, 7);
    qDebug() << "Read bytes" << retVal;
    if(retVal != 7)
        return -1;

    for(int ind=0; ind<retVal; ind++) {
        sprintf(msg, "0x%x  ", bufferIn[ind]);
        qDebug() << msg;
    }
    return 0;
}

int BaseComm::setSpeakerSetting(int nSetting) {
    byte bufferOut[16];
    int fdI2C = wiringPiI2CSetup(0x58);
    switch(nSetting) {
    case 1:
        bufferOut[0] = 0x1;
        bufferOut[1] = 0xc3;
        bufferOut[2] = 0x05;
        bufferOut[3] = 0xB;
        bufferOut[4] = 0;
        bufferOut[5] = 0x6;
        bufferOut[6] = 0x3A;
        bufferOut[7] = 0xc3;
        break;
    case 2:
        bufferOut[0] = 0x1;
        bufferOut[1] = 0xc3;
        bufferOut[2] = 0x3f;
        bufferOut[3] = 0x3f;
        bufferOut[4] = 0x3f;
        bufferOut[5] = 0x6;
        bufferOut[6] = 0x6;
        bufferOut[7] = 0xc2;
        break;
    case 3:
        bufferOut[0] = 0x1;
        bufferOut[1] = 0xc3;
        bufferOut[2] = 0x1;
        bufferOut[3] = 0x1;
        bufferOut[4] = 0x0;
        bufferOut[5] = 0x6;
        bufferOut[6] = 0x1f;
        bufferOut[7] = 0xc2;
        break;
    case 4:
        bufferOut[0] = 0x1;
        bufferOut[1] = 0xc3;
        bufferOut[2] = 0x3f;
        bufferOut[3] = 0x3f;
        bufferOut[4] = 0x3f;
        bufferOut[5] = 0x16;
        bufferOut[6] = 0x10;
        bufferOut[7] = 0xc3;
        break;
    case 5:
        bufferOut[0] = 0x1;
        bufferOut[1] = 0xc3;
        bufferOut[2] = 0x3f;
        bufferOut[3] = 0x3f;
        bufferOut[4] = 0x3f;
        bufferOut[5] = 0x16;
        bufferOut[6] = 0x1f;
        bufferOut[7] = 0x23;
        break;
    default:
        qDebug() << "setSpeakerSetting wrong arg";
    }
    int retVal = write(fdI2C, (const void *)bufferOut, 8);
    if(retVal != 8) {
        printf("I2C write error %d\n", retVal);
        return -1;
    }
    printRegs(fdI2C);
    return 0;
}


