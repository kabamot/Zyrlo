/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#pragma once

#include <QString>
#include <QStringList>

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

private:
    int m_id {-1};
    int m_firstLineNum {-1};
    int m_numLines {-1};
    QStringList m_lines;
};

