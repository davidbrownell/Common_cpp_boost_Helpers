/////////////////////////////////////////////////////////////////////////
///
///  \file          Serialization_UnitTest.hpp
///  \brief         Unit tests for Serialization.h.
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2019-03-29 14:44:39
///
///  \note
///
///  \bug
///
/////////////////////////////////////////////////////////////////////////
///
///  \attention
///  Copyright David Brownell 2019-20
///  Distributed under the Boost Software License, Version 1.0. See
///  accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt.
///
/////////////////////////////////////////////////////////////////////////
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_CONSOLE_WIDTH 200
#include "../Serialization.h"
#include <catch.hpp>

#include <CommonHelpers/Compare.h>
#include <CommonHelpers/Copy.h>
#include <CommonHelpers/Constructor.h>
#include <CommonHelpers/Move.h>

#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include "../Serialization.suffix.h"

template <typename T>
using ComparisonFunc = void (*)(T const &, T const &);

// ----------------------------------------------------------------------
template <typename OArchiveT, typename IArchiveT, typename T>
void TestImplArchive(T const &value, ComparisonFunc<T> comparison_func=nullptr) {
    std::ostringstream                      out;

    value.template Serialize<OArchiveT>(out);
    out.flush();

    std::string                             result(out.str());

    UNSCOPED_INFO(result);

    std::istringstream                      in(result);
    T const                                 other(T::template Deserialize<IArchiveT>(in));

    CHECK(CommonHelpers::Compare(other, value) == 0);
    if(comparison_func)
        comparison_func(other, value);
}

template <typename T>
void TestImpl(T const &value, ComparisonFunc<T> comparison_func=nullptr) {
    TestImplArchive<boost::archive::text_oarchive, boost::archive::text_iarchive>(value, comparison_func);
    TestImplArchive<boost::archive::xml_oarchive, boost::archive::xml_iarchive>(value, comparison_func);
}

// ----------------------------------------------------------------------
template <typename BaseT, typename OArchiveT, typename IArchiveT, typename T>
void PtrTestImplArchive(T const &value, ComparisonFunc<T> comparison_func=nullptr) {
    std::ostringstream                      out;

    value.template SerializePtr<OArchiveT>(out);
    out.flush();

    std::string                             result(out.str());

    UNSCOPED_INFO(result);

    std::istringstream                      in(result);
    std::unique_ptr<BaseT> const            other(T::template DeserializePtr<IArchiveT>(in));

    CHECK(CommonHelpers::Compare(static_cast<T const &>(*other), value) == 0);
    if(comparison_func)
        comparison_func(static_cast<T const &>(*other), value);
}

template <typename BaseT, typename T>
void PtrTestImpl(T const &value, ComparisonFunc<T> comparison_func=nullptr) {
    PtrTestImplArchive<BaseT, boost::archive::text_oarchive, boost::archive::text_iarchive>(value, comparison_func);
    PtrTestImplArchive<BaseT, boost::archive::xml_oarchive, boost::archive::xml_iarchive>(value, comparison_func);
}

// ----------------------------------------------------------------------
template <typename OArchiveT, typename IArchiveT, typename T>
void StandardTestImplArchive(T const &value, ComparisonFunc<T> comparison_func=nullptr) {
    std::ostringstream                      out;

    OArchiveT(out) << boost::serialization::make_nvp("obj", value);
    out.flush();

    std::string                             result(out.str());

    UNSCOPED_INFO(result);

    std::istringstream                      in(result);
    T                                       other;

    IArchiveT(in) >> boost::serialization::make_nvp("obj", other);

    CHECK(CommonHelpers::Compare(other, value) == 0);
    if(comparison_func)
        comparison_func(other, value);
}

template <typename T>
void StandardTestImpl(T const &value, ComparisonFunc<T> comparison_func=nullptr) {
    StandardTestImplArchive<boost::archive::text_oarchive, boost::archive::text_iarchive>(value, comparison_func);
    StandardTestImplArchive<boost::archive::xml_oarchive, boost::archive::xml_iarchive>(value, comparison_func);
}

// ----------------------------------------------------------------------
struct EmptyObj {
    CONSTRUCTOR(EmptyObj, FLAGS(CONSTRUCTOR_BASE_NUM_ARGS(0)));
    COMPARE(EmptyObj);
    SERIALIZATION(EmptyObj);
};

TEST_CASE("EmptyObj") {
    TestImpl(EmptyObj());
}

struct SingleMemberObj {
    int const a;

    CONSTRUCTOR(SingleMemberObj, a);
    COMPARE(SingleMemberObj, a);
    SERIALIZATION(SingleMemberObj, a);
};

TEST_CASE("SingleMemberObj") {
    TestImpl(SingleMemberObj(10));
}

