/////////////////////////////////////////////////////////////////////////
///
///  \file          TestHelpers.h
///  \brief         Contains the TestHelpers namespace
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-14 21:42:53
///
///  \note
///
///  \bug
///
/////////////////////////////////////////////////////////////////////////
///
///  \attention
///  Copyright David Brownell 2020-22
///  Distributed under the Boost Software License, Version 1.0. See
///  accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt.
///
/////////////////////////////////////////////////////////////////////////
#pragma once

#include "Serialization.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include "Serialization.suffix.h"

#include <functional>
#include <optional>
#include <sstream>

namespace BoostHelpers {
namespace TestHelpers {

/////////////////////////////////////////////////////////////////////////
///  \fn            SerializeTest
///  \brief         Test that verifies serialization for an object. Returns
///                 0 if all tests pass, or an index value corresponding to
///                 any failures.
///
template <typename T>
unsigned char SerializeTest(T const &obj, std::optional<std::function<void (std::string const &)>> const &onSerializedFunc=std::nullopt);

/////////////////////////////////////////////////////////////////////////
///  \fn            SerializePtrTest
///  \brief         Test that verifies serialization via pointer for an object.
///                 Returns  0 if all tests pass, or an index value
///                 corresponding to any failures.
///
template <typename T, typename DerivedT=typename T::element_type>
unsigned char SerializePtrTest(T const &obj, std::optional<std::function<void (std::string const &)>> const &onSerializedFunc=std::nullopt);

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// |
// |  Implementation
// |
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
namespace Details {

template <typename OArchiveT, typename IArchiveT, typename T>
bool SerializeTestImpl(T const &obj, std::optional<std::function<void (std::string const &)>> const &onSerializedFunc) {
    std::ostringstream                      out;

    obj.template Serialize<OArchiveT>(out);
    out.flush();

    std::string                             result(out.str());

    if(onSerializedFunc) {
        assert(*onSerializedFunc);
        (*onSerializedFunc)(result);
    }

    std::istringstream                      in(result);
    T const                                 other(T::template Deserialize<IArchiveT>(in));

    return other == obj;
}

template <typename OArchiveT, typename IArchiveT, typename DerivedT, typename T>
bool SerializePtrTestImpl(T const &obj, std::optional<std::function<void (std::string const &)>> const &onSerializedFunc) {
    std::ostringstream                      out;

    obj->template SerializePtr<OArchiveT>(out);
    out.flush();

    std::string                             result(out.str());

    if(onSerializedFunc) {
        assert(*onSerializedFunc);
        (*onSerializedFunc)(result);
    }

    std::istringstream                      in(result);
    auto const                              other(DerivedT::template DeserializePtr<IArchiveT>(in));

    return *other == *static_cast<DerivedT const *>(obj.get());
}

} // namespace Details

template <typename T>
unsigned char SerializeTest(T const &obj, std::optional<std::function<void (std::string const &)>> const &onSerializedFunc/*=std::nullopt*/) {
    if(Details::SerializeTestImpl<boost::archive::text_oarchive, boost::archive::text_iarchive>(obj, onSerializedFunc) == false)
        return 1;
    if(Details::SerializeTestImpl<boost::archive::xml_oarchive, boost::archive::xml_iarchive>(obj, onSerializedFunc) == false)
        return 2;

    return 0;
}

template <typename T, typename DerivedT/*=typename T::element_type*/>
unsigned char SerializePtrTest(T const &obj, std::optional<std::function<void (std::string const &)>> const &onSerializedFunc/*=std::nullopt*/) {
    if(Details::SerializePtrTestImpl<boost::archive::text_oarchive, boost::archive::text_iarchive, DerivedT>(obj, onSerializedFunc) == false)
        return 1;
    if(Details::SerializePtrTestImpl<boost::archive::xml_oarchive, boost::archive::xml_iarchive, DerivedT>(obj, onSerializedFunc) == false)
        return 2;

    return 0;
}

} // namespace TestHelpers
} // namespace BoostHelpers
