/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#pragma once

#include <QString>
#include <QStringList>

struct TextPosition {
    int parPos {-1};
    int absPos {-1};
    int length {0};

    bool isValid() const { return parPos >= 0; }
};

using Positions = QVector<TextPosition>; // Represents starting position of words/sentences in the paragraph

class Paragraph
{
public:
    Paragraph() = default;
    explicit Paragraph(int id, int firstLineNum, int numLines);

    int id() const;
    void setId(int id);

    int firstLineNum() const;
    void setFirstLineNum(int firstLine);

    int numLines() const;
    void setNumLines(int numLines);

    void addLine(const QString &line);
    QString text() const;

    bool isComplete() const;
    bool hasText() const;

    TextPosition prevWordPosition(int pos) const;
    TextPosition nextWordPosition(int pos) const;
    TextPosition firstWordPosition() const;
    TextPosition lastWordPosition() const;

    TextPosition prevSentencePosition(int pos) const;
    TextPosition nextSentencePosition(int pos) const;
    TextPosition currentSentencePosition(int pos) const;
    TextPosition firstSentencePosition() const;
    TextPosition lastSentencePosition() const;

private:
    void parseWords();
    void parseSenteces();
    Positions parseToPositions(const QString &text, const QRegularExpression &re);

    TextPosition prevPosition(const Positions &positions, int pos) const;
    TextPosition nextPosition(const Positions &positions, int pos) const;
    TextPosition currentPosition(const Positions &positions, int pos) const;
    TextPosition firstPosition(const Positions &positions) const;
    TextPosition lastPosition(const Positions &positions) const;
    int indexByTextPosition(const Positions &positions, int currentPosition) const;

private:
    int m_id {-1};
    int m_firstLineNum {-1};
    int m_numLines {-1};
    int m_addedNumLines {0};
    int m_paragraphPosition {0};
    QStringList m_lines;

    Positions m_words;
    Positions m_sentences;
};

