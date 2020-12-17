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
    explicit TextPage(int numOfParagraphs);

    int numParagraphs() const;

    Paragraph &paragraph(int num);
    const Paragraph &paragraph(int num) const;
    QString text() const;
    QString formattedText() const;

private:
    bool isNumOk(int num) const;

private:
    QVector<Paragraph> m_paragraphs;
};