struct SingleBaseObj : public SingleMemberObj {
    CONSTRUCTOR(SingleBaseObj, BASES(SingleMemberObj));
    COMPARE(SingleBaseObj, BASES(SingleMemberObj));
    SERIALIZATION(SingleBaseObj, BASES(SingleMemberObj));
};

TEST_CASE("SingleBaseObj") {
    TestImpl(SingleBaseObj(10));
}

struct SingleMemberSingleBaseObj : public SingleMemberObj {
    bool const b;

    CONSTRUCTOR(SingleMemberSingleBaseObj, MEMBERS(b), BASES(SingleMemberObj), FLAGS(CONSTRUCTOR_BASES_BEFORE_MEMBERS));
    COMPARE(SingleMemberSingleBaseObj, MEMBERS(b), BASES(SingleMemberObj));
    SERIALIZATION(SingleMemberSingleBaseObj, MEMBERS(b), BASES(SingleMemberObj));
};

TEST_CASE("SingleMemberSingleBaseObj") {
    TestImpl(SingleMemberSingleBaseObj(10, true));
}

struct MultiMemberObj {
    bool const b;
    char const c;

    CONSTRUCTOR(MultiMemberObj, MEMBERS(b, c));
    COMPARE(MultiMemberObj, MEMBERS(b, c));
    SERIALIZATION(MultiMemberObj, MEMBERS(b, c));
};

TEST_CASE("MultiMemberObj") {
    TestImpl(MultiMemberObj(true, 'c'));
}

struct MultiBaseObj : public SingleMemberObj, public MultiMemberObj {
    CONSTRUCTOR(MultiBaseObj, BASES(SingleMemberObj, MultiMemberObj), FLAGS(CONSTRUCTOR_BASE_ARGS_1(2)));
    COMPARE(MultiBaseObj, BASES(SingleMemberObj, MultiMemberObj));
    SERIALIZATION(MultiBaseObj, BASES(SingleMemberObj, MultiMemberObj));
};

TEST_CASE("MultiBaseObj") {
    TestImpl(MultiBaseObj(10, true, 'c'));
}

struct MultiMemberMultiBaseObj : public SingleMemberObj, public MultiMemberObj {
    double const d;
    float const f;

    CONSTRUCTOR(MultiMemberMultiBaseObj, MEMBERS(d, f), BASES(SingleMemberObj, MultiMemberObj), FLAGS(CONSTRUCTOR_BASE_ARGS_1(2), CONSTRUCTOR_BASES_BEFORE_MEMBERS));
    COMPARE(MultiMemberMultiBaseObj, MEMBERS(d, f), BASES(SingleMemberObj, MultiMemberObj));
    SERIALIZATION(MultiMemberMultiBaseObj, MEMBERS(d, f), BASES(SingleMemberObj, MultiMemberObj));
};

TEST_CASE("MultiMemberMultiBaseObj") {
    TestImpl(MultiMemberMultiBaseObj(10, true, 'c', 1.0, 2.0f));
}

TEST_CASE("std::unique_ptr") {
    StandardTestImpl(std::make_unique<MultiMemberMultiBaseObj>(10, true, 'c', 1.0, 2.0f));
}

TEST_CASE("std::shared_ptr") {
    StandardTestImpl(std::make_shared<MultiMemberMultiBaseObj>(10, true, 'c', 1.0, 2.0f));
}

TEST_CASE("std::vector") {
    StandardTestImpl(std::vector<int>{ 1, 2, 3 });
    StandardTestImpl(std::vector<MultiMemberMultiBaseObj>{ {10, true, 'c', 1.0, 2.0f}, {20, true, 'z', 3.0, 4.0f} });
    StandardTestImpl(std::vector<std::shared_ptr<MultiMemberMultiBaseObj>>{ std::make_shared<MultiMemberMultiBaseObj>(10, true, 'c', 1.0, 2.0f) });

    auto ptr(std::make_shared<MultiMemberMultiBaseObj>(10, true, 'c', 1.0, 2.0f));

    // ----------------------------------------------------------------------
    struct Internal {
        static void Compare(std::vector<std::shared_ptr<MultiMemberMultiBaseObj>> const &new_value, std::vector<std::shared_ptr<MultiMemberMultiBaseObj>> const &value) {
            CHECK(value[0].get() == value[1].get());
            CHECK(value[1].get() == value[2].get());

            CHECK(new_value[0].get() == new_value[1].get());
            CHECK(new_value[1].get() == new_value[2].get());
        }
    };

    StandardTestImpl(std::vector<std::shared_ptr<MultiMemberMultiBaseObj>>{ ptr, ptr, ptr }, Internal::Compare);
}

struct BaseObj {
    int const a;

