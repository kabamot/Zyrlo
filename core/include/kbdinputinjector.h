#ifndef KBDINPUTINJECTOR_H
#define KBDINPUTINJECTOR_H

#define KEYCODE_LEFT   105
#define KEYCODE_RIGHT  106
#define KEYCODE_UP     103
#define KEYCODE_DOWN   108
#define KEYCODE_ENTER  28
#define KEYCODE_ESC    1

class KbdInputInjector
{
    int m_fd_kb = -1;
public:
    KbdInputInjector();
    ~KbdInputInjector();
    void SendKey(int key, bool bDown);
    void sendKeyEvent(int keyCode);
 };

#endif // KBDINPUTINJECTOR_H
