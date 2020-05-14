/////////////////////////////////////////////////////////////////////////
///
///  \file          Serialization.suffix.h
///  \brief         Contains functionality that should be defined AFTER
///                 other serialization archives and customization for common
///                 types (serialization functionality for std::vector,
///                 std::shared_ptr, etc.) have been included.
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2019-03-30 19:00:38
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
#pragma once

#include "Serialization.h"

#include <boost/serialization/serialization.hpp>

namespace BoostHelpers {
namespace Serialization {
namespace Details {

/////////////////////////////////////////////////////////////////////////
///  \namespace     PODBasedSerialization
///  \brief         Contains types and methods useful when detecting and
///                 working with objects that support POD-based serialization
///                 (where "POD-based" simply means an object that is
///                 serialized via a contained type named "SerializationPOD").
///
namespace PODBasedSerialization {

namespace Details {

template <typename T>
static std::true_type IsSerializationPODImpl(typename T::SerializationPODTag const *);

template <typename T>
static std::false_type IsSerializationPODImpl(...);

template <typename T>
static std::true_type HasPolymorphicSerializationMethodsImpl(typename T::PolymorphicSerializationPODUniquePtr const *);

template <typename T>
static std::false_type HasPolymorphicSerializationMethodsImpl(...);

} // namespace Details

template <typename T>
constexpr bool const IsSerializationPOD = std::is_same_v<std::true_type, decltype(Details::IsSerializationPODImpl<T>(nullptr))>;

template <typename T>
constexpr bool const HasSerializationPOD = has_SerializationPOD<T> && IsSerializationPOD<T> == false;

template <typename T>
constexpr bool const HasStandardSerializationMethods = HasSerializationPOD<T>;

template <typename T>
constexpr bool const HasPolymorphicSerializationMethods = std::is_same_v<std::true_type, decltype(Details::HasPolymorphicSerializationMethodsImpl<T>(nullptr))>;

struct SerializationTag_PODBased {};  ///< Object is POD-based and should be serialized via standard methods
struct SerializationTag_Standard {};  ///< Object is not POD-based.

template <typename T>
using GetSerializationTag = std::conditional_t<
    HasSerializationPOD<T>,
    SerializationTag_PODBased,
    SerializationTag_Standard
>;

}  // namespace PODBasedSerialization

#if (defined NDEBUG || defined DEBUG)
inline void __Ensure_correct_include__See_SERIALIZATION_for_more_info(void) {
    // This method is invoked by functionality in Serialization.h. If this file isn't included,
    // the linker will produce errors that will hopefully help end-users track down the problem.
}
#endif

}  // namespace Details
}  // namespace Serialization
}  // namespace BoostHelpers

namespace boost {
namespace serialization {

namespace Details {

template <typename ArchiveT, typename T>
void serialize_impl(
    ArchiveT &,
    T &,
    version_type,
    BoostHelpers::Serialization::Details::PODBasedSerialization::SerializationTag_PODBased
) {
    // Nothing to do here, as the serialization is handled by save_construct_data and load_construct_data
}

template <typename ArchiveT, typename T>
void serialize_impl(
    ArchiveT &ar,
    T &t,
    version_type version,
    BoostHelpers::Serialization::Details::PODBasedSerialization::SerializationTag_Standard
) {
    serialize(ar, t, static_cast<const unsigned int>(version));
}

// This overload registers derived types when saving via a base-class pointer. By registering
// the type, we ensure that the archive can instantiate the correct derived type during
// deserialization. Note that this must happen here (where the raw pointer is wrapped by
// a name-value pair), as the archive won't invoke the functionality when the value is
// extracted from the nvp.
template <typename ArchiveT, typename T, std::enable_if_t<std::is_same_v<boost::mpl::true_, typename ArchiveT::is_saving> && BoostHelpers::Serialization::Details::PODBasedSerialization::HasPolymorphicSerializationMethods<std::decay_t<T>>> * = nullptr>
void serialize_impl(
    ArchiveT &ar,
    boost::serialization::nvp<T *> &t,
    version_type version,
    BoostHelpers::Serialization::Details::PODBasedSerialization::SerializationTag_Standard
) {
    if(*t.second)
        (*t.second)->RegisterSerializationTypes();

    serialize(ar, t, static_cast<const unsigned int>(version));
}

template <typename ArchiveT, typename T>
void save_construct_data_impl(
    ArchiveT &ar,
    T const *t,
    version_type,
    BoostHelpers::Serialization::Details::PODBasedSerialization::SerializationTag_PODBased
) {
    typename T::SerializationPOD            pod(*t);

    ar << boost::serialization::make_nvp("data", pod);
}

template <typename ArchiveT, typename T>
void save_construct_data_impl(
    ArchiveT &ar,
    T const *t,
    version_type version,
    BoostHelpers::Serialization::Details::PODBasedSerialization::SerializationTag_Standard
) {
    save_construct_data(ar, t, static_cast<const unsigned int>(version));
}

template <typename ArchiveT, typename T>
void load_construct_data_impl(
    ArchiveT &ar,
    T *t,
    version_type,
    BoostHelpers::Serialization::Details::PODBasedSerialization::SerializationTag_PODBased
) {
    typename T::SerializationPOD            pod;

    ar >> boost::serialization::make_nvp("data", pod);
    ::new(t) T(pod.Construct());
}

template <typename ArchiveT, typename T>
void load_construct_data_impl(
    ArchiveT &ar,
    T *t,
    version_type version,
    BoostHelpers::Serialization::Details::PODBasedSerialization::SerializationTag_Standard
) {
    load_construct_data(ar, t, static_cast<const unsigned int>(version));
}

} // namespace Details

template <typename ArchiveT, typename T>
void serialize(ArchiveT &ar, T &t, version_type const &version) {
    Details::serialize_impl(
        ar,
        t,
        version,
        BoostHelpers::Serialization::Details::PODBasedSerialization::GetSerializationTag<T>()
    );
}

template <typename ArchiveT, typename T>
void save_construct_data(ArchiveT &ar, T const *t, version_type const &version) {
    Details::save_construct_data_impl(
        ar,
        t,
        version,
        BoostHelpers::Serialization::Details::PODBasedSerialization::GetSerializationTag<T>()
    );
}

template <typename ArchiveT, typename T>
void load_construct_data(ArchiveT &ar, T *t, version_type const &version) {
    Details::load_construct_data_impl(
        ar,
        t,
        version,
        BoostHelpers::Serialization::Details::PODBasedSerialization::GetSerializationTag<T>()
    );
}

}  // namespace serialization
}  // namespace boost