    CONSTRUCTOR(BaseObj, MEMBERS(a));
    COMPARE(BaseObj, MEMBERS(a));
    SERIALIZATION(BaseObj, MEMBERS(a), FLAGS(SERIALIZATION_ABSTRACT));

    virtual ~BaseObj(void) = default;
    virtual void Method1(void) const = 0;
};

SERIALIZATION_POLYMORPHIC_DECLARE(BaseObj);
SERIALIZATION_POLYMORPHIC_DEFINE(BaseObj);

struct AbstractObj : public BaseObj {
    bool const b;

    CONSTRUCTOR(AbstractObj, MEMBERS(b), BASES(BaseObj), FLAGS(CONSTRUCTOR_BASES_BEFORE_MEMBERS));
    NON_COPYABLE(AbstractObj);
    NON_MOVABLE(AbstractObj);
    COMPARE(AbstractObj, MEMBERS(b), BASES(BaseObj));
    SERIALIZATION(AbstractObj, MEMBERS(b), BASES(BaseObj), FLAGS(SERIALIZATION_DATA_ONLY));

    virtual ~AbstractObj(void) = default;
    virtual void Method2(void) const = 0;
};

struct Derived1Obj : public AbstractObj {
    char const c;

    CONSTRUCTOR(Derived1Obj, MEMBERS(c), BASES(AbstractObj), FLAGS(CONSTRUCTOR_BASES_BEFORE_MEMBERS, CONSTRUCTOR_BASE_ARGS_0(2)));
    COMPARE(Derived1Obj, MEMBERS(c), BASES(AbstractObj));
    SERIALIZATION(Derived1Obj, MEMBERS(c), BASES(AbstractObj), FLAGS(SERIALIZATION_POLYMORPHIC(BaseObj)));

    ~Derived1Obj(void) override = default;

    void Method1(void) const override {}
    void Method2(void) const override {}
};

SERIALIZATION_POLYMORPHIC_DECLARE(Derived1Obj);
SERIALIZATION_POLYMORPHIC_DEFINE(Derived1Obj);

struct Derived2Obj : public AbstractObj {
    double const d;

    CONSTRUCTOR(Derived2Obj, MEMBERS(d), BASES(AbstractObj), FLAGS(CONSTRUCTOR_BASES_BEFORE_MEMBERS, CONSTRUCTOR_BASE_ARGS_0(2)));
    COMPARE(Derived2Obj, MEMBERS(d), BASES(AbstractObj));
    SERIALIZATION(Derived2Obj, MEMBERS(d), BASES(AbstractObj), FLAGS(SERIALIZATION_POLYMORPHIC(BaseObj)));

    ~Derived2Obj(void) override = default;

    void Method1(void) const override {}
    void Method2(void) const override {}
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(Derived2Obj);

TEST_CASE("Polymorphic") {
    PtrTestImpl<BaseObj>(Derived1Obj(10, true, 'c'));
    PtrTestImpl<BaseObj>(Derived2Obj(10, true, 1.0));

    // ----------------------------------------------------------------------
    using BasePtr                           = std::shared_ptr<BaseObj>;
    using Container                         = std::vector<BasePtr>;

    struct Internal {
        static void Compare(Container const &new_value, Container const &value) {
            REQUIRE(std::dynamic_pointer_cast<Derived1Obj>(value[0]));
            REQUIRE(std::dynamic_pointer_cast<Derived2Obj>(value[1]));
            REQUIRE(std::dynamic_pointer_cast<Derived1Obj>(new_value[0]));
            REQUIRE(std::dynamic_pointer_cast<Derived2Obj>(new_value[1]));

            CHECK(CommonHelpers::Compare(std::static_pointer_cast<Derived1Obj>(new_value[0]), std::static_pointer_cast<Derived1Obj>(value[0])) == 0);
            CHECK(CommonHelpers::Compare(std::static_pointer_cast<Derived2Obj>(new_value[1]), std::static_pointer_cast<Derived2Obj>(value[1])) == 0);
        }
    };
    // ----------------------------------------------------------------------

    StandardTestImpl(BasePtr(std::make_shared<Derived1Obj>(10, true, 'c')));
    StandardTestImpl(Container{ BasePtr(std::make_shared<Derived1Obj>(10, true, 'c')), BasePtr(std::make_shared<Derived2Obj>(20, false, 2.0)) }, Internal::Compare);

    // Empty objects
    StandardTestImpl(std::unique_ptr<BaseObj>());
    StandardTestImpl(std::shared_ptr<BaseObj>());
    StandardTestImpl(Container{ BasePtr() });
}

struct DataCustomConstructorObj {
    struct Value {
        int const a;

        CONSTRUCTOR(Value, a);
        COMPARE(Value, a);

