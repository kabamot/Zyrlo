/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#include <doctest.h>
#include "textpage.h"

TEST_CASE("TextPage 1")
{
    TextPage page;

    page.addParagraph();
    page.addParagraphLine("Hello world");
    page.addParagraphLine("one two three. Starting");

    page.addParagraph();
    page.addParagraphLine("second paragraph");

    page.addParagraph();

    DOCTEST_SUBCASE("numParagraphs") {
        CHECK_EQ(page.numParagraphs(), 3);
    }

    DOCTEST_SUBCASE("Non-empy paragrah 0") {
        CHECK_FALSE(page.paragraph(0).text().isEmpty());
        CHECK_FALSE(page.paragraph(1).text().isEmpty());
        CHECK(page.paragraph(2).text().isEmpty());
    }

    DOCTEST_SUBCASE("isComplete") {
        CHECK_FALSE(page.isComplete());

        page.setCompleted();
        CHECK(page.isComplete());
    }
}

TEST_CASE("TextPage 2")
{
    TextPage page;

    page.addParagraph();
    page.addParagraphLine("Hello world");
    page.addParagraphLine("one two three. Starting");

    DOCTEST_SUBCASE("getText") {
        CHECK_EQ(page.getText(0, 0), QString("Hello world one two three."));
    }

    DOCTEST_SUBCASE("getText with more added lines") {
        page.addParagraphLine("and finishing");
        page.addParagraph(); // this automatically completes previous paragpraph

        DOCTEST_INFO(page.getText(0, 0).toStdString());
        CHECK_EQ(page.getText(0, 0), QString("Hello world one two three. Starting and finishing"));
    }
}
