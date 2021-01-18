/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#pragma once

#include <QString>
#include <QStringList>

using Positions = QVector<int>; // Represents starting position of words/sentences in the paragraph

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

    int prevWordPosition(int pos) const;
    int nextWordPosition(int pos) const;
    int lastWordPosition() const;

    int prevSentencePosition(int pos) const;
    int nextSentencePosition(int pos) const;
    int currentSentencePosition(int pos) const;
    int lastSentencePosition() const;

private:
    void parseWords();
    void parseSenteces();
    Positions parseToPositions(const QString &text, const QRegularExpression &re);

    int prevPosition(const Positions &positions, int pos) const;
    int nextPosition(const Positions &positions, int pos) const;
    int currentPosition(const Positions &positions, int pos) const;
    int lastPosition(const Positions &positions) const;
    int indexByTextPosition(const Positions &positions, int currentPosition) const;

private:
    int m_id {-1};
    int m_firstLineNum {-1};
    int m_numLines {-1};
    int m_addedNumLines {0};
    QStringList m_lines;

    Positions m_words;
    Positions m_sentences;
};

