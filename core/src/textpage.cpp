/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Nov 2020
 *
 ****************************************************************************/

#include "textpage.h"

#include <QRegularExpression>
#include <QDebug>

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

void TextPage::addParagraph()
{
    const auto paragraphNum = m_paragraphs.size();
    m_paragraphs.push_back(Paragraph(paragraphNum));

    if (paragraphNum > 0) {
        // As new paragraph added, the previous paragraph we set as completed
        m_paragraphs[paragraphNum - 1].setCompleted();
    }
}

void TextPage::addParagraphLine(const QString &text)
{
    if (m_paragraphs.empty()) {
        qWarning() << __func__ << __LINE__ << "Adding line to non-existent paragraph!";
        return;
    }

    const auto paragraphNum = m_paragraphs.size() - 1;

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
}

bool TextPage::isComplete() const
{
    return m_isComplete;
}

void TextPage::setCompleted()
{
    m_isComplete = true;

    // Set last paragraph as complete
    if (!m_paragraphs.isEmpty())
        m_paragraphs.back().setCompleted();
}

bool TextPage::isNumOk(int num) const
{
    return num >= 0 && num < numParagraphs();
}
