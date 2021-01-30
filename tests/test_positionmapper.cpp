/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Jan 2021
 *
 ****************************************************************************/

#include <doctest.h>
#include "cerence/positionmapper.h"

TEST_CASE("PositionMapper with tags")
{
    // "hello world"
    const QString s1 = "\x1b\\tn=spell\\hello \x1b\\tn=normal\\world";

    PositionMapper mapper;
    mapper.setText(s1);

    CHECK_EQ(mapper.position(0), 0);
    CHECK_EQ(mapper.position(5), 0);
    CHECK_EQ(mapper.position(11), 0);
    CHECK_EQ(mapper.position(12), 1);
    CHECK_EQ(mapper.position(29), 6);
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
