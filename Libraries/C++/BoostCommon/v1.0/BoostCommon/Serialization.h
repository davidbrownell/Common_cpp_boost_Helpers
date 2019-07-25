/////////////////////////////////////////////////////////////////////////
///
///  \file          Serialization.h
///  \brief         Contains the Serialization namespace
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2019-03-24 20:13:20
///
///  \note
///
///  \bug
///
/////////////////////////////////////////////////////////////////////////
///
///  \attention
///  Copyright David Brownell 2019
///  Distributed under the Boost Software License, Version 1.0. See
///  accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt.
///
/////////////////////////////////////////////////////////////////////////
#pragma once

#include <CommonHelpers/TypeTraits.h>
#include <CommonHelpers/Details/PreprocessorObjectFunctionalityImpl.h>

#include <boost/iostreams/device/null.hpp>
#include <boost/iostreams/filter/counter.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/level.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/traits.hpp>

namespace BoostCommon {
namespace Serialization {

// ----------------------------------------------------------------------
// |
// |  Preprocessor Definitions
// |
// ----------------------------------------------------------------------

// clang-format off

/////////////////////////////////////////////////////////////////////////
///  \def           SERIALIZATION
///  \brief         Implements Serialize and Deserialize methods while still
///                 maintaining correct-by-construction semantics (traditional
///                 serialization requires objects that are default constructible).
///
///                 SPECIAL NOTE:
///                     The boost serialization library makes heavy use of template
///                     overrides and specializations. Due to the way C++ processes
///                     these overrides, the following header file must be included
///                     after all types and archives have been defined:
///
///                         #include <BoostCommon/Serialization.suffix.h>
///
///                 Creates the following methods:
///
///                     template <typename ArchiveT> ArchiveT & Serialize(ArchiveT &ar) const;
///                     template <typename Archive, typename CharT, typename TraitsT> std::basic_ostream<CharT, TraitsT> & Serialize(std::basic_ostream<CharT, TraitsT> &s) const;
///                     template <typename Archive, typename CharT, typename TraitsT> std::basic_streambuf<CharT, TraitsT> & Serialize(std::basic_streambuf<CharT, TraitsT> &s) const;
///                     template <typename ArchiveT> size_t GetSerializedSize(void) const;
///                     template <typename ArchiveT> static ClassName Deserialize(ArchiveT &ar);
///                     template <typename ArchiveT, typename CharT, typename TraitsT> static ClassName Deserialize(std::basic_istream<CharT, TraitsT> &s);
///                     template <typename ArchiveT, typename CharT, typename TraitsT> static ClassName Deserialize(std::basic_streambuf<CharT, TraitsT> &s);
///
///                 If the object is polymorphic, the following methods will be created as well:
///
///                     template <typename ArchiveT> ArchiveT & SerializePtr(ArchiveT &ar) const;
///                     template <typename Archive, typename CharT, typename TraitsT> std::basic_ostream<CharT, TraitsT> & SerializePtr(std::basic_ostream<CharT, TraitsT> &s) const;
///                     template <typename Archive, typename CharT, typename TraitsT> std::basic_streambuf<CharT, TraitsT> & SerializePtr(std::basic_streambuf<CharT, TraitsT> &s) const;
///                     template <typename ArchiveT> size_t GetSerializedPtrSize(void) const;
///                     template <typename ArchiveT> static std::unique_ptr<PolymorphicBaseName> DeserializePtr(ArchiveT &ar);
///                     template <typename ArchiveT, typename CharT, typename TraitsT> static std::unique_ptr<PolymorphicBaseName> DeserializePtr(std::basic_istream<CharT, TraitsT> &s);
///                     template <typename ArchiveT, typename CharT, typename TraitsT> static std::unique_ptr<PolymorphicBaseName> DeserializePtr(std::basic_streambuf<CharT, TraitsT> &s);
///
///                 The following methods will be called if they exist:
///                     void DeserializeFinalConstruct(void);
///                     void FinalConstruct(void);
///
#define SERIALIZATION(ClassName, ...)       PPOFImpl(SERIALIZATION_Impl, (ClassName), __VA_ARGS__)

// Context macros that can be used with SERIALIZATION

// ----------------------------------------------------------------------
// |  Class-based flags

#define SERIALIZATION_ABSTRACT                          0                   ///< Adds virtual abstract SerializePtr and DeserializePtr methods
#define SERIALIZATION_POLYMORPHIC(BaseClassName)        (1, BaseClassName)  ///< Adds virtual SerializePtr and DeserializePtr methods

/// Implements serialization mechanics so the class can be used within a class
/// hierarchy supporting serialization, but does not implement methods that would
/// allow this class to be serialized in isolation. Use this when the class is
/// abstract, but implemented in terms of a base class that is already declared 
/// with the SERIALIZATION_ABSTRACT flag.
#define SERIALIZATION_DATA_ONLY                         2

// ----------------------------------------------------------------------
// |  Data-based flags

/// Declare the deserialization data constructor ("SerializationPOD::DeserializeLocalImpl::DeserializeLocalImpl(void)"),
/// but do not provide a default implementation (where the default implementation is empty). This is useful when 
/// there are members within the class that are serializable but not default constructible;  therefore requiring 
/// custom construction semantics.
///
#define SERIALIZATION_DATA_CUSTOM_CONSTRUCTOR           3

/// Serialize local data according to the provided types. Use this when the serialization mechanics
/// required by the object are too complex for the default behavior (where the default implementation 
/// is implemented in terms of member-by-member serialization). 
/// 
/// Requires the following classes and functionality to be implemented:
///
///     SerializeLocalData:
///         - SerializeLocalData(ClassName const &);
///         - template <typename ArchiveT> void Execute(ArchiveT &ar) const;
///         - Non-movable
///         - Non-copyable
/// 
///     DeserializeLocalData:
///         - template <typename ArchiveT> void Execute(ArchiveT &ar);
///         - Default constructible
///         - Move constructor
///         - No move assignment
///         - Non-copyable
///
///     ClassName(SerializationPOD::DeserializeData && data);
///
#define SERIALIZATION_DATA_CUSTOM_TYPES                 4

#define __NUM_SERIALIZATION_FLAGS                       5

/////////////////////////////////////////////////////////////////////////
///  \def           SERIALIZATION_POLYMORPHIC_DECLARE
///  \brief         The boost serialization library requires that any object serialized
///                 through a virtual base class take special steps to register the
///                 desired type with the serialization engine. Interestingly, the
///                 registration system is very particular when it comes to when
///                 and where types are registered. Registration is a 2-step process:
/// 
///                     1) Header files must register types via BOOST_CLASS_EXPORT_KEY
///                     2) A SINGLE compilation unit must register via BOOST_CLASS_EXPORT_IMPLEMENT
/// 
///                 This macro (along with SERIALIZATION_POLYMORPHIC_DEFINE) abstract
///                 the details of this process, while providing compile time checks
///                 to ensure that derived types are registered as necessary.
/// 
///                 Note that this macro must appear after the object has been declared,
///                 and must appear in the root namespace.
///
#define SERIALIZATION_POLYMORPHIC_DECLARE(FullyQualifiedObjectName)         SERIALIZATION_POLYMORPHIC_DECLARE_Impl(FullyQualifiedObjectName)

/////////////////////////////////////////////////////////////////////////
///  \def           SERIALIZATION_POLYMORPHIC_DEFINE
///  \brief         See SERIALIZATION_POLYMORPHIC_DECLARE
///
#define SERIALIZATION_POLYMORPHIC_DEFINE(FullyQualifiedObjectName)          SERIALIZATION_POLYMORPHIC_DEFINE_Impl(FullyQualifiedObjectName)

/////////////////////////////////////////////////////////////////////////
///  \def           SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE
///  \brief         Invokes SERIALIZATION_POLYMORPHIC_DECLARE and 
///                 SERIALIZATION_POLYMORPHIC_DEFINE.
///  
#define SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(FullyQualifiedObjectName)      SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE_Impl(FullyQualifiedObjectName)

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// |
// |  Implementation
// |
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

#define SERIALIZATION_Impl(NameTuple, Members, Bases, Methods, Flags)                                                                                                                                                                                                                                 \
    static_assert(BOOST_PP_IIF(BOOST_VMD_IS_EMPTY(Methods BOOST_PP_EMPTY()), true, false), "The macro does not support methods");                                                                                                                                                                     \
    SERIALIZATION_Impl_Delay(SERIALIZATION_Impl2) (BOOST_PP_TUPLE_ELEM(0, NameTuple), BOOST_PP_NOT(BOOST_VMD_IS_EMPTY(Members BOOST_PP_EMPTY())), Members, BOOST_PP_NOT(BOOST_VMD_IS_EMPTY(Bases BOOST_PP_EMPTY())), Bases, CONTEXT_TUPLE_TO_INITIALIZED_TUPLE(__NUM_SERIALIZATION_FLAGS, 0, Flags))

#define SERIALIZATION_Impl_Delay(x)         BOOST_PP_CAT(x, SERIALIZATION_Impl_Empty())
#define SERIALIZATION_Impl_Empty()

#if (defined _MSC_VER && !defined __clang__)
    // MSVC doesn't like BOOST_PP_EXPAND
#   define SERIALIZATION_Impl2(Name, HasMembers, Members, HasBases, Bases, Flags)   SERIALIZATION_PreInvoke BOOST_PP_LPAREN() Name BOOST_PP_COMMA() HasMembers BOOST_PP_COMMA() Members BOOST_PP_COMMA() HasBases BOOST_PP_COMMA() Bases BOOST_PP_COMMA() BOOST_PP_TUPLE_ENUM(Flags) BOOST_PP_RPAREN()
#else
#   define SERIALIZATION_Impl2(Name, HasMembers, Members, HasBases, Bases, Flags)   BOOST_PP_EXPAND(SERIALIZATION_PreInvoke BOOST_PP_LPAREN() Name BOOST_PP_COMMA() HasMembers BOOST_PP_COMMA() Members BOOST_PP_COMMA() HasBases BOOST_PP_COMMA() Bases BOOST_PP_COMMA() BOOST_PP_TUPLE_ENUM(Flags) BOOST_PP_RPAREN())
#endif

#define SERIALIZATION_Impl2_Delay(x)        BOOST_PP_CAT(x, SERIALIZATION_Impl2_Empty())
#define SERIALIZATION_Impl2_Empty()

#define SERIALIZATION_PreInvoke(Name, HasMembers, Members, HasBases, Bases, IsAbstract, OptionalPolymorphicBaseName, IsDataOnly, HasDeserializeDataCustomCtor, HasCustomLocalDataTypes)                     SERIALIZATION_PreInvoke2(Name, HasMembers, Members, HasBases, Bases, IsAbstract, BOOST_PP_OR(IsAbstract, BOOST_PP_NOT(BOOST_VMD_IS_NUMBER(OptionalPolymorphicBaseName))), OptionalPolymorphicBaseName, IsDataOnly, HasDeserializeDataCustomCtor, HasCustomLocalDataTypes)
#define SERIALIZATION_PreInvoke2(Name, HasMembers, Members, HasBases, Bases, IsAbstract, IsPolymorphic, OptionalPolymorphicBaseName, IsDataOnly, HasDeserializeDataCustomCtor, HasCustomLocalDataTypes)     SERIALIZATION_Invoke(Name, HasMembers, Members, HasBases, Bases, IsAbstract, IsPolymorphic, BOOST_PP_IIF(BOOST_PP_OR(IsAbstract, IsPolymorphic), SERIALIZATION_PreInvoke2_PolymorphicName, BOOST_VMD_EMPTY)(Name, IsAbstract, OptionalPolymorphicBaseName), IsDataOnly, HasDeserializeDataCustomCtor, HasCustomLocalDataTypes)
#define SERIALIZATION_PreInvoke2_PolymorphicName(Name, IsAbstract, OptionalPolymorphicBaseName)                                                                                                             BOOST_PP_IIF(IsAbstract, BOOST_PP_IDENTITY(Name), BOOST_PP_IDENTITY(OptionalPolymorphicBaseName))()

// ----------------------------------------------------------------------
#define SERIALIZATION_Invoke(Name, HasMembers, Members, HasBases, Bases, IsAbstract, IsPolymorphic, PolymorphicBaseName, IsDataOnly, HasDeserializeDataCustomCtor, HasCustomLocalDataTypes)         \
    public:                                                                                                                                                                                         \
        SERIALIZATION_Impl_PODImpl(Name, HasMembers, Members, HasBases, Bases, IsAbstract, IsPolymorphic, PolymorphicBaseName, IsDataOnly, HasDeserializeDataCustomCtor, HasCustomLocalDataTypes)   \
                                                                                                                                                                                                    \
        Name(typename SerializationPOD::DeserializeData && data)                                                                                                                                    \
            BOOST_PP_IIF(HasCustomLocalDataTypes, SERIALIZATION_Invoke_CustomCtor, SERIALIZATION_Invoke_DefaultCtor)(Name, HasMembers, Members, HasBases, Bases)                                    \
                                                                                                                                                                                                    \
        BOOST_PP_IIF(BOOST_PP_AND(BOOST_PP_NOT(IsDataOnly), BOOST_PP_NOT(IsAbstract)), SERIALIZATION_Invoke_Methods, BOOST_VMD_EMPTY)(Name)                                                         \
        BOOST_PP_IIF(BOOST_PP_AND(BOOST_PP_NOT(IsDataOnly), IsPolymorphic), SERIALIZATION_Invoke_PtrMethods, BOOST_VMD_EMPTY)(Name, IsAbstract, PolymorphicBaseName)

#define SERIALIZATION_Invoke_CustomCtor(Name, HasMembers, Members, HasBases, Base)  ;

#define SERIALIZATION_Invoke_DefaultCtor(Name, HasMembers, Members, HasBases, Bases)                        \
        BOOST_PP_IIF(BOOST_PP_OR(HasMembers, HasBases), BOOST_PP_IDENTITY(:), BOOST_VMD_EMPTY)()            \
        BOOST_PP_IIF(HasBases, SERIALIZATION_Invoke_DefaultCtor_Bases, BOOST_VMD_EMPTY)(Bases)              \
        BOOST_PP_COMMA_IF(BOOST_PP_AND(HasMembers, HasBases))                                               \
        BOOST_PP_IIF(HasMembers, SERIALIZATION_Invoke_DefaultCtor_Members, BOOST_VMD_EMPTY)(Name, Members)  \
    {                                                                                                       \
        (data);                                                                                             \
                                                                                                            \
        CommonHelpers::TypeTraits::Access::DeserializeFinalConstruct(*this);                                \
        CommonHelpers::TypeTraits::Access::FinalConstruct(*this);                                           \
    }

#define SERIALIZATION_Invoke_DefaultCtor_Bases(Bases)                       BOOST_PP_TUPLE_FOR_EACH_ENUM_I(SERIALIZATION_Invoke_DefaultCtor_Bases_Macro, _, Bases)
#define SERIALIZATION_Invoke_DefaultCtor_Bases_Macro(r, _, Index, Base)     Base(std::move(data. BOOST_PP_CAT(base, Index)))

#define SERIALIZATION_Invoke_DefaultCtor_Members(Name, Members)             BOOST_PP_TUPLE_FOR_EACH_ENUM(SERIALIZATION_Invoke_DefaultCtor_Members_Macro, Name, Members)
#define SERIALIZATION_Invoke_DefaultCtor_Members_Macro(r, Name, Member)     Member(BoostCommon::Serialization::Details::CreateMember<decltype(Name::Member)>(std::move(data.local. Member)))

#define SERIALIZATION_Invoke_Methods(Name)                                                                              \
    SerializationPOD CreateSerializationPOD(void) const {                                                               \
        return SerializationPOD(*this);                                                                                 \
    }                                                                                                                   \
                                                                                                                        \
    template <typename ArchiveT>                                                                                        \
    ArchiveT & Serialize(ArchiveT &ar) const {                                                                          \
        return Serialize(ar, BOOST_PP_STRINGIZE(Name));                                                                 \
    }                                                                                                                   \
                                                                                                                        \
    template <typename ArchiveT>                                                                                        \
    ArchiveT & Serialize(ArchiveT &ar, char const *tag) const {                                                         \
        SerializationPOD const              pod(CreateSerializationPOD());                                              \
                                                                                                                        \
        ar << boost::serialization::make_nvp(tag, pod);                                                                 \
        return ar;                                                                                                      \
    }                                                                                                                   \
                                                                                                                        \
    template <typename ArchiveT, typename CharT, typename TraitsT>                                                      \
    std::basic_ostream<CharT, TraitsT> & Serialize(std::basic_ostream<CharT, TraitsT> &s) const {                       \
        return Serialize<ArchiveT>(s, BOOST_PP_STRINGIZE(Name));                                                        \
    }                                                                                                                   \
                                                                                                                        \
    template <typename ArchiveT, typename CharT, typename TraitsT>                                                      \
    std::basic_ostream<CharT, TraitsT> & Serialize(std::basic_ostream<CharT, TraitsT> &s, char const *tag) const {      \
        ArchiveT                            ar(s);                                                                      \
                                                                                                                        \
        Serialize(ar, tag);                                                                                             \
        return s;                                                                                                       \
    }                                                                                                                   \
                                                                                                                        \
    template <typename ArchiveT, typename CharT, typename TraitsT>                                                      \
    std::basic_streambuf<CharT, TraitsT> & Serialize(std::basic_streambuf<CharT, TraitsT> &s) const {                   \
        return Serialize<ArchiveT>(s, BOOST_PP_STRINGIZE(Name));                                                        \
    }                                                                                                                   \
                                                                                                                        \
    template <typename ArchiveT, typename CharT, typename TraitsT>                                                      \
    std::basic_streambuf<CharT, TraitsT> & Serialize(std::basic_streambuf<CharT, TraitsT> &s, char const *tag) const {  \
        ArchiveT                            ar(s);                                                                      \
                                                                                                                        \
        Serialize(ar, tag);                                                                                             \
        return s;                                                                                                       \
    }                                                                                                                   \
                                                                                                                        \
    template <typename ArchiveT>                                                                                        \
    size_t GetSerializedSize(void) const {                                                                              \
        boost::iostreams::filtering_ostream out(boost::iostreams::counter() | boost::iostreams::null_sink());           \
                                                                                                                        \
        Serialize<ArchiveT>(out);                                                                                       \
        out.flush();                                                                                                    \
                                                                                                                        \
        return out.component<boost::iostreams::counter>(0)->characters();                                               \
    }                                                                                                                   \
                                                                                                                        \
    template <typename ArchiveT>                                                                                        \
    static Name Deserialize(ArchiveT ar) {                                                                              \
        return Deserialize(ar, BOOST_PP_STRINGIZE(Name));                                                               \
    }                                                                                                                   \
                                                                                                                        \
    template <typename ArchiveT>                                                                                        \
    static Name Deserialize(ArchiveT &ar, char const *tag) {                                                            \
        SerializationPOD                    pod;                                                                        \
                                                                                                                        \
        ar >> boost::serialization::make_nvp(tag, pod);                                                                 \
        return Name(pod.Construct());                                                                                   \
    }                                                                                                                   \
                                                                                                                        \
    template <typename ArchiveT, typename CharT, typename TraitsT>                                                      \
    static Name Deserialize(std::basic_istream<CharT, TraitsT> &s) {                                                    \
        return Deserialize<ArchiveT>(s, BOOST_PP_STRINGIZE(Name));                                                      \
    }                                                                                                                   \
                                                                                                                        \
    template <typename ArchiveT, typename CharT, typename TraitsT>                                                      \
    static Name Deserialize(std::basic_istream<CharT, TraitsT> &s, char const *tag) {                                   \
        ArchiveT                            ar(s);                                                                      \
                                                                                                                        \
        return Deserialize(ar, tag);                                                                                    \
    }                                                                                                                   \
                                                                                                                        \
    template <typename ArchiveT, typename CharT, typename TraitsT>                                                      \
    static Name Deserialize(std::basic_streambuf<CharT, TraitsT> &s) {                                                  \
        return Deserialize<ArchiveT>(s, BOOST_PP_STRINGIZE(Name));                                                      \
    }                                                                                                                   \
                                                                                                                        \
    template <typename ArchiveT, typename CharT, typename TraitsT>                                                      \
    static Name Deserialize(std::basic_streambuf<CharT, TraitsT> &s, char const *tag) {                                 \
        ArchiveT                            ar(s);                                                                      \
                                                                                                                        \
        return Deserialize(ar, tag);                                                                                    \
    }

#define SERIALIZATION_Invoke_PtrMethods(Name, IsAbstract, PolymorphicBaseName)                                                                                          \
    using PolymorphicSerializationUniquePtr             = std::unique_ptr<PolymorphicBaseName>;                                                                         \
    using PolymorphicSerializationPODUniquePtr          = std::unique_ptr<PolymorphicBaseName::SerializationPOD>;                                                       \
                                                                                                                                                                        \
    /* Functionality that helps to ensure that the require mechanics necessary to support polymorphic */                                                                \
    /* serialization with the boost library are in place for this class. */                                                                                             \
    inline static void SERIALIZATION_POLYMORPHIC_DECLARE_Impl_Func_Name() (void);                                                                                       \
    static void SERIALIZATION_POLYMORPHIC_DEFINE_Impl_Func_Name() (void);                                                                                               \
                                                                                                                                                                        \
    virtual void RegisterSerializationTypes(void) const                                                                                                                 \
        BOOST_PP_IIF(IsAbstract, SERIALIZATION_Invoke_PtrMethods_Register_Abstract, SERIALIZATION_Invoke_PtrMethods_Regster_Concrete)(Name, PolymorphicBaseName)        \
                                                                                                                                                                        \
    virtual PolymorphicSerializationPODUniquePtr __CreateSerializationPODPtrImpl(PolymorphicBaseName const &base) const                                                 \
        BOOST_PP_IIF(IsAbstract, SERIALIZATION_Invoke_PtrMethods_Create_Abstract, SERIALIZATION_Invoke_PtrMethods_Create_NotAbstract)(Name, PolymorphicBaseName)        \
                                                                                                                                                                        \
    PolymorphicSerializationPODUniquePtr CreateSerializationPODPtr(PolymorphicBaseName const &base) const {                                                             \
        SERIALIZATION_POLYMORPHIC_DECLARE_Impl_Func_Name() ();                                                                                                          \
        SERIALIZATION_POLYMORPHIC_DEFINE_Impl_Func_Name() ();                                                                                                           \
                                                                                                                                                                        \
        return __CreateSerializationPODPtrImpl(base);                                                                                                                   \
    }                                                                                                                                                                   \
                                                                                                                                                                        \
    template <typename ArchiveT>                                                                                                                                        \
    ArchiveT & SerializePtr(ArchiveT &ar) const {                                                                                                                       \
        return SerializePtr(ar, BoostCommon::Serialization::Details::ScrubSerializationName(BOOST_PP_STRINGIZE(BOOST_PP_CAT(PolymorphicBaseName, Ptr))));               \
    }                                                                                                                                                                   \
                                                                                                                                                                        \
    template <typename ArchiveT>                                                                                                                                        \
    ArchiveT & SerializePtr(ArchiveT &ar, char const *tag) const {                                                                                                      \
        PolymorphicSerializationPODUniquePtr const      pPod(CreateSerializationPODPtr(*this));                                                                         \
                                                                                                                                                                        \
        ar << boost::serialization::make_nvp(tag, pPod);                                                                                                                \
        return ar;                                                                                                                                                      \
    }                                                                                                                                                                   \
                                                                                                                                                                        \
    template <typename ArchiveT, typename CharT, typename TraitsT>                                                                                                      \
    std::basic_ostream<CharT, TraitsT> & SerializePtr(std::basic_ostream<CharT, TraitsT> &s) const {                                                                    \
        return SerializePtr<ArchiveT>(s, BoostCommon::Serialization::Details::ScrubSerializationName(BOOST_PP_STRINGIZE(BOOST_PP_CAT(PolymorphicBaseName, Ptr))));      \
    }                                                                                                                                                                   \
                                                                                                                                                                        \
    template <typename ArchiveT, typename CharT, typename TraitsT>                                                                                                      \
    std::basic_ostream<CharT, TraitsT> & SerializePtr(std::basic_ostream<CharT, TraitsT> &s, char const *tag) const {                                                   \
        ArchiveT                            ar(s);                                                                                                                      \
                                                                                                                                                                        \
        SerializePtr(ar, tag);                                                                                                                                          \
        return s;                                                                                                                                                       \
    }                                                                                                                                                                   \
                                                                                                                                                                        \
    template <typename ArchiveT, typename CharT, typename TraitsT>                                                                                                      \
    std::basic_streambuf<CharT, TraitsT> & SerializePtr(std::basic_streambuf<CharT, TraitsT> &s) const {                                                                \
        return SerializePtr<ArchiveT>(s, BoostCommon::Serialization::Details::ScrubSerializationName(BOOST_PP_STRINGIZE(BOOST_PP_CAT(PolymorphicBaseName, Ptr))));      \
    }                                                                                                                                                                   \
                                                                                                                                                                        \
    template <typename ArchiveT, typename CharT, typename TraitsT>                                                                                                      \
    std::basic_streambuf<CharT, TraitsT> & SerializePtr(std::basic_streambuf<CharT, TraitsT> &s, char const *tag) const {                                               \
        ArchiveT                            ar(s);                                                                                                                      \
                                                                                                                                                                        \
        SerializePtr(ar, tag);                                                                                                                                          \
        return s;                                                                                                                                                       \
    }                                                                                                                                                                   \
                                                                                                                                                                        \
    template <typename ArchiveT>                                                                                                                                        \
    size_t GetSerializedPtrSize(void) const {                                                                                                                           \
        boost::iostreams::filtering_ostream             out(boost::iostreams::counter() | boost::iostreams::null_sink());                                               \
                                                                                                                                                                        \
        SerializePtr<ArchiveT>(out);                                                                                                                                    \
        out.flush();                                                                                                                                                    \
                                                                                                                                                                        \
        return out.component<boost::iostreams::counter>(0)->characters();                                                                                               \
    }                                                                                                                                                                   \
                                                                                                                                                                        \
    template <typename ArchiveT>                                                                                                                                        \
    static PolymorphicSerializationUniquePtr DeserializePtr(ArchiveT &ar) {                                                                                             \
        return DeserializePtr(ar, BoostCommon::Serialization::Details::ScrubSerializationName(BOOST_PP_STRINGIZE(BOOST_PP_CAT(PolymorphicBaseName, Ptr))));             \
    }                                                                                                                                                                   \
                                                                                                                                                                        \
    template <typename ArchiveT>                                                                                                                                        \
    static PolymorphicSerializationUniquePtr DeserializePtr(ArchiveT &ar, char const *tag) {                                                                            \
        PolymorphicSerializationPODUniquePtr            pPod;                                                                                                           \
                                                                                                                                                                        \
        ar >> boost::serialization::make_nvp(tag, pPod);                                                                                                                \
        return pPod->ConstructPtr();                                                                                                                                    \
    }                                                                                                                                                                   \
                                                                                                                                                                        \
    template <typename ArchiveT, typename CharT, typename TraitsT>                                                                                                      \
    static PolymorphicSerializationUniquePtr DeserializePtr(std::basic_istream<CharT, TraitsT> &s) {                                                                    \
        return DeserializePtr<ArchiveT>(s, BoostCommon::Serialization::Details::ScrubSerializationName(BOOST_PP_STRINGIZE(BOOST_PP_CAT(PolymorphicBaseName, Ptr))));    \
    }                                                                                                                                                                   \
                                                                                                                                                                        \
    template <typename ArchiveT, typename CharT, typename TraitsT>                                                                                                      \
    static PolymorphicSerializationUniquePtr DeserializePtr(std::basic_istream<CharT, TraitsT> &s, char const *tag) {                                                   \
        ArchiveT                            ar(s);                                                                                                                      \
                                                                                                                                                                        \
        return DeserializePtr(ar, tag);                                                                                                                                 \
    }                                                                                                                                                                   \
                                                                                                                                                                        \
    template <typename ArchiveT, typename CharT, typename TraitsT>                                                                                                      \
    static PolymorphicSerializationUniquePtr DeserializePtr(std::basic_streambuf<CharT, TraitsT> &s) {                                                                  \
        return DeserializePtr<ArchiveT>(s, BoostCommon::Serialization::Details::ScrubSerializationName(BOOST_PP_STRINGIZE(BOOST_PP_CAT(PolymorphicBaseName, Ptr))));    \
    }                                                                                                                                                                   \
                                                                                                                                                                        \
    template <typename ArchiveT, typename CharT, typename TraitsT>                                                                                                      \
    static PolymorphicSerializationUniquePtr DeserializePtr(std::basic_streambuf<CharT, TraitsT> &s, char const *tag) {                                                 \
        ArchiveT                            ar(s);                                                                                                                      \
                                                                                                                                                                        \
        return DeserializePtr(ar, tag);                                                                                                                                 \
    }

#define SERIALIZATION_Invoke_PtrMethods_Create_Abstract(Name, PolymorphicBaseName)      = 0;

#define SERIALIZATION_Invoke_PtrMethods_Create_NotAbstract(Name, PolymorphicBaseName)                           \
    {                                                                                                           \
        SERIALIZATION_POLYMORPHIC_DECLARE_Impl_Func_Name() ();                                                  \
        SERIALIZATION_POLYMORPHIC_DEFINE_Impl_Func_Name() ();                                                   \
                                                                                                                \
        std::unique_ptr<Name::SerializationPOD>         pResult(std::make_unique<SerializationPOD>(*this));     \
                                                                                                                \
        pResult->SetOriginalBaseClass(base);                                                                    \
                                                                                                                \
        return PolymorphicSerializationPODUniquePtr(pResult.release());                                         \
    }

#define SERIALIZATION_Invoke_PtrMethods_Register_Abstract(Name, PolymorphicBaseName)    = 0;

#define SERIALIZATION_Invoke_PtrMethods_Regster_Concrete(Name, PolymorphicBaseName)     \
    {                                                                                   \
        boost::serialization::void_cast_register(                                       \
            static_cast<Name const *>(nullptr),                                         \
            static_cast<PolymorphicBaseName const *>(nullptr)                           \
        );                                                                              \
    }

// ----------------------------------------------------------------------
// TODO: Figure out a way to not allocate memory in SerializationPOD
#define SERIALIZATION_Impl_PODImpl(Name, HasMembers, Members, HasBases, Bases, IsAbstract, IsPolymorphic, PolymorphicBaseName, IsDataOnly, HasDeserializeDataCustomCtor, HasCustomLocalDataTypes)   \
    class SerializationPOD :                                                                                                                                                                        \
        public virtual boost::serialization::basic_traits                                                                                                                                           \
        BOOST_PP_IIF(HasBases, SERIALIZATION_Impl_PODImpl_BaseClasses, BOOST_VMD_EMPTY)(Bases)                                                                                                      \
    {                                                                                                                                                                                               \
    public:                                                                                                                                                                                         \
        struct SerializationPODTag {};                                                                                                                                                              \
                                                                                                                                                                                                    \
        using level                         = boost::mpl::int_<boost::serialization::object_serializable>;                                                                                          \
        using tracking                      = boost::mpl::int_<boost::serialization::track_never>;                                                                                                  \
        using version                       = boost::mpl::int_<0>;                                                                                                                                  \
        using type_info_implementation      = boost::serialization::extended_type_info_impl<SerializationPOD>;                                                                                      \
        using is_wrapper                    = boost::mpl::false_;                                                                                                                                   \
                                                                                                                                                                                                    \
        BOOST_PP_IIF(HasCustomLocalDataTypes, BOOST_VMD_EMPTY, SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes)(Name, HasMembers, Members, HasDeserializeDataCustomCtor)                           \
                                                                                                                                                                                                    \
        struct SerializeData {                                                                                                                                                                      \
            BOOST_PP_IIF(IsPolymorphic, SERIALIZATION_Impl_PODImpl_SerializePolymorphicMembers, BOOST_VMD_EMPTY)(PolymorphicBaseName)                                                               \
                                                                                                                                                                                                    \
            BOOST_PP_IIF(HasBases, SERIALIZATION_Impl_PODImpl_Serialize_Bases, BOOST_VMD_EMPTY)(Bases)                                                                                              \
            SerializeLocalData const        local;                                                                                                                                                  \
                                                                                                                                                                                                    \
            SerializeData(Name const &obj) :                                                                                                                                                        \
                BOOST_PP_IIF(HasBases, SERIALIZATION_Impl_PODImpl_Serialize_Ctor, BOOST_VMD_EMPTY)(Bases)                                                                                           \
                local(obj) {                                                                                                                                                                        \
            }                                                                                                                                                                                       \
                                                                                                                                                                                                    \
            SerializeData(SerializeData &&) = delete;                                                                                                                                               \
            SerializeData & operator =(SerializeData &&) = delete;                                                                                                                                  \
                                                                                                                                                                                                    \
            SerializeData(SerializeData const &) = delete;                                                                                                                                          \
            SerializeData & operator =(SerializeData const &) = delete;                                                                                                                             \
        };                                                                                                                                                                                          \
                                                                                                                                                                                                    \
        struct DeserializeData {                                                                                                                                                                    \
            BOOST_PP_IIF(HasBases, SERIALIZATION_Impl_PODImpl_Deserialize_Bases, BOOST_VMD_EMPTY)(Bases)                                                                                            \
            DeserializeLocalData            local;                                                                                                                                                  \
                                                                                                                                                                                                    \
            DeserializeData(void) {                                                                                                                                                                 \
            }                                                                                                                                                                                       \
                                                                                                                                                                                                    \
            DeserializeData(DeserializeData && other) :                                                                                                                                             \
                BOOST_PP_IIF(HasBases, SERIALIZATION_Impl_PODImpl_Deserialize_Ctor, BOOST_VMD_EMPTY)(Bases)                                                                                         \
                local(std::move(other.local))                                                                                                                                                       \
            {}                                                                                                                                                                                      \
                                                                                                                                                                                                    \
            DeserializeData & operator =(DeserializeData &&) = delete;                                                                                                                              \
                                                                                                                                                                                                    \
            DeserializeData(DeserializeData const &) = delete;                                                                                                                                      \
            DeserializeData & operator =(DeserializeData const &) = delete;                                                                                                                         \
        };                                                                                                                                                                                          \
                                                                                                                                                                                                    \
        SerializationPOD(Name const &obj) :                                                                                                                                                         \
            BOOST_PP_IIF(HasBases, SERIALIZATION_Impl_PODImpl_DelayInitCtor, BOOST_VMD_EMPTY)(Bases)                                                                                                \
            _serialize_data_storage(std::make_unique<SerializeData>(obj))                                                                                                                           \
        {                                                                                                                                                                                           \
            Init(*_serialize_data_storage);                                                                                                                                                         \
        }                                                                                                                                                                                           \
                                                                                                                                                                                                    \
        SerializationPOD(void) :                                                                                                                                                                    \
            BOOST_PP_IIF(HasBases, SERIALIZATION_Impl_PODImpl_DelayInitCtor, BOOST_VMD_EMPTY)(Bases)                                                                                                \
            _deserialize_data_storage(std::make_unique<DeserializeData>())                                                                                                                          \
        {                                                                                                                                                                                           \
            Init(*_deserialize_data_storage);                                                                                                                                                       \
        }                                                                                                                                                                                           \
                                                                                                                                                                                                    \
        SerializationPOD(SerializationPOD && other) :                                                                                                                                               \
            BOOST_PP_IIF(HasBases, SERIALIZATION_Impl_PODImpl_DelayInitCtor, BOOST_VMD_EMPTY)(Bases)                                                                                                \
            _deserialize_data_storage(std::move(make_mutable(other._deserialize_data_storage)))                                                                                                     \
        {                                                                                                                                                                                           \
            Init(*_deserialize_data_storage);                                                                                                                                                       \
        }                                                                                                                                                                                           \
                                                                                                                                                                                                    \
        SerializationPOD & operator =(SerializationPOD &&) = delete;                                                                                                                                \
                                                                                                                                                                                                    \
        SerializationPOD(SerializationPOD const &) = delete;                                                                                                                                        \
        SerializationPOD & operator =(SerializationPOD const &) = delete;                                                                                                                           \
                                                                                                                                                                                                    \
        virtual ~SerializationPOD(void) = default;                                                                                                                                                  \
                                                                                                                                                                                                    \
        BOOST_PP_IIF(BOOST_PP_AND(BOOST_PP_NOT(IsDataOnly), BOOST_PP_NOT(IsAbstract)), SERIALIZATION_Impl_PODImpl_Construct, BOOST_VMD_EMPTY)(Name)                                                 \
        BOOST_PP_IIF(BOOST_PP_AND(BOOST_PP_NOT(IsDataOnly), IsPolymorphic), SERIALIZATION_Impl_PODImpl_ConstructPtr, BOOST_VMD_EMPTY)(Name, IsAbstract, PolymorphicBaseName)                        \
                                                                                                                                                                                                    \
    protected:                                                                                                                                                                                      \
        SerializationPOD(BoostCommon::Serialization::Details::DelayInitTag const &tag)                                                                                                              \
            BOOST_PP_IIF(HasBases, SERIALIZATION_Impl_PODImpl_DelayedCtor, BOOST_VMD_EMPTY)(Bases)                                                                                                  \
        { (tag); }                                                                                                                                                                                  \
                                                                                                                                                                                                    \
        void Init(SerializeData const &data) {                                                                                                                                                      \
            BOOST_PP_IIF(HasBases, SERIALIZATION_Impl_PODImpl_Init, BOOST_VMD_EMPTY)(Bases)                                                                                                         \
            const_cast<SerializeData const *&>(_pSerializeData) = &data;                                                                                                                            \
        }                                                                                                                                                                                           \
                                                                                                                                                                                                    \
        void Init(DeserializeData &data) {                                                                                                                                                          \
            BOOST_PP_IIF(HasBases, SERIALIZATION_Impl_PODImpl_Init, BOOST_VMD_EMPTY)(Bases)                                                                                                         \
            const_cast<DeserializeData *&>(_pDeserializeData) = &data;                                                                                                                              \
        }                                                                                                                                                                                           \
                                                                                                                                                                                                    \
    private:                                                                                                                                                                                        \
        friend class boost::serialization::access;                                                                                                                                                  \
                                                                                                                                                                                                    \
        std::unique_ptr<SerializeData> const            _serialize_data_storage;                                                                                                                    \
        std::unique_ptr<DeserializeData> const          _deserialize_data_storage;                                                                                                                  \
                                                                                                                                                                                                    \
        SerializeData const * const                     _pSerializeData = nullptr;                                                                                                                  \
        DeserializeData * const                         _pDeserializeData = nullptr;                                                                                                                \
                                                                                                                                                                                                    \
        DeserializeData MoveDeserializeData(void) {                                                                                                                                                 \
            if(_deserialize_data_storage == false)                                                                                                                                                  \
                throw std::logic_error("DeserializeData has already been moved or never existed");                                                                                                  \
                                                                                                                                                                                                    \
            DeserializeData                 result(std::move(*_deserialize_data_storage));                                                                                                          \
                                                                                                                                                                                                    \
            make_mutable(_deserialize_data_storage).reset();                                                                                                                                        \
            return result;                                                                                                                                                                          \
        }                                                                                                                                                                                           \
                                                                                                                                                                                                    \
        BOOST_SERIALIZATION_SPLIT_MEMBER();                                                                                                                                                         \
                                                                                                                                                                                                    \
        template <typename ArchiveT>                                                                                                                                                                \
        void save(ArchiveT &ar, unsigned int const) const {                                                                                                                                         \
            if(_pSerializeData == nullptr)                                                                                                                                                          \
                throw std::logic_error("SerializeData is not available");                                                                                                                           \
                                                                                                                                                                                                    \
            BOOST_PP_IIF(HasBases, SERIALIZATION_Impl_PODImpl_Serialize, BOOST_VMD_EMPTY)(Bases)                                                                                                    \
            _pSerializeData->local.Execute(ar);                                                                                                                                                     \
        }                                                                                                                                                                                           \
                                                                                                                                                                                                    \
        template <typename ArchiveT>                                                                                                                                                                \
        void load(ArchiveT &ar, unsigned int const) {                                                                                                                                               \
            if(_pDeserializeData == nullptr)                                                                                                                                                        \
                throw std::logic_error("DeserializeData is not available");                                                                                                                         \
                                                                                                                                                                                                    \
            BOOST_PP_IIF(HasBases, SERIALIZATION_Impl_PODImpl_Serialize, BOOST_VMD_EMPTY)(Bases)                                                                                                    \
            _pDeserializeData->local.Execute(ar);                                                                                                                                                   \
        }                                                                                                                                                                                           \
    };  

#define SERIALIZATION_Impl_PODImpl_BaseClasses(Bases)                                   , BOOST_PP_TUPLE_FOR_EACH_ENUM(SERIALIZATION_Impl_PODImpl_BaseClasses_Macro, _, Bases)
#define SERIALIZATION_Impl_PODImpl_BaseClasses_Macro(r, _, Base)                        public Base::SerializationPOD

#define SERIALIZATION_Impl_PODImpl_SerializePolymorphicMembers(PolymorphicBaseName)     using PolymorphicBaseClass = PolymorphicBaseName; PolymorphicBaseClass const * const pPolymorphicBaseClass = nullptr;

#define SERIALIZATION_Impl_PODImpl_Serialize_Bases(Bases)                               BOOST_PP_TUPLE_FOR_EACH_I(SERIALIZATION_Impl_PODImpl_Serialize_Bases_Macro, _, Bases)
#define SERIALIZATION_Impl_PODImpl_Serialize_Bases_Macro(r, _, Index, Base)             typename Base::SerializationPOD::SerializeData const BOOST_PP_CAT(base, Index);

#define SERIALIZATION_Impl_PODImpl_Serialize_Ctor(Bases)                                BOOST_PP_TUPLE_FOR_EACH_ENUM_I(SERIALIZATION_Impl_PODImpl_Serialize_Ctor_Macro, _, Bases) ,
#define SERIALIZATION_Impl_PODImpl_Serialize_Ctor_Macro(r, _, Index, Base)              BOOST_PP_CAT(base, Index)(obj)

#define SERIALIZATION_Impl_PODImpl_Deserialize_Bases(Bases)                             BOOST_PP_TUPLE_FOR_EACH_I(SERIALIZATION_Impl_PODImpl_Deserialize_Bases_Macro, _, Bases)
#define SERIALIZATION_Impl_PODImpl_Deserialize_Bases_Macro(r, _, Index, Base)           typename Base::SerializationPOD::DeserializeData BOOST_PP_CAT(base, Index);

#define SERIALIZATION_Impl_PODImpl_Deserialize_Ctor(Bases)                              BOOST_PP_TUPLE_FOR_EACH_ENUM_I(SERIALIZATION_Impl_PODImpl_Deserialize_Ctor_Macro, _, Bases) ,
#define SERIALIZATION_Impl_PODImpl_Deserialize_Ctor_Macro(r, _, Index, Base)            BOOST_PP_CAT(base, Index)(std::move(other. BOOST_PP_CAT(base, Index)))

#define SERIALIZATION_Impl_PODImpl_DelayInitCtor(Bases)                                 BOOST_PP_TUPLE_FOR_EACH_ENUM(SERIALIZATION_Impl_PODImpl_DelayInitCtor_Macro, _, Bases) ,
#define SERIALIZATION_Impl_PODImpl_DelayInitCtor_Macro(r, _, Base)                      Base::SerializationPOD(BoostCommon::Serialization::Details::DelayInitTag())

#define SERIALIZATION_Impl_PODImpl_Construct(Name)                                      Name Construct(void) { return MoveDeserializeData(); }

#define SERIALIZATION_Impl_PODImpl_ConstructPtr(Name, IsAbstract, PolymorphicBaseName)                                                                              \
    virtual std::unique_ptr<PolymorphicBaseName> ConstructPtr(void)                                                                                                 \
        BOOST_PP_IIF(IsAbstract, SERIALIZATION_Impl_PODImpl_ConstructPtr_Abstract, SERIALIZATION_Impl_PODImpl_ConstructPtr_Concrete)(Name, PolymorphicBaseName)     \
                                                                                                                                                                    \
    void SetOriginalBaseClass(PolymorphicBaseName const &obj) const {                                                                                               \
        if(_pSerializeData == nullptr)                                                                                                                              \
            throw std::logic_error("SetOriginalBaseClass can only be invoked on serializing PODs");                                                                 \
                                                                                                                                                                    \
        if(_pSerializeData->pPolymorphicBaseClass != nullptr)                                                                                                       \
            throw std::logic_error("The original base class has already been set");                                                                                 \
                                                                                                                                                                    \
        const_cast<PolymorphicBaseName const *&>(_pSerializeData->pPolymorphicBaseClass) = &obj;                                                                    \
    }                                                                                                                                                               \
                                                                                                                                                                    \
    PolymorphicBaseName const & GetOriginalBaseClass(void) const {                                                                                                  \
        if(_pSerializeData == nullptr)                                                                                                                              \
            throw std::logic_error("SerializeData is not available");                                                                                               \
                                                                                                                                                                    \
        if(_pSerializeData->pPolymorphicBaseClass == nullptr)                                                                                                       \
            throw std::logic_error("The original base class has not been set");                                                                                     \
                                                                                                                                                                    \
        return *_pSerializeData->pPolymorphicBaseClass;                                                                                                             \
    }   

#define SERIALIZATION_Impl_PODImpl_ConstructPtr_Abstract(Name, PolymorphicBaseName)     = 0;
#define SERIALIZATION_Impl_PODImpl_ConstructPtr_Concrete(Name, PolymorphicBaseName)     { return std::make_unique<Name>(MoveDeserializeData()); }

#define SERIALIZATION_Impl_PODImpl_DelayedCtor(Bases)                       : BOOST_PP_TUPLE_FOR_EACH_ENUM(SERIALIZATION_Impl_PODImpl_DelayedCtor_Macro, _, Bases)
#define SERIALIZATION_Impl_PODImpl_DelayedCtor_Macro(r, _, Base)            Base::SerializationPOD(tag)

#define SERIALIZATION_Impl_PODImpl_Init(Bases)                              BOOST_PP_TUPLE_FOR_EACH_I(SERIALIZATION_Impl_PODImpl_Init_Macro, _, Bases)
#define SERIALIZATION_Impl_PODImpl_Init_Macro(r, _, Index, Base)            Base::SerializationPOD::Init(data. BOOST_PP_CAT(base, Index));

#define SERIALIZATION_Impl_PODImpl_Serialize(Bases)                         BOOST_PP_TUPLE_FOR_EACH_I(SERIALIZATION_Impl_PODImpl_Serialize_Macro, _, Bases)
#define SERIALIZATION_Impl_PODImpl_Serialize_Macro(r, _, Index, Base)       ar & boost::serialization::make_nvp(BoostCommon::Serialization::Details::ScrubSerializationName(BOOST_PP_STRINGIZE(Base)), boost::serialization::base_object<Base::SerializationPOD>(*this));

// ----------------------------------------------------------------------
#define SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes(Name, HasMembers, Members, HasDeserializeDataCustomCtor)                                                                                           \
    struct SerializeLocalData {                                                                                                                                                                             \
        BOOST_PP_IIF(HasMembers, SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_SerializeMembers, BOOST_VMD_EMPTY)(Name, Members)                                                                         \
                                                                                                                                                                                                            \
        SerializeLocalData(Name const &obj)                                                                                                                                                                 \
            BOOST_PP_IIF(HasMembers, SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_SerializeCtor, BOOST_VMD_EMPTY)(Members)                                                                              \
        { (obj); }                                                                                                                                                                                          \
                                                                                                                                                                                                            \
        SerializeLocalData(SerializeLocalData &&) = delete;                                                                                                                                                 \
        SerializeLocalData & operator =(SerializeLocalData &&) = delete;                                                                                                                                    \
                                                                                                                                                                                                            \
        SerializeLocalData(SerializeLocalData const &) = delete;                                                                                                                                            \
        SerializeLocalData & operator =(SerializeLocalData const &) = delete;                                                                                                                               \
                                                                                                                                                                                                            \
        template <typename ArchiveT>                                                                                                                                                                        \
        void Execute(ArchiveT &ar) const {                                                                                                                                                                  \
            (ar);                                                                                                                                                                                           \
            BOOST_PP_IIF(HasMembers, SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_SerializeExecute, BOOST_VMD_EMPTY)(Members)                                                                           \
        }                                                                                                                                                                                                   \
    };                                                                                                                                                                                                      \
                                                                                                                                                                                                            \
    struct DeserializeLocalData {                                                                                                                                                                           \
        BOOST_PP_IIF(HasMembers, SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_DeserializeMembers, BOOST_VMD_EMPTY)(Name, Members)                                                                       \
                                                                                                                                                                                                            \
        DeserializeLocalData(void)                                                                                                                                                                          \
            BOOST_PP_IIF(HasDeserializeDataCustomCtor, SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_DeserializeCustomCtor, SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_DeserializeDefaultCtor)()   \
                                                                                                                                                                                                            \
        DeserializeLocalData(DeserializeLocalData && other)                                                                                                                                                 \
            BOOST_PP_IIF(HasMembers, SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_DeserializeCtor, BOOST_VMD_EMPTY)(Members)                                                                            \
        { (other); }                                                                                                                                                                                        \
                                                                                                                                                                                                            \
        DeserializeLocalData & operator =(DeserializeLocalData &&) = delete;                                                                                                                                \
                                                                                                                                                                                                            \
        DeserializeLocalData(DeserializeLocalData const &) = delete;                                                                                                                                        \
        DeserializeLocalData & operator =(DeserializeLocalData const &) = delete;                                                                                                                           \
                                                                                                                                                                                                            \
        template <typename ArchiveT>                                                                                                                                                                        \
        void Execute(ArchiveT &ar) {                                                                                                                                                                        \
            (ar);                                                                                                                                                                                           \
            BOOST_PP_IIF(HasMembers, SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_DeserializeExecute, BOOST_VMD_EMPTY)(Members)                                                                         \
        }                                                                                                                                                                                                   \
    };

#define SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_SerializeMembers(Name, Members)                BOOST_PP_TUPLE_FOR_EACH(SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_SerializeMembers_Macro, Name, Members)
#define SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_SerializeMembers_Macro(r, Name, Member)        BoostCommon::Serialization::Details::SerializeDataType<decltype(Name::Member)> const Member;

#define SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_SerializeCtor(Members)                         : BOOST_PP_TUPLE_FOR_EACH_ENUM(SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_SerializeCtor_Macro, _, Members)
#define SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_SerializeCtor_Macro(r, _, Member)              Member(obj.Member)

#define SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_SerializeExecute(Members)                      BOOST_PP_TUPLE_FOR_EACH(SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_SerializeExecute_Macro, _, Members)
#define SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_SerializeExecute_Macro(r, _, Member)           ar << boost::serialization::make_nvp(BOOST_PP_STRINGIZE(Member), Member);

#define SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_DeserializeMembers(Name, Members)              BOOST_PP_TUPLE_FOR_EACH(SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_DeserializeMembers_Macro, Name, Members)
#define SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_DeserializeMembers_Macro(r, Name, Member)      BoostCommon::Serialization::Details::DeserializeDataType<decltype(Name::Member)> Member;

#define SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_DeserializeCustomCtor()                        ;
#define SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_DeserializeDefaultCtor()                       {}

#define SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_DeserializeCtor(Members)                       : BOOST_PP_TUPLE_FOR_EACH_ENUM(SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_DeserializeCtor_Macro, _, Members)
#define SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_DeserializeCtor_Macro(r, _, Member)            Member(std::move(other.Member))

#define SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_DeserializeExecute(Members)                    BOOST_PP_TUPLE_FOR_EACH(SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_DeserializeExecute_Macro, _, Members)
#define SERIALIZATION_Impl_PODImpl_DefaultLocalDataTypes_DeserializeExecute_Macro(r, _, Member)         ar >> boost::serialization::make_nvp(BOOST_PP_STRINGIZE(Member), Member);

#define SERIALIZATION_Impl_Func_Name()                                                                  __Ensure_correct_include__See_SERIALIZATION_for_more_info

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
#define SERIALIZATION_POLYMORPHIC_DECLARE_Impl(FullyQualifiedObjectName)                                \
    inline void FullyQualifiedObjectName::SERIALIZATION_POLYMORPHIC_DECLARE_Impl_Func_Name() (void) {}  \
    BOOST_CLASS_EXPORT_KEY(FullyQualifiedObjectName::SerializationPOD);                                 \
    BOOST_CLASS_EXPORT_KEY(FullyQualifiedObjectName);

#define SERIALIZATION_POLYMORPHIC_DEFINE_Impl(FullyQualifiedObjectName)                                 \
    void FullyQualifiedObjectName::SERIALIZATION_POLYMORPHIC_DEFINE_Impl_Func_Name() (void) {}          \
    BOOST_CLASS_EXPORT_IMPLEMENT(FullyQualifiedObjectName::SerializationPOD);                           \
    BOOST_CLASS_EXPORT_IMPLEMENT(FullyQualifiedObjectName);

#define SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE_Impl(FullyQualifiedObjectName)                     \
    SERIALIZATION_POLYMORPHIC_DECLARE(FullyQualifiedObjectName)                                         \
    SERIALIZATION_POLYMORPHIC_DEFINE(FullyQualifiedObjectName)

#define SERIALIZATION_POLYMORPHIC_DECLARE_Impl_Func_Name()                                              __Ensure_correct_include__See_SERIALIZATION_POLYMORPHIC_DECLARE_for_more_info
#define SERIALIZATION_POLYMORPHIC_DEFINE_Impl_Func_Name()                                               __Ensure_correct_linkage__See_SERIALIZATION_POLYMORPHIC_DEFINE_for_more_info

// clang-format on

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
namespace Details {

#if (defined NDEBUG || defined DEBUG)

/////////////////////////////////////////////////////////////////////////
///  \function      SERIALIZATION_Impl_Func_Name()
///  \brief         Declares a function implemented in Serialization.suffix.h.
///                 Linker errors when this function isn't defined indicate that
///                 Serialization.suffix.h hasn't been included.
///
void SERIALIZATION_Impl_Func_Name()(void);

#endif

/////////////////////////////////////////////////////////////////////////
///  \struct        DelayInitTag
///  \brief         Tag used as a constructor argument to indicate that
///                 the object will be initialized by a derived object
///                 (which is different from default construction).
///
struct DelayInitTag {};

// has_SerializationPOD is used in Serialization.suffix.h.
CREATE_HAS_TYPE_CHECKER(SerializationPOD);

// ----------------------------------------------------------------------
namespace {

enum class CreateMemberType {
    SerializationPOD_SmartPointer,
    SerializationPOD_Standard,
    Standard
};

template <typename T, typename U>
T CreateMemberImpl(U && data, std::integral_constant<CreateMemberType, CreateMemberType::SerializationPOD_SmartPointer>) {
    return data.ConstructPtr();
}

template <typename T, typename U>
T CreateMemberImpl(U && data, std::integral_constant<CreateMemberType, CreateMemberType::SerializationPOD_Standard>) {
    return data.Construct();
}

template <typename T, typename U>
T && CreateMemberImpl(U && data, std::integral_constant<CreateMemberType, CreateMemberType::Standard>) {
    return std::forward<T>(data);
}

// ----------------------------------------------------------------------
template <typename T, bool>
struct SerializeDataTypeImpl_IsPointer {
    using type                              = CommonHelpers::TypeTraits::MakeTargetImmutable<T>;
};

template <typename T>
struct SerializeDataTypeImpl_IsPointer<T, false> {
    using type                              = std::add_lvalue_reference_t<CommonHelpers::TypeTraits::MakeTargetImmutable<T>>;
};

template <typename T, bool>
struct SerializeDataTypeImpl_HasSerializationPOD {
    using type                              = typename T::SerializationPOD;
};

template <typename T>
struct SerializeDataTypeImpl_HasSerializationPOD<T, false> : public SerializeDataTypeImpl_IsPointer<T, std::is_pointer_v<T>> {
};

template <typename T>
struct SerializeDataTypeImpl : public SerializeDataTypeImpl_HasSerializationPOD<T, has_SerializationPOD<T>> {
};

// ----------------------------------------------------------------------
template <typename T, bool>
struct DeserializeDataTypeImpl_HasSerializationPOD {
    using type                              = typename T::SerializationPOD;
};

template <typename T>
struct DeserializeDataTypeImpl_HasSerializationPOD<T, false> {
    using type                              = std::remove_const_t<std::remove_reference_t<T>>;
};

template <typename T>
struct DeserializeDataTypeImpl : public DeserializeDataTypeImpl_HasSerializationPOD<T, has_SerializationPOD<T>> {
};

}  // anonymous namespace

/////////////////////////////////////////////////////////////////////////
///  \function      CreateMember
///  \brief         Creates a data member during the deserialization process,
///                 taking its specific serialization semantics into account
///                 based on its type.
///
template <typename T, typename U>
std::remove_const_t<T> CreateMember(U && data) {

#if (defined NDEBUG || defined DEBUG)
    // Errors here indicate that "Serialization.suffix.h" has not been
    // included; see the documentation for SERIALIZATION for more 
    // information.
    SERIALIZATION_Impl_Func_Name()();
#endif

    return CreateMemberImpl<std::remove_const_t<T>>(
        std::forward<U>(data),
        std::conditional_t<
            has_SerializationPOD<T>,
            std::conditional_t<
                CommonHelpers::TypeTraits::IsSmartPointer<T>,
                std::integral_constant<CreateMemberType, CreateMemberType::SerializationPOD_SmartPointer>,
                std::integral_constant<CreateMemberType, CreateMemberType::SerializationPOD_Standard>
            >,
            std::integral_constant<CreateMemberType, CreateMemberType::Standard>
        >()
    );
}

/////////////////////////////////////////////////////////////////////////
///  \typedef       SerializeDataType
///  \brief         Determines the best type to use in the process of
///                 serializing T.
///
template <typename T>
using SerializeDataType                     = typename SerializeDataTypeImpl<std::remove_const_t<T>>::type;

/////////////////////////////////////////////////////////////////////////
///  \typedef       DeserializeDataType
///  \brief         Determines the best type to use in the process of 
///                 deserializing T.
///
template <typename T>
using DeserializeDataType                   = typename DeserializeDataTypeImpl<std::remove_const_t<T>>::type;

/////////////////////////////////////////////////////////////////////////
///  \function      ScrubSerializationName
///  \brief         The name used when serializing name-value pairs must be
///                 alphanumeric, but the names passed as auto-generated tags
///                 may have punctuation chars (for example when the name is
///                 part of a namespace). This method will return the first
///                 alphanumeric char after the last non-alphanumeric char.
///
inline char const * ScrubSerializationName(char const *name) {
    char const *                            pLastInvalid(nullptr);
    char const *                            ptr(name);

    while(ptr && *ptr != 0)
    {
        if(
            (
                (*ptr >= 'a' && *ptr <= 'z')
                || (*ptr >= 'A' && *ptr <= 'Z')
                || (*ptr >= '0' && *ptr <= '9')
                || *ptr == '.'
                || *ptr == '-'
                || *ptr == '_'
            ) == false
        ) {
            pLastInvalid = ptr;
        }

        ++ptr;
    }

    if(pLastInvalid == nullptr)
        return name;

    ++pLastInvalid;

    if(*pLastInvalid == 0)
        return "GenericTag";

    return pLastInvalid;
}

}  // namespace Details
}  // namespace Serialization
}  // namespace BoostCommon
