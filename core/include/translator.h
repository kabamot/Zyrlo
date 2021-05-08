#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <map>
class Translator : public std::map<std::string, std::map<std::string, std::string> >
{
    std::string m_sCurrLang = "eng";
public:
    Translator();
    bool Init(std::string sFileName);
    void SetLanguage(const std::string & sLang);
    std::string GetString(const std::string & sTag) const;
};

#define MENU_ABOUT                                      "MENU_ABOUT"
#define MENU_MSG_BLUETOOTH_ERROR                        "MENU_MSG_BLUETOOTH_ERROR"
#define MENU_BLUETOOTH                                  "MENU_BLUETOOTH"
#define MSG_BLUETOOTH_SCAN_FINISHED                     "MSG_BLUETOOTH_SCAN_FINISHED"
#define CLEAR_SURF                                      "CLEAR_SURF"
#define USB_KEY_CONV_PAGE                               "USB_KEY_CONV_PAGE"
#define USB_CONVERT_COMPLETE                            "USB_CONVERT_COMPLETE"
#define MENU_MSG_UNPAIR                                 "MENU_MSG_UNPAIR"
#define PUNCT_EOL                                       "PUNCT_EOL"
#define END_OF_TEXT                                     "END_OF_TEXT"
#define MENU_MSG_PAIR_ERROR                             "MENU_MSG_PAIR_ERROR"
#define MENU_EXIT                                       "MENU_EXIT"
#define MENU_MSG_EXITED                                 "MENU_MSG_EXITED"
#define USB_FILES_TO_CONVERT                            "USB_FILES_TO_CONVERT"
#define GESTURES_OFF                                    "GESTURES_OFF"
#define GESTURES_ON                                     "GESTURES_ON"
#define MENU_LANGUAGE                                   "MENU_LANGUAGE"
#define MAIN_BATTERY_LEVEL                              "MAIN_BATTERY_LEVEL"
#define NAVIGATION_BY_PARAGRAPH                         "NAVIGATION_BY_PARAGRAPH"
#define NAVIGATION_BY_SENTENCE                          "NAVIGATION_BY_SENTENCE"
#define NAVIGATION_BY_SYMBOL                            "NAVIGATION_BY_SYMBOL"
#define NAVIGATION_BY_WORD                              "NAVIGATION_BY_WORD"
#define MENU_OPTIONS                                    "MENU_OPTIONS"
#define PAGE_RECALL                                     "PAGE_RECALL"
#define PAGE_SAVED                                      "PAGE_SAVED"

#define MENU_PAIRED_DEV                                 "MENU_PAIRED_DEV"
#define MENU_MSG_PAIRING                                "MENU_MSG_PAIRING"
#define PLACE_DOC                                       "PLACE_DOC"
#define READ_NORMAL                                     "READ_NORMAL"
#define READ_THRU_COLUMNS                               "READ_THRU_COLUMNS"
#define MENU_SCAN_DEV                                   "MENU_SCAN_DEV"
#define MENU_MSG_SCANNING                               "MENU_MSG_SCANNING"
#define PUNCT_SPACE                                     "PUNCT_SPACE"
#define SPEECH_SPEED_DN                                 "SPEECH_SPEED_DN"
#define SPEECH_SPEED_UP                                 "SPEECH_SPEED_UP"
#define MENU_MSG_PAIRED_DEV                             "MENU_MSG_PAIRED_DEV"
#define PUNCT_TAB                                       "PUNCT_TAB"
#define TOP_OF_TEXT                                     "TOP_OF_TEXT"

#define USB_KEY_INSERTED                                "USB_KEY_INSERTED"
#define USB_KEY_REMOVED                                 "USB_KEY_REMOVED"
#define WAIT_FOR_OCR_TO_COMPLETE                        "WAIT_FOR_OCR_TO_COMPLETE"

#define VOICE_SET_TO                                    "VOICE_SET_TO"
#define VOICE_SET_AUTO                                  "VOICE_SET_AUTO"
#define SWITCHING_TO_BT                                 "SWITCHING_TO_BT"
#define SWITCHING_TO_SPK                                "SWITCHING_TO_SPK"
#define TEXT_NOT_FOUND                                  "TEXT_NOT_FOUND"
#define BATTERY_LOW                                     "BATTERY_LOW"
#define USB_NUM_PAGE_EXCEED                             "USB_NUM_PAGE_EXCEED"
#define USB_OUT_OF_STORAGE                              "USB_OUT_OF_STORAGE"

#define MENU_MENU                                       "MENU_MENU"
#define MENU_ENABLED                                    "MENU_ENABLED"
#define MENU_DISABLED                                   "MENU_DISABLED"
#define MENU_VERSION                                    "MENU_VERSION"
#define MENU_SERIAL                                     "MENU_SERIAL"
#define MENU_MSG_PAIR_SUCCESS                           "MENU_MSG_PAIR_SUCCESS"
#define MENU_ENABLE_FLASH                               "MENU_ENABLE_FLASH"

//Help
#define BUTTON_HELP                                     "BUTTON_HELP"
#define BUTTON_PAUSE_RESUME                             "BUTTON_PAUSE_RESUME"
#define BUTTON_ARROW_UP                                 "BUTTON_ARROW_UP"
#define BUTTON_ARROW_DOWN                               "BUTTON_ARROW_DOWN"
#define BUTTON_ARROW_LEFT                               "BUTTON_ARROW_LEFT"
#define BUTTON_ARROW_RIGHT                              "BUTTON_ARROW_RIGHT"
#define BUTTON_VOICE                                    "BUTTON_VOICE"
#define BUTTON_SPELL                                    "BUTTON_SPELL"
#define BUTTON_SAVE                                     "BUTTON_SAVE"
#define BUTTON_RECALL                                   "BUTTON_RECALL"

#endif // TRANSLATOR_H
