
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

    int m_nSequence = 0;
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




