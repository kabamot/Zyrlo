/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Nov 2020
 *
 ****************************************************************************/

#include "textpage.h"

#include <QRegularExpression>
#include <QDebug>

TextPage::TextPage(int numOfParagraphs)
    : m_paragraphs(numOfParagraphs)
{
    for (int i = 0; i < m_paragraphs.size(); ++i) {
        setParagraphId(i, i);
    }
}

int TextPage::numParagraphs() const
{
    return m_paragraphs.size();
}

const Paragraph &TextPage::paragraph(int num) const
{
    return m_paragraphs[num];
}

QString TextPage::text() const
{
    QString allText;
    for (const auto &paragraph : m_paragraphs) {
        const auto parText = paragraph.text();
        if (!parText.isEmpty()) {
            if (!allText.isEmpty()) {
                allText.append('\n');
            }
            allText.append(paragraph.text());
        }
    }

    return allText;
}

QString TextPage::getText(int paragraphNum, int position) const
{
    if (paragraphNum >= m_paragraphs.size())
        return QString();

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
        allText.append(QStringLiteral("<p style='font-size:14pt'>%1</p>").arg(paragraph.text()));
    }

    return allText;
}

void TextPage::setParagraphId(int paragraphNum, int id)
{
    if (paragraphNum < numParagraphs()) {
        m_paragraphs[paragraphNum].setId(id);
    } else {
        qWarning() << __func__ << __LINE__ << "wrong paragraph number" << paragraphNum;
    }
}

void TextPage::setParagraphNumLines(int paragraphNum, int numLines)
{
    if (paragraphNum < numParagraphs()) {
        m_paragraphs[paragraphNum].setNumLines(numLines);
    } else {
        qWarning() << __func__ << __LINE__ << "wrong paragraph number" << paragraphNum;
    }
}

void TextPage::addParagraphLine(int paragraphNum, const QString &text)
{
    if (paragraphNum < numParagraphs()) {
        // Order is important:
        // First set paragraph's position in the page if it was not yet initialized
        if (m_paragraphs[paragraphNum].paragraphPosition() < 0) {
            if (paragraphNum == 0) {
                m_paragraphs[paragraphNum].setParagraphPosition(0);
            } else {
                const auto &prevParagraph = m_paragraphs[paragraphNum - 1];
                // Added 1 to the length due to newline character between paragraphs
                m_paragraphs[paragraphNum].setParagraphPosition(
                    prevParagraph.paragraphPosition() + prevParagraph.length() + 1);
            }
        }

        // Then add the line
        m_paragraphs[paragraphNum].addLine(text);
    } else {
        qWarning() << __func__ << __LINE__ << "wrong paragraph number" << paragraphNum;
    }
}

bool TextPage::isNumOk(int num) const
{
    return num >= 0 && num < numParagraphs();
}
