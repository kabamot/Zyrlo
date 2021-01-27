/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#include "paragraph.h"

#include <QRegularExpression>

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

int Paragraph::paragraphPosition() const
{
    return m_paragraphPosition;
}

void Paragraph::setParagraphPosition(int pos)
{
    m_paragraphPosition = pos;
}

int Paragraph::length() const
{
    return text().length();
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

    parseWords();
    parseSenteces();
}

QString Paragraph::text() const
{
    return m_lines.join("").trimmed();
}

bool Paragraph::isComplete() const
{
    return m_addedNumLines >= m_numLines;
}

bool Paragraph::hasText() const
{
    return m_addedNumLines > 0;
}

TextPosition Paragraph::prevWordPosition(int pos) const
{
    return prevPosition(m_words, pos);
}

TextPosition Paragraph::nextWordPosition(int pos) const
{
    return nextPosition(m_words, pos);
}

TextPosition Paragraph::currentWordPosition(int pos) const
{
    return currentPosition(m_words, pos);
}

TextPosition Paragraph::firstWordPosition() const
{
    return firstPosition(m_words);
}

TextPosition Paragraph::lastWordPosition() const
{
    return lastPosition(m_words);
}

TextPosition Paragraph::prevSentencePosition(int pos) const
{
    return prevPosition(m_sentences, pos);
}

TextPosition Paragraph::nextSentencePosition(int pos) const
{
    return nextPosition(m_sentences, pos);
}

TextPosition Paragraph::currentSentencePosition(int pos) const
{
    return currentPosition(m_sentences, pos);
}

TextPosition Paragraph::firstSentencePosition() const
{
    return firstPosition(m_sentences);
}

TextPosition Paragraph::lastSentencePosition() const
{
    return lastPosition(m_sentences);
}

void Paragraph::parseWords()
{
    const static QRegularExpression re(R"([\w\d]+)");
    m_words = parseToPositions(text(), re);
}

void Paragraph::parseSenteces()
{
    const static QRegularExpression re(R"([^\.!?]{3,})");
    m_sentences = parseToPositions(text(), re);
}

Positions Paragraph::parseToPositions(const QString &text, const QRegularExpression &re)
{
    Positions positions;

    int nextPos = 0;

    while (true) {
        auto match = re.match(text, nextPos);
        if (!match.hasMatch()) {
            break;
        }

        positions.push_back(TextPosition(match.capturedStart(),
                                         match.capturedLength(),
                                         paragraphPosition()));

        nextPos = match.capturedEnd();
    }

    return positions;
}

TextPosition Paragraph::prevPosition(const Positions &positions, int pos) const
{
    int index = indexByTextPosition(positions, pos);
    if (index <= 0)
        return TextPosition{};

    return positions[index - 1];
}

TextPosition Paragraph::nextPosition(const Positions &positions, int pos) const
{
    int index = indexByTextPosition(positions, pos);
    if (index < 0 || index == positions.size() - 1)
        return TextPosition{};

    return positions[index + 1];
}

TextPosition Paragraph::currentPosition(const Positions &positions, int pos) const
{
    int index = indexByTextPosition(positions, pos);
    if (index < 0)
        return TextPosition{};

    return positions[index];
}

TextPosition Paragraph::firstPosition(const Positions &positions) const
{
    if (positions.empty())
        return TextPosition{};

    return positions.front();
}

TextPosition Paragraph::lastPosition(const Positions &positions) const
{
    if (positions.empty())
        return TextPosition{};

    return positions.back();
}

// Returns index in the given positions array using the current position. If the
// current position is less than the first index's position, then it returns -1
int Paragraph::indexByTextPosition(const Positions &positions, int currentPosition) const
{
    int currentIndex = 0;
    for (; currentIndex < positions.size(); ++currentIndex) {
        if (positions[currentIndex].parPos() > currentPosition) {
            break;
        }
    }

    return --currentIndex;
}
