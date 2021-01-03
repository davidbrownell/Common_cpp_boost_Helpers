/////////////////////////////////////////////////////////////////////////
///
///  \file          SharedObject_UnitTest.cpp
///  \brief         Unit test for SharedObject.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-10 13:06:12
///
///  \note
///
///  \bug
///
/////////////////////////////////////////////////////////////////////////
///
///  \attention
///  Copyright David Brownell 2020-21
///  Distributed under the Boost Software License, Version 1.0. See
///  accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt.
///
/////////////////////////////////////////////////////////////////////////
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_CONSOLE_WIDTH 200
#include "../SharedObject.h"
#include <catch.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <BoostHelpers/Serialization.suffix.h>

struct Base : public BoostHelpers::SharedObject {
    int const                               I;

    CREATE(Base);

    template <typename PrivateConstructorTagT>
    Base(PrivateConstructorTagT tag, int i) :
        BoostHelpers::SharedObject(tag),
        I(i)
    {}

#define ARGS                                MEMBERS(I), BASES(BoostHelpers::SharedObject)

    NON_COPYABLE(Base);
    MOVE(Base, ARGS);
    COMPARE(Base, ARGS);
    SERIALIZATION(Base, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC_BASE, SERIALIZATION_SHARED_OBJECT));

#undef ARGS
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(Base);

struct Derived : public Base {
    bool const                              B;

    CREATE(Derived);

    template <typename PrivateConstructorTagT>
    Derived(PrivateConstructorTagT tag, int i, bool b) :
        Base(tag, i),
        B(b)
    {}

#define ARGS                                MEMBERS(B), BASES(Base)

    NON_COPYABLE(Derived);
    MOVE(Derived, ARGS);
    COMPARE(Derived, ARGS);
    SERIALIZATION(Derived, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC(Base), SERIALIZATION_SHARED_OBJECT));

#undef ARGS
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(Derived);

TEST_CASE("Standard") {
#if 0
    Base                                    base(Base::PrivateConstructorTag(), 10); // compiler error
#endif

    std::shared_ptr<Base>                   pBase(Base::Create(10));

    CHECK(pBase->I == 10);

    std::shared_ptr<Derived>                pDerived(Derived::Create(20, true));

    CHECK(pDerived->B == true);
    CHECK(pDerived->I == 20);

    std::shared_ptr<Base>                   pNewBase(pBase->CreateSharedPtr<Base>());

    CHECK(pNewBase.get() == pBase.get());

    std::shared_ptr<Derived>                pNewDerived(pDerived->CreateSharedPtr<Derived>());

    CHECK(pNewDerived.get() == pDerived.get());

    std::shared_ptr<Base>                   pBaseFromDerived(pDerived->CreateSharedPtr<Base>());

    CHECK(pBaseFromDerived.get() == pDerived.get());
    CHECK(pBaseFromDerived->I == 20);
}

template <typename T>
void Test(std::shared_ptr<T> original) {
    std::ostringstream                      out;
    boost::archive::text_oarchive           aout(out);

    original->SerializePtr(aout);
    original->SerializePtr(aout);
    out.flush();

    std::string                             result(out.str());

    UNSCOPED_INFO(result);

    std::istringstream                      in(result);
    boost::archive::text_iarchive           ain(in);
    std::shared_ptr<T> const                other1(T::DeserializePtr(ain));
    std::shared_ptr<T> const                other2(T::DeserializePtr(ain));

    CHECK(*other1 == *original);
    CHECK(*other2 == *original);
    CHECK(other1.get() == other2.get());
}

TEST_CASE("Standard Serialization") {
    Test(Base::Create(10));
    Test(Derived::Create(20, true));
}

TEST_CASE("Polymorphic Serialization") {
    std::shared_ptr<Base>                   original(Derived::Create(20, true));
    std::ostringstream                      out;
    boost::archive::text_oarchive           aout(out);

    original->SerializePtr(aout);
    original->SerializePtr(aout);
    out.flush();

    std::string                             result(out.str());

    UNSCOPED_INFO(result);

    std::istringstream                      in(result);
    boost::archive::text_iarchive           ain(in);
    std::shared_ptr<Base>                   other1(Base::DeserializePtr(ain));
    std::shared_ptr<Base>                   other2(Base::DeserializePtr(ain));

    CHECK(static_cast<Derived const &>(*other1) == static_cast<Derived const &>(*original));
    CHECK(static_cast<Derived const &>(*other2) == static_cast<Derived const &>(*original));
    CHECK(other1.get() == other2.get());
}

TEST_CASE("Serialization via boost") {
    std::shared_ptr<Derived>                original(Derived::Create(20, true));
    std::ostringstream                      out;
    boost::archive::text_oarchive           aout(out);

    aout << boost::serialization::make_nvp("test1", original);
    aout << boost::serialization::make_nvp("test2", original);
    out.flush();

    std::string                             result(out.str());

    UNSCOPED_INFO(result);

    std::istringstream                      in(result);
    boost::archive::text_iarchive           ain(in);
    std::shared_ptr<Derived>                pDerived1;
    std::shared_ptr<Derived>                pDerived2;

    ain >> boost::serialization::make_nvp("test1", pDerived1);
    ain >> boost::serialization::make_nvp("test2", pDerived2);

    CHECK(*pDerived1 == *original);
    CHECK(*pDerived2 == *original);
    CHECK(pDerived1.get() == pDerived2.get());
}

// Ideally, this code would result in a compile-time error as it is attempting to serialize and
// deserialize the objects themselves rather than via smart pointers. However, there doesn't seem
// to be a way to do this.
#if 0
TEST_CASE("Direct Serialization") {
    std::shared_ptr<Derived>                original(Derived::Create(20, true));
    std::ostringstream                      out;
    boost::archive::text_oarchive           aout(out);

    aout << boost::serialization::make_nvp("test1", *original);
    aout << boost::serialization::make_nvp("test2", *original);
    out.flush();

    std::string                             result(out.str());

    UNSCOPED_INFO(result);

    std::istringstream                      in(result);
    boost::archive::text_iarchive           ain(in);
    std::shared_ptr<Derived>                pDerived1(Derived::Create(0, false));
    std::shared_ptr<Derived>                pDerived2(Derived::Create(100, false));

    ain >> boost::serialization::make_nvp("test1", *pDerived1);
    ain >> boost::serialization::make_nvp("test2", *pDerived2);

    CHECK(*pDerived1 == *original);
    CHECK(*pDerived2 == *original);
    CHECK(pDerived1.get() == pDerived2.get());
}
#endif
