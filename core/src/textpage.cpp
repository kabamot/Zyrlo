/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Nov 2020
 *
 ****************************************************************************/

#include "textpage.h"

#include <QRegularExpression>

TextPage::TextPage(int numOfParagraphs)
    : m_paragraphs(numOfParagraphs)
{
    for (int i = 0; i < m_paragraphs.size(); ++i) {
        paragraph(i).setId(i);
    }
}

int TextPage::numParagraphs() const
{
    return m_paragraphs.size();
}

Paragraph &TextPage::paragraph(int num)
{
    return m_paragraphs[num];
}

const Paragraph &TextPage::paragraph(int num) const
{
    return m_paragraphs[num];
}

QString TextPage::text() const
{
    QString allText;
    for (const auto &paragraph : m_paragraphs) {
        allText.append(paragraph.text());
    }

    return allText;
}

QString TextPage::getText(int paragraphNum, int position) const
{
    auto paragraphText = m_paragraphs[paragraphNum].text().mid(position);
    if (!m_paragraphs[paragraphNum].isComplete()) {
        static const QRegularExpression sentenceRe(R"([\.!?])");
        const auto sentenceBoundaryPos = paragraphText.lastIndexOf(sentenceRe);
        if (sentenceBoundaryPos >= 0) {
            paragraphText.chop(paragraphText.size() - sentenceBoundaryPos - 1);
        }
    }
    return paragraphText;
}

QString TextPage::formattedText() const
{
    QString allText;
    for (const auto &paragraph : m_paragraphs) {
        allText.append(QStringLiteral("<p style='font-size:30px'>%1</p>").arg(paragraph.text()));
    }

    return allText;
}

bool TextPage::isNumOk(int num) const
{
    return num >= 0 && num < numParagraphs();
}
