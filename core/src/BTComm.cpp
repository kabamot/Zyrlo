
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "BTComm.h"
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <errno.h>
#include <QDebug>

#define LOGI(x) qDebug() << x

#define KEYPAD_CONFIG_FILE	"/home/pi/keypad_config.txt"
const char RFCOMM_FILE[] =	"/dev/rfcomm0";





int BTComm::init()
{
	int iRet;
	FILE *fp = fopen(KEYPAD_CONFIG_FILE, "r");
	if(fp == NULL) {
		LOGI("Keypad config file not found\n");
		m_eStatus = eStatusOff;
		return -1;
	}

	iRet = fread(keypadMacStr, 1, 17, fp);
	if(iRet != 17) {
		LOGI("Keypad config file error\n");
		m_eStatus = eStatusOff;
		return -1;
	}
	keypadMacStr[17] = '\0';
	fclose(fp);

	m_exitRequest = 0;
	m_eStatus = eStatusIdle;

    m_eStatus = eStatusDisconnected;

	return 0;
}

int readByteFromSocket(int sd, unsigned char *val)
{
	int n;
//printf(".\n");
	int x = fcntl(sd, F_GETFL, 0);
	fcntl(sd, F_SETFL, x | O_NONBLOCK);
	if ( (n = read (sd, val, 2)) < 0) {
			if(errno == EWOULDBLOCK)
				return 0;
			else {
				printf("read error. Not WOULDBLOCK\n");
				return -1;
			}
	}
	return n;
}

int BTComm::receiveLoopStep(int & nVal)
{
    int status = readByteFromSocket(m_s, &m_readBuffer[0]);
    if(status < 0) {
        close(m_s);
        m_s = -1;
        m_eStatus = eStatusDisconnected;
        return -1;
    }
    if(status > 0) {
           qDebug() << "Received: " <<  status << m_readBuffer[0] << m_readBuffer[1] << "\n";
           nVal = m_readBuffer[1];
           return (int)m_readBuffer[0];
    }
    return 0;
}

int BTComm::btConnect(const std::atomic_bool &isStop) {
    int status;
    qDebug() << "btConnect 0\n";
    for(; !isStop;  sleep(1)) {
        if(m_bConnectLock) {
             continue;
        }
        m_s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
        m_addr.rc_family = AF_BLUETOOTH;
        m_addr.rc_channel = (uint8_t) 1;
        str2ba( keypadMacStr, &m_addr.rc_bdaddr );
        status = connect(m_s, (struct sockaddr *)&m_addr, sizeof(m_addr));
        if( status < 0 )
            perror("Connect err: ");
        else {
            qDebug() << "Connected\n";
            m_eStatus = eStatusConnected;
            break;
        }
        close(m_s);
        m_s = -1;
    }
    return status;
}
