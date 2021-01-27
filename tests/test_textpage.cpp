/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#include <doctest.h>
#include "textpage.h"

TEST_CASE("TextPage")
{
    TextPage page(5);

    page.setParagraphNumLines(0, 3);
    page.addParagraphLine(0, "Hello world");
    page.addParagraphLine(0, "one two three. Starting");

    DOCTEST_SUBCASE("numParagraphs") {
        CHECK_EQ(page.numParagraphs(), 5);
    }

    DOCTEST_SUBCASE("Non-empy paragrah 0") {
        CHECK_FALSE(page.paragraph(0).text().isEmpty());
    }

    DOCTEST_SUBCASE("Empty paragrah 1") {
        CHECK(page.paragraph(1).text().isEmpty());
    }


    DOCTEST_SUBCASE("getText") {
        CHECK_EQ(page.getText(0, 0), QString("Hello world one two three."));
    }

    DOCTEST_SUBCASE("getText2") {
        page.addParagraphLine(0, "and finishing");
        CHECK_EQ(page.getText(0, 0), QString("Hello world one two three. Starting and finishing"));
    }
}
