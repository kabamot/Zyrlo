/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/


#include <doctest.h>
#include "paragraph.h"

TEST_CASE("Paragraph")
{
    Paragraph p0(0, 0, 10);
    Paragraph p1(1, 10, 15);
    Paragraph p2;

    DOCTEST_SUBCASE("Paragraph id") {
        CHECK_EQ(p0.id(), 0);
        CHECK_EQ(p1.id(), 1);
        CHECK_EQ(p2.id(), -1);
    }

    DOCTEST_SUBCASE("Paragraph setId") {
        p2.setId(3);
        CHECK_EQ(p2.id(), 3);
    }

    DOCTEST_SUBCASE("Paragraph fistLineNum") {
        CHECK_EQ(p0.firstLineNum(), 0);
        CHECK_EQ(p1.firstLineNum(), 10);
        CHECK_EQ(p2.firstLineNum(), -1);
    }

    DOCTEST_SUBCASE("Paragraph setFistLineNum") {
        p2.setFirstLineNum(20);
        CHECK_EQ(p2.firstLineNum(), 20);
    }

    DOCTEST_SUBCASE("Paragraph numLines") {
        CHECK_EQ(p0.numLines(), 10);
        CHECK_EQ(p1.numLines(), 15);
        CHECK_EQ(p2.numLines(), -1);
    }

    DOCTEST_SUBCASE("Paragraph setFistLineNum") {
        p2.setNumLines(9);
        CHECK_EQ(p2.numLines(), 9);
    }

    DOCTEST_SUBCASE("Paragraph addLines with extra spaces") {
        p0.addLine("  Hello     world.    ");
        p0.addLine(" One two three  ");
        CHECK(p0.text() == "Hello world. One two three");
    }

    DOCTEST_SUBCASE("Paragraph addLines without spaces") {
        p0.addLine("Hello world.");
        p0.addLine("One two three");
        CHECK_EQ(p0.text(), "Hello world. One two three");
    }

    DOCTEST_SUBCASE("Paragraph addLines with dash") {
        p0.addLine("Hel-");
        p0.addLine(" lo world  ");
        CHECK_EQ(p0.text(), "Hello world");
    }
}
