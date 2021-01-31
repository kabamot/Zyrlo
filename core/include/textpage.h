/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Nov 2020
 *
 ****************************************************************************/

#pragma once

#include <QVector>
#include "paragraph.h"

/*
 * @brief TextPage handles one page of text extracted by OcrHandler
 *
 */
class TextPage
{
public:
    TextPage() = default;

    int numParagraphs() const;

    const Paragraph &paragraph(int num) const;
    QString text() const;
    QString getText(int paragraphNum, int position) const;

    void addParagraph();
    void addParagraphLine(const QString &text);

    bool isComplete() const;
    void setCompleted();

private:
    bool isNumOk(int num) const;

private:
    bool m_isComplete {false};
    QVector<Paragraph> m_paragraphs;
};

