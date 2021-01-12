/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#include "paragraph.h"

Paragraph::Paragraph(int id, int firstLineNum, int numLines)
    : m_id(id)
    , m_firstLineNum(firstLineNum)
    , m_numLines(numLines)
{

}

int Paragraph::id() const
{
    return m_id;
}

void Paragraph::setId(int id)
{
    m_id = id;
}

int Paragraph::firstLineNum() const
{
    return m_firstLineNum;
}

void Paragraph::setFirstLineNum(int firstLine)
{
    m_firstLineNum = firstLine;
}

int Paragraph::numLines() const
{
    return m_numLines;
}

void Paragraph::setNumLines(int numLines)
{
    m_numLines = numLines;
}

void Paragraph::addLine(const QString &line)
{
    auto newLine = line.simplified();
    if (newLine.back() == '-') {
        // Remove last dash '-' as it will be connected to the next line
        newLine.chop(1);
    } else {
        // Normally add space between lines
        newLine.append(' ');
    }

    m_lines.append(newLine);
    ++m_addedNumLines;
}

QString Paragraph::text() const
{
    return m_lines.join("").trimmed();
}

bool Paragraph::isComplete() const
{
    return m_addedNumLines == m_numLines;
}
