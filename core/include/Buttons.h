#ifndef BUTTONS_H
#define BUTTONS_H

// Packet format
// <HEADER><DATA1>

// HEADER
#define KP_BUTTON_PRESSED     0x01
#define KP_BUTTON_RELEASED    0x02
#define KP_BATTERY_INFO       0x03
#define KP_POWERING_DOWN      0x04
#define KP_VERSION_INFO       0x05
#define KP_SERIAL_INFO        0x06


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

#define LED1_IO         19    // LED on when any button is pressed

typedef enum {
  eBUTTON_CENTER  = 0,
  eBUTTON_UP,
  eBUTTON_DOWN ,
  eBUTTON_RIGHT ,
  eBUTTON_LEFT ,
  eBUTTON_HELP ,
  eBUTTON_ROUND_LEFT,
  eBUTTON_ROUND_RIGHT,
  eBUTTON_SQUARE_LEFT ,
  eBUTTON_SQUARE_RIGHT ,
  NUM_BUTTONS
} eButtonEnum_t;

#define IO_BUTTON_CENTER   26
#define IO_BUTTON_UP       33
#define IO_BUTTON_DOWN     27
#define IO_BUTTON_RIGHT    25
#define IO_BUTTON_LEFT     17
#define IO_BUTTON_HELP     4
#define IO_BUTTON_ROUND_LEFT     16
#define IO_BUTTON_ROUND_RIGHT    14
#define IO_BUTTON_SQUARE_LEFT    15
#define IO_BUTTON_SQUARE_RIGHT   13

#define IO_USB_CONNECTED          2

#endif // BUTTONS_H
