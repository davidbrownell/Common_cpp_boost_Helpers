/////////////////////////////////////////////////////////////////////////
///
///  \file          SharedObject.h
///  \brief         Contains the SharedObject object
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-10 11:06:40
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
#pragma once

#include <CommonHelpers/SharedObject.h>
#include <BoostHelpers/Serialization.h>

namespace BoostHelpers {

/////////////////////////////////////////////////////////////////////////
///  \class         SharedObject
///  \brief         This object is essentially the same as CommonHelpers::SharedObject,
///                 with the only difference being the support of serialization
///                 (which is enabled through SERIALIZATION which is defined in
///                 this library).
///
///                 Base object that simplifies usage of `std::enable_shared_from_this`
///                 by enforcing conventions that ensure correct usage.
///
///                 Example:
///                     struct Base : public SharedObject {
///                         int             i;
///
///                         // Explicit Create method or use the CREATE macro
///                         static std::shared_ptr<Base> Create(int i) { return CreateImpl<Base>(i); }
///
///                         template <typename PrivateConstructorTagT>
///                         Base(PrivateConstructorTagT tag, int iParam) : SharedObject(tag), i(iParam) {}
///                     };
///
///                     struct Derived : public Base {
///                         bool            b;
///
///                         // Explicit Create method or use the CREATE macro
///                         CREATE(Derived);
///
///                         template <typename PrivateConstructorTagT>
///                         Derived(PrivateConstructorTagT tag, int i, bool bParam) : Base(tag, i), b(bParam) {}
///                     };
///
///                     std::shared_ptr<Base>           pBase(Base::Create(10));
///                     std::shared_ptr<Derived>        pDerived(Derived::Create(20, true));
///
///                     std::shared_ptr<Base>           pNewBase(pBase->SharedFromThis());
///                     std::shared_ptr<Derived>        pNewDerived(pDerived->CreateSharedPtr<Derived>());
///                     std::shared_ptr<Base>           pNewBaseFromDerived(pDerived->CreateSharedPtr<Base>());
///
class SharedObject : public std::enable_shared_from_this<SharedObject> {
private:
    // ----------------------------------------------------------------------
    // |  Private Types (used in public declarations)
    struct PrivateConstructorTag {};

public:
    // ----------------------------------------------------------------------
    // |  Public Methods
    SharedObject(PrivateConstructorTag);

    NON_COPYABLE(SharedObject);
    MOVE(SharedObject);
    COMPARE(SharedObject);
    SERIALIZATION(SharedObject, FLAGS(SERIALIZATION_DATA_ONLY));

    template <typename T>
    std::shared_ptr<T> CreateSharedPtr(void) const;

protected:
    // ----------------------------------------------------------------------
    // |  Protected Methods
    template <typename T, typename... ArgTs>
    static std::shared_ptr<T> CreateImpl(ArgTs &&... args);

    virtual ~SharedObject(void) = default;
};

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// |
// |  Implementation
// |
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
inline SharedObject::SharedObject(PrivateConstructorTag)
{}

template <typename T>
std::shared_ptr<T> SharedObject::CreateSharedPtr(void) const {
    assert(dynamic_cast<T const *>(this) != nullptr);
    return std::static_pointer_cast<T>(const_cast<SharedObject &>(*this).shared_from_this());
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
template <typename T, typename... ArgTs>
// static
std::shared_ptr<T> SharedObject::CreateImpl(ArgTs &&...args) {
    return std::make_shared<T>(PrivateConstructorTag(), std::forward<ArgTs>(args)...);
}

} // namespace BoostHelpers
