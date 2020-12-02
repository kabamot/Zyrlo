/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Nov 2020
 *
 ****************************************************************************/

#include "textpage.h"

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

bool TextPage::isNumOk(int num) const
{
    return num >= 0 && num < numParagraphs();
}
