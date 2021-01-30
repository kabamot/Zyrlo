/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Jan 2021
 *
 ****************************************************************************/

#pragma once

#include <QString>
#include <vector>

/*
 * PositionMapper maps the positions from text with Cerence's escape codes
 * to clean text positions
 *
 * Ex.    "<ESC>\tn=spell\hello <ESC>\tn=normal\world" - escape codes with two
 * words: "hello world"
 * First word "hello" which is 11 (position) is mapped to 0
 * and second word "world" which is 29 is mapped to  6
 *
 */
class PositionMapper
{
public:
    PositionMapper() = default;

    void setText(const QString &text);
    int position(int pos) const;

private:
    std::vector<std::pair<int, int>> m_positions;
};

