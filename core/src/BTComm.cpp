
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
#include "maincontroller.h"

#define LOGI(x) qDebug() << x

#define KEYPAD_CONFIG_FILE	"/home/pi/keypad_config.txt"
#define BT_ERROR_FILE "/home/pi/BtError.txt"

bool BTComm::readKpConfig() {
    int iRet;
    FILE *fp = fopen(KEYPAD_CONFIG_FILE, "r");
    if(fp == NULL) {
        LOGI("Keypad config file not found\n");
        m_eStatus = eStatusOff;
        return false;
    }

    iRet = fread(keypadMacStr, 1, 17, fp);
    if(iRet != 17) {
        LOGI("Keypad config file error\n");
        m_eStatus = eStatusOff;
        return false;
    }
    keypadMacStr[17] = '\0';
    fclose(fp);
    return true;
}

int BTComm::init() {
    if(!readKpConfig())
        return -1;
    m_exitRequest = 0;
    m_eStatus = eStatusIdle;
    m_eStatus = eStatusDisconnected;
    return 0;
}

int readByteFromSocket(int sd, unsigned char *val) {
	int n;
	int x = fcntl(sd, F_GETFL, 0);
    fcntl(sd, F_SETFL, x | O_NONBLOCK);
    if ( (n = read (sd, val, 2)) < 0) {
        if(errno == EWOULDBLOCK)
            return 0;
        printf("read error. Not WOULDBLOCK\n");
        return -1;
    }
    if(n == 1) {
        x = fcntl(sd, F_GETFL, 0);
        fcntl(sd, F_SETFL, x | O_NONBLOCK);
        n+= read (sd, val + 1, 1);
    }
    return n;
}

int BTComm::receiveLoopStep(int & nVal) {
    int status = readByteFromSocket(m_s, &m_readBuffer[0]);
    if(status < 0) {
        close(m_s);
        m_s = -1;
        m_eStatus = eStatusDisconnected;
        return -1;
    }
    if(status == 2) {
           qDebug() << "ReceivedKP: " << m_readBuffer[0] << m_readBuffer[1] << "\n";
           nVal = m_readBuffer[1];
           return (int)m_readBuffer[0];
    }
    return 0;
}

int BTComm::btConnect(const std::atomic_bool &isStop) {
    int status = 0;
    for(; !isStop;  sleep(1)) {
        if((m_pMainController->isSpeaking()  || m_pMainController->isPlayingSound()) && !m_bUsingMainAudioSink) {
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

bool IsBtRunning() {
    FILE *fp;
    char path[256] = {0};

    fp = popen("hciconfig", "r");
    if (fp == NULL) {
        qDebug() << "Failed to run command\n";
        return "";
    }
    /* Read the output a line at a time - output it. */
    while (fgets(path, sizeof(path), fp) != NULL) {
        if(strstr(path, "UP RUNNING")) {
            pclose(fp);
            return true;
        }
    }
    pclose(fp);
    return false;
}

bool file_exists (const char *filename);

void RebootOnBtError() {
    if(!file_exists(KEYPAD_CONFIG_FILE))
        return;
    if(IsBtRunning()) {
        if(file_exists(BT_ERROR_FILE))
            remove(BT_ERROR_FILE);
        return;
    }
    if(!file_exists(BT_ERROR_FILE)) {
        FILE *fp = fopen(BT_ERROR_FILE, "w");
        fclose(fp);
        system("sync");
        system("reboot");
    }
//    static pthread_t  h = 0;
//    pthread_create(&h, NULL, [](void *param) {
//        sleep(1);
//        if(IsBtRunning()) {
//            if(file_exists(BT_ERROR_FILE))
//                remove(BT_ERROR_FILE);
//            return (void*)NULL;
//        }
//        if(!file_exists(BT_ERROR_FILE)) {
//            FILE *fp = fopen(BT_ERROR_FILE, "w");
//            fclose(fp);
//            system("sync");
//            system("reboot");
//        }
//        return (void*)NULL;
//    }, NULL);
}
