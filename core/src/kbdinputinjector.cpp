#include "kbdinputinjector.h"
#include <fcntl.h>
#include "uinput.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/input.h>

void kbd_emit(int fd, int type, int code, int val)
{
   struct input_event ie;

   ie.type = type;
   ie.code = code;
   ie.value = val;
   /* timestamp values below are ignored */
   ie.time.tv_sec = 0;
   ie.time.tv_usec = 0;

   write(fd, &ie, sizeof(ie));
}

int create_virtual_device(const char *szDev, bool keyboard, int mouse)
{
    struct uinput_dev dev;
    int fd_kb, aux;

    fd_kb = open(szDev, O_RDWR);
    if (fd_kb < 0) {
        printf("Can't open input device:%s ", szDev);
        return -1;
    }

    memset(&dev, 0, sizeof(dev));
    strcpy(dev.name, "KeyInjector Input");
    dev.id.bustype = BUS_HOST;
    dev.id.vendor  = 0x0001;
    dev.id.product = 0x0002;
    dev.id.version = 0x0001;

    if (write(fd_kb, &dev, sizeof(dev)) < 0) {
        perror("Can't write device information");
        close(fd_kb);
        return -1;
    }

    if (mouse) {
        ioctl(fd_kb, UI_SET_EVBIT, EV_REL);
        for (aux = REL_X; aux <= REL_MISC; aux++)
            ioctl(fd_kb, UI_SET_RELBIT, aux);
    }

    if (keyboard) {
        ioctl(fd_kb, UI_SET_EVBIT, EV_KEY);
        ioctl(fd_kb, UI_SET_EVBIT, EV_LED);
        ioctl(fd_kb, UI_SET_EVBIT, EV_REP);

        for (aux = KEY_RESERVED; aux <= KEY_UNKNOWN; aux++)
            ioctl(fd_kb, UI_SET_KEYBIT, aux);
    }

    if (mouse) {
        ioctl(fd_kb, UI_SET_EVBIT, EV_KEY);

        for (aux = BTN_LEFT; aux <= BTN_BACK; aux++)
            ioctl(fd_kb, UI_SET_KEYBIT, aux);
    }

    ioctl(fd_kb, UI_DEV_CREATE);
    //printf("intCreate success: %d",  fd_kb);
    return fd_kb;
}

void close_virtual_device(int fd_kb) {
    if(fd_kb < 0)
        return;
    ioctl(fd_kb, UI_DEV_DESTROY);
    close(fd_kb);
}

void intSendEvent(int fd_kb, uint16_t type, uint16_t code, int32_t value)
{

   if (fd_kb <= fileno(stderr))
        return;

   if(value == 2) {
        kbd_emit(fd_kb, type, code, 1);
        kbd_emit(fd_kb, EV_SYN, SYN_REPORT, 0);
        kbd_emit(fd_kb, type, code, 0);
        kbd_emit(fd_kb, EV_SYN, SYN_REPORT, 0);
   }
   else {
        kbd_emit(fd_kb, type, code, value);
        kbd_emit(fd_kb, EV_SYN, SYN_REPORT, 0);
   }
}

void KbdInputInjector::SendKey(int key, bool bDown) {
        if(key == 0)
            intSendEvent(m_fd_kb, 0, 0, 0);
        if (bDown)
            intSendEvent(m_fd_kb, EV_KEY, key, 1); //key down
        else
            intSendEvent(m_fd_kb, EV_KEY, key, 0); //key up
    }

KbdInputInjector::KbdInputInjector() {
    system("sudo chmod 666 /dev/uinput");
    m_fd_kb = create_virtual_device("/dev/uinput", true, false);
}

KbdInputInjector::~KbdInputInjector()
{
    if(m_fd_kb >= 0)
        close_virtual_device(m_fd_kb);
}

void KbdInputInjector::sendKeyEvent(int keyCode) {
    SendKey(keyCode, true);
    SendKey(keyCode, false);
}
