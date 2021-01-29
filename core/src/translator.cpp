#include "translator.h"
#include "tinyxml.h"
#include <QDebug>

using namespace std;

Translator::Translator()
{
}

bool Translator::Init(string sFileName) {
    clear();
    TiXmlDocument doc(sFileName.c_str());

    if(!doc.LoadFile())
        return false;
    TiXmlNode *spRoot;
    for(spRoot = doc.FirstChild(); spRoot && !strstr(spRoot->Value(), "Zyrlo"); spRoot = spRoot->NextSibling());
    if (!spRoot)
        return false;
    for(TiXmlNode *pTag = spRoot->FirstChild(); pTag; pTag = pTag->NextSibling()) {
        iterator i = insert(value_type(pTag->Value(), map<string, string>())).first;
        for(TiXmlElement *pEl = pTag->FirstChildElement(); pEl; pEl = pEl->NextSiblingElement()) {
            i->second.insert(pair<string, string>(pEl->Value(), pEl->GetText()));
            qDebug() << pEl->Value() << pTag->Value() << pEl->GetText() << Qt::endl;
        }
    }
    return true;
}

void Translator::SetLanguage(const string & sLang) {
    m_sCurrLang = sLang;
}

string Translator::GetString(const string & sTag) const {
    const_iterator i = find(sTag);
    if(i == end())
        return sTag;
    map<string, string>::const_iterator j = i->second.find(m_sCurrLang);
    if(j == i->second.end())
        return sTag;
    return j->second;
}

