#ifndef _BASE_COMM_H_
#define _BASE_COMM_H_

#define byte unsigned char

#define LogError printf

// command bits 7,6
#define I2C_COMMAND_GET_KEY_STATUS  0x40
#define I2C_COMMAND_GET_BATTERY     0x80
#define I2C_COMMAND_OTHER           0xc0

//Command bits 5-0 when bits 7-6 are OTHER
#define I2C_COMMAND_OTHER_POWER_STATUS          0x20      // Base will respond with status in next 5 bytes
#define I2C_COMMAND_OTHER_BOOT_COMPLETE         0x10       // RPI to Base. Base should sto beeps
#define I2C_COMMAND_OTHER_SAFE_TO_POWER_DOWN    0x11       // RPI to Base
#define I2C_COMMAND_OTHER_POWER_DOWN            0x12       // Base to RPI. Request to sync


#define BUTTON_PAUSE_MASK        0x01
#define BUTTON_BACK_MASK         0x02
#define BUTTON_RATE_UP_MASK      0x04
#define BUTTON_RATE_DN_MASK      0x10
#define SWITCH_FOLDED_MASK       0x20
#define SWITCH_FOLDED_MASK_UP    0x40

//I2C_COMMAND_OTHER_POWER_STATUS bitmap
#define I2C_POWER_STATUS_CHARGER_IN           0x1
#define I2C_POWER_STATUS_CHARGING             0x2
#define I2C_POWER_STATUS_CHARGE_FLT           0x4

class BaseComm {
public:
	int m_nSequence;
	int m_fdI2C;

	int init();	
	int sendCommand(byte pCommand, byte *pReply);
	int flushInput();

};

#endif //_BASE_COMM_H_
