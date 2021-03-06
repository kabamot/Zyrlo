#ifndef __BT_COMM_H__
#define __BT_COMM_H__

#include <pthread.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <atomic>
#include <QFuture>

// Packet format
// <HEADER><DATA1>

// HEADER
#define KP_BUTTON_PRESSED     0x01
#define KP_BUTTON_RELEASED    0x02
#define KP_BATTERY_INFO       0x03
#define KP_POWERING_DOWN      0x04


#define KP_BUTTON_CENTER      0x01
#define KP_BUTTON_UP          0x02
#define KP_BUTTON_DOWN        0x03
#define KP_BUTTON_LEFT        0x04
#define KP_BUTTON_RIGHT       0x05
#define KP_BUTTON_HELP        0x06
#define KP_BUTTON_ROUND_L     0x07
#define KP_BUTTON_ROUND_R     0x08
#define KP_BUTTON_SQUARE_L    0x09
#define KP_BUTTON_SQUARE_R    0x0a

typedef enum {
	eStatusIdle,
	eStatusConnected,
	eStatusDisconnected,
	eStatusOff
} eBtCommStatus;

class MainController;

class BTComm
{
    struct sockaddr_rc m_addr = { 0 };
    unsigned char m_readBuffer[10];
    int m_s = -1;
    bool m_bUsingMainAudioSink = true;

public:

	int m_exitRequest;
	pthread_t m_ConnectThread;
    eBtCommStatus m_eStatus;

    char keypadMacStr[18] = {0};

    BTComm(MainController *pMainController) : m_pMainController(pMainController) {}
    int init();
    void btStop() { m_exitRequest = 1; };
    int receiveLoopStep(int & nVal);
    int btConnect(const std::atomic_bool &isStop);
    bool readKpConfig();
    void setUsingMainAudioSink(bool bUsingMainAudioSink) { m_bUsingMainAudioSink = bUsingMainAudioSink; }
    MainController *m_pMainController {nullptr};
    std::string kpConfig() const { return keypadMacStr; }
    void setKpConfig(const std::string & conf);
};

#endif // __BT_COMM_H__
