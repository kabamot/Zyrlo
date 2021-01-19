#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <map>
class Translator : public std::map<std::string, std::map<std::string, std::string> >
{
    std::string m_sCurrLang = "enu";
public:
    Translator();
    bool Init();
    void SetLanguage(const std::string & sLang);
    std::string GetString(const std::string & sTag) const;
};

#endif // TRANSLATOR_H