        // Note that we can't use SERIALIZATION here, as its local data value will actually be a SerializationPOD object.
        template <typename ArchiveT>
        void serialize(ArchiveT &ar, const unsigned int) {
            ar & boost::serialization::make_nvp("a", make_mutable(a));
        }
    };

    Value const value;

    CONSTRUCTOR(DataCustomConstructorObj, value);
    COMPARE(DataCustomConstructorObj, value);
    SERIALIZATION(DataCustomConstructorObj, MEMBERS(value), FLAGS(SERIALIZATION_DATA_CUSTOM_CONSTRUCTOR));
};

DataCustomConstructorObj::SerializationPOD::DeserializeLocalData::DeserializeLocalData(void)
    : value(-1) {
}

TEST_CASE("DataCustomConstructorObj") {
    TestImpl(DataCustomConstructorObj(DataCustomConstructorObj::Value(10)));
}

class CustomTypesObj {
private:
    // In the following code, the 2 nibbles are serialized as a single byte
    struct SerializeLocalData {
        unsigned char a;

        SerializeLocalData(CustomTypesObj const &obj);
        NON_COPYABLE(SerializeLocalData);
        NON_MOVABLE(SerializeLocalData);

        template <typename ArchiveT>
        void Execute(ArchiveT &ar) const {
            ar << boost::serialization::make_nvp("a", a);
        }
    };

    struct DeserializeLocalData {
        unsigned char a;

        DeserializeLocalData(void) = default;
        NON_COPYABLE(DeserializeLocalData);
        MOVE(DeserializeLocalData, MEMBERS(a), FLAGS(MOVE_NO_ASSIGNMENT));

        template <typename ArchiveT>
        void Execute(ArchiveT &ar) {
            ar >> boost::serialization::make_nvp("a", a);
        }
    };
public:
    unsigned char const nibble1: 4;
    unsigned char const nibble2: 4;

    CONSTRUCTOR(CustomTypesObj, nibble1, nibble2);
    COMPARE(CustomTypesObj, nibble1, nibble2);
    SERIALIZATION(CustomTypesObj, FLAGS(SERIALIZATION_DATA_CUSTOM_CONSTRUCTOR, SERIALIZATION_DATA_CUSTOM_TYPES));
};

CustomTypesObj::SerializeLocalData::SerializeLocalData(CustomTypesObj const &obj)
    : a(static_cast<unsigned char>(obj.nibble1 | (obj.nibble2 << 4))) {
}

CustomTypesObj::CustomTypesObj(SerializationPOD::DeserializeData && data)
    : nibble1(data.local.a & 0xFF)
    , nibble2(data.local.a >> 4) {
}

TEST_CASE("CustomTypesObj") {
    TestImpl(CustomTypesObj(static_cast<unsigned char>(10), static_cast<unsigned char>(20)));
}

class EventObj {
public:
    int const a;
    bool const b;

    unsigned long ulConstructCtr = 0;
    unsigned long ulDeserializeConstructCtr = 0;

    CONSTRUCTOR(EventObj, a, b);
    COMPARE(EventObj, a, b);
    SERIALIZATION(EventObj, a, b);

private:
    friend class CommonHelpers::TypeTraits::Access;

    void DeserializeFinalConstruct(void) {
        ++ulDeserializeConstructCtr;
    }

    void FinalConstruct(void) {
        ++ulConstructCtr;
    }
};

TEST_CASE("EventObj") {
    // ----------------------------------------------------------------------
    struct Internal {
        static void Compare(EventObj const &new_value, EventObj const &value) {
            CHECK(value.ulConstructCtr == 1);
            CHECK(value.ulDeserializeConstructCtr == 0);
            CHECK(new_value.ulConstructCtr == 1);
            CHECK(new_value.ulDeserializeConstructCtr == 1);
        }
    };
    // ----------------------------------------------------------------------

    TestImpl(EventObj(10, true), Internal::Compare);
}

TEST_CASE("GetSerializedSize") {
    EventObj const                          obj(10, true);
    size_t const                            value1(obj.GetSerializedSize<boost::archive::text_oarchive>());
    size_t const                            value2(obj.GetSerializedSize<boost::archive::xml_oarchive>());

    CHECK(value1 != 0);
    CHECK(value2 != 0);
    CHECK(value1 != value2);
}

TEST_CASE("GetSerializedPtrSize") {
    Derived1Obj const                       obj(10, true, 'c');
    size_t const                            value1(obj.GetSerializedPtrSize<boost::archive::text_oarchive>());
    size_t const                            value2(obj.GetSerializedPtrSize<boost::archive::xml_oarchive>());

    CHECK(value1 != 0);
    CHECK(value2 != 0);
    CHECK(value1 != value2);
}
