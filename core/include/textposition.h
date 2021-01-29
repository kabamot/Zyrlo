/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Jan 2021
 *
 ****************************************************************************/

#pragma once

class TextPosition {
public:
    TextPosition() = default;

    TextPosition(int posInParagraph, int textLength, int paragraphPosition)
    {
        m_parPos = posInParagraph;
        m_absPos = posInParagraph + paragraphPosition;
        m_length = textLength;
    }

    inline int parPos() const { return m_parPos; }
    inline int absPos() const { return m_absPos; }
    inline int length() const { return m_length; }
    inline bool isValid() const { return m_parPos >= 0; }
    inline void clear() { m_parPos = -1; m_absPos = -1; m_length = 0; }

private:
    int m_parPos {-1};
    int m_absPos {-1};
    int m_length {0};
};
