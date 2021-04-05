/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#pragma once

#include <QString>
#include <QStringList>

#include "textposition.h"
#include <map>

using Positions = QVector<TextPosition>; // Represents starting position of words/sentences in the paragraph

class Paragraph
{

public:
    Paragraph() = default;
    explicit Paragraph(int id);

    int id() const;
    void setId(int id);

    int paragraphPosition() const;
    void setParagraphPosition(int pos);

    int length() const;

    int firstLineNum() const;
    void setFirstLineNum(int firstLine);

    int numLines() const;

    void addLine(const QString &line, const QString &lang);
    QString text() const;
    std::pair<QString, int> lang(int position) const;

    bool isComplete() const;
    void setCompleted();
    bool hasText() const;

    TextPosition prevCharPosition(int pos) const;
    TextPosition nextCharPosition(int pos) const;
    TextPosition firstCharPosition() const;
    TextPosition lastCharPosition() const;
    TextPosition prevWordPosition(int pos) const;
    TextPosition nextWordPosition(int pos) const;
    TextPosition currentWordPosition(int pos) const;
    TextPosition firstWordPosition() const;
    TextPosition lastWordPosition() const;

    TextPosition prevSentencePosition(int pos) const;
    TextPosition nextSentencePosition(int pos) const;
    TextPosition currentSentencePosition(int pos) const;
    TextPosition firstSentencePosition() const;
    TextPosition lastSentencePosition() const;

private:
    void parseChars();
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
    int m_addedNumLines {0};
    int m_paragraphPosition {-1};
    bool m_isComplete {false};

    QStringList m_lines;
    std::map<int, QString> m_langTagPos;

    Positions m_chars;
    Positions m_words;
    Positions m_sentences;
};

