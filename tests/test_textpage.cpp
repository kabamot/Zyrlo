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

    page.paragraph(0).setNumLines(3);
    page.paragraph(0).addLine("Hello world");
    page.paragraph(0).addLine("one two three.");

    DOCTEST_SUBCASE("numParagraphs") {
        CHECK_EQ(page.numParagraphs(), 5);
    }

    DOCTEST_SUBCASE("Non-empy paragrah 0") {
        CHECK_FALSE(page.paragraph(0).text().isEmpty());
    }

    DOCTEST_SUBCASE("Empty paragrah 1") {
        CHECK(page.paragraph(1).text().isEmpty());
    }
}
