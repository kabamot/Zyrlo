/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Jan 2021
 *
 ****************************************************************************/

#include <doctest.h>
#include "cerence/positionmapper.h"
#include "cerence/cerencetts_const.h"

TEST_CASE("PositionMapper with tags from the beginning")
{
    // "hello world"
    QString s1 = R"(<ESC>\tn=spell\hello <ESC>\tn=normal\world)";
    s1.replace("<ESC>", CERENCE_ESC);

    PositionMapper mapper;
    mapper.setText(s1);

    CHECK_EQ(mapper.position(0), 0);

    // it returns the same number, because the given position is incorrect:
    // it doesn't have to be inside tags
    CHECK_EQ(mapper.position(5), 5);

    CHECK_EQ(mapper.position(11), 0);
    CHECK_EQ(mapper.position(12), 1);
    CHECK_EQ(mapper.position(29), 6);
}

TEST_CASE("PositionMapper with tags")
{
    // "What a perfect day! hello world"
    QString s1 = R"(What a perfect day! <ESC>\tn=spell\hello <ESC>\tn=normal\world)";
    s1.replace("<ESC>", CERENCE_ESC);

    PositionMapper mapper;
    mapper.setText(s1);

    CHECK_EQ(mapper.position(0), 0);
    CHECK_EQ(mapper.position(7), 7);
    CHECK_EQ(mapper.position(31), 20);
    CHECK_EQ(mapper.position(32), 21);
    CHECK_EQ(mapper.position(49), 26);
}

TEST_CASE("PositionMapper without tags")
{
    // "hello world"
    const QString s1 = "hello \\aaa\\ world";

    PositionMapper mapper;
    mapper.setText(s1);

    CHECK_EQ(mapper.position(0), 0);
    CHECK_EQ(mapper.position(5), 5);
    CHECK_EQ(mapper.position(15), 15);
}
