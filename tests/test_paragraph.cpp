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
    Paragraph p0(0);
    p0.addLine("one", "eng");
    p0.addLine("two\nthree", "eng");
    p0.setFirstLineNum(0);
    p0.setCompleted();

    Paragraph p1(1);
    p1.addLine("four, five",  "eng");
    p1.setFirstLineNum(2);

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
        CHECK_EQ(p1.firstLineNum(), 2);
        CHECK_EQ(p2.firstLineNum(), -1);
    }

    DOCTEST_SUBCASE("Paragraph setFistLineNum") {
        p2.setFirstLineNum(20);
        CHECK_EQ(p2.firstLineNum(), 20);
    }

    DOCTEST_SUBCASE("Paragraph numLines") {
        CHECK_EQ(p0.numLines(), 2);
        CHECK_EQ(p1.numLines(), 1);
        CHECK_EQ(p2.numLines(), 0);
    }

    DOCTEST_SUBCASE("Paragraph setFistLineNum") {
        p2.setFirstLineNum(9);
        CHECK_EQ(p2.firstLineNum(), 9);
    }

    DOCTEST_SUBCASE("Paragraph addLines with extra spaces") {
        p2.addLine("  Hello     world.    ", "eng");
        p2.addLine(" One two three  ", "eng");
        CHECK(p2.text() == "Hello world. One two three");
    }

    DOCTEST_SUBCASE("Paragraph addLines without spaces") {
        p2.addLine("Hello world.", "eng");
        p2.addLine("One two three", "eng");
        CHECK_EQ(p2.text(), "Hello world. One two three");
    }

    DOCTEST_SUBCASE("Paragraph addLines with dash") {
        p2.addLine("Hel-",  "eng");
        p2.addLine(" lo world  ",  "eng");
        CHECK_EQ(p2.text(), "Hello world");
    }

    DOCTEST_SUBCASE("isCompleted") {
        CHECK(p0.isComplete());
        CHECK_FALSE(p1.isComplete());
        CHECK_FALSE(p2.isComplete());
    }

    DOCTEST_SUBCASE("hasText") {
        CHECK(p0.hasText());
        CHECK(p1.hasText());
        CHECK_FALSE(p2.hasText());
    }

    DOCTEST_SUBCASE("length") {
        CHECK_EQ(p0.length(), 13);
    }
}
