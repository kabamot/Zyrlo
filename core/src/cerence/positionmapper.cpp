/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Jan 2021
 *
 ****************************************************************************/

#include "positionmapper.h"
#include "cerencetts_const.h"

#include <algorithm>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

void PositionMapper::setText(const QString &text)
{
    static const QRegularExpression re(CERENCE_ESC R"(\\[^\\]+\\)");

    m_positions.clear();

    int pos = 0;
    int tagsLength = 0;

    while (pos >= 0) {
        QRegularExpressionMatch match;
        pos = text.indexOf(re, pos, &match);
        if (pos >= 0) {
            const auto length = match.capturedLength();
            tagsLength += length;
            m_positions.push_back({pos + length, tagsLength});
            pos += length;
        }
    }
}

int PositionMapper::position(int pos) const
{
    auto it = std::find_if(std::crbegin(m_positions), std::crend(m_positions),
                           [pos](const auto &a) { return pos >= a.first; });

    if (it == std::crend(m_positions))
        return pos;

    return pos - it->second;
}
