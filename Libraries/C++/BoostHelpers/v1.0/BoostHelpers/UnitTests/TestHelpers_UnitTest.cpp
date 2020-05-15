/////////////////////////////////////////////////////////////////////////
///
///  \file          TestHelpers_UnitTest.cpp
///  \brief         Unit test for TestHelpers.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-14 21:42:17
///
///  \note
///
///  \bug
///
/////////////////////////////////////////////////////////////////////////
///
///  \attention
///  Copyright David Brownell 2020
///  Distributed under the Boost Software License, Version 1.0. See
///  accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt.
///
/////////////////////////////////////////////////////////////////////////
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_CONSOLE_WIDTH 200
#include "../TestHelpers.h"
#include <catch.hpp>

#include <CommonHelpers/Compare.h>
#include <CommonHelpers/Copy.h>
#include <CommonHelpers/Move.h>

struct Base {
    int const                               I;
    bool const                              B;

    Base(int i, bool b) : I(i), B(b) {}
    virtual ~Base(void) = default;

    NON_COPYABLE(Base);
    MOVE(Base, MEMBERS(I, B));
    COMPARE(Base, MEMBERS(I, B));
    SERIALIZATION(Base, MEMBERS(I, B), FLAGS(SERIALIZATION_POLYMORPHIC_BASE));
};

struct Derived : public Base {
    double const                            D;

    Derived(int i, bool b, double d) : Base(i, b), D(d) {}

    NON_COPYABLE(Derived);
    MOVE(Derived, MEMBERS(D), BASES(Base));
    COMPARE(Derived, MEMBERS(D), BASES(Base));
    SERIALIZATION(Derived, MEMBERS(D), BASES(Base), FLAGS(SERIALIZATION_POLYMORPHIC(Base)));
};

TEST_CASE("Standard") {
    CHECK(BoostHelpers::TestHelpers::SerializeTest(Base(10, true)) == 0);
    CHECK(BoostHelpers::TestHelpers::SerializeTest(Derived(10, true, 3.0)) == 0);
    CHECK(BoostHelpers::TestHelpers::SerializePtrTest(std::make_shared<Derived>(10, true, 3.0)) == 0);
    CHECK(BoostHelpers::TestHelpers::SerializePtrTest<std::shared_ptr<Base>, Derived>(std::make_shared<Derived>(10, true, 3.0)) == 0);
}

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(Base);
SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(Derived);
