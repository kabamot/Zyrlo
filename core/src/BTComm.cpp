
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

//	iRet = pthread_create(&m_ConnectThread, NULL, BtCommThread, (void *)this);
//	if(iRet) {
//		LOGI("Error: pthread_create camLoopThread\n");
//		return -1;
//	}

	return 0;
}

void * BtCommThread(void *p)
{
	BTComm *btComm = (BTComm *)p;
	btComm->receiveLoop();
	return NULL;
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

int BTComm::btConnect() {
    int status;
    qDebug() << "btConnect 0\n";
    do {
        qDebug() << "btConnect 1\n";
        m_s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

        m_addr.rc_family = AF_BLUETOOTH;
        m_addr.rc_channel = (uint8_t) 1;
        str2ba( keypadMacStr, &m_addr.rc_bdaddr );

        status = connect(m_s, (struct sockaddr *)&m_addr, sizeof(m_addr));
        if( status < 0 ) perror("Connect err: "); else printf("Connected\n");
        sleep(1);

    } while(status < 0);
    qDebug() << "btConnect 2\n";
    m_eStatus = eStatusConnected;
    return status;
}

int BTComm::receiveLoop()
{
	int s, status;
   
	while(!m_exitRequest) {
		do {
			s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
			
            m_addr.rc_family = AF_BLUETOOTH;
        m_addr.rc_channel = (uint8_t) 1;
        str2ba( keypadMacStr, &m_addr.rc_bdaddr );

            status = connect(s, (struct sockaddr *)&m_addr, sizeof(m_addr));
			if( status < 0 ) perror("Connect err: "); else printf("Connected\n");
			sleep(1);

		} while(status < 0);

		m_eStatus = eStatusConnected;
		do {
			if(m_exitRequest) {
				close(s);
				m_eStatus = eStatusOff;
				return 0;
			}
			
            status = readByteFromSocket(s, &m_readBuffer[0]);
			if(status > 0) {
                    //printf("Received: %d  %d %d\n", status, readBuffer[0], readBuffer[1]);
                    qDebug() << "Received: " <<  status << m_readBuffer[0] << m_readBuffer[1] << "\n";
			}
			usleep(100000);
/*
			status = read(s, readBuffer, 1);
			if(status > 0) {
				status = read(s, &readBuffer[1], 1);
				if(status > 0) {
					printf("Received:  %d %d\n", status, readBuffer[0], readBuffer[1]);
				}
*/ 
		} while(status >=0);
		
		perror("Read error: ");
		close(s);
		m_eStatus = eStatusDisconnected;
	}

	m_eStatus = eStatusOff;
	return 0;
}

/*
int main(void)
{
	BTComm btComm;

	btComm.init();
	char ch;
	scanf("%c", &ch);
	btComm.btStop();
	while(1) {
		printf("Status: %d\n", 	(int)btComm.m_eStatus);
		sleep(1);
	}
	return 0;
}
*/
