#pragma once

#include "cpp-rt.hpp"

#define MAKE_ALL_PTRS(a) typedef a* Ptr; \
    typedef ::BmCpp::ObjectPtr< a >	OPtr; \
    typedef ::BmCpp::ConstObjectPtr< a > ConstOPtr

namespace BmCpp {

struct Object : public BaseAllocation {
    inline virtual      ~Object()               {}

    inline Object()     { count_.store(0); }
    inline void         grab() const            { count_.fetch_add(1); }
    inline void         release() const         { if( count_.fetch_sub(1) == 1 ) { delete const_cast<Object*>(this); } }
    inline size_t		getRefCount() const     { return count_.load(); }

protected:
    inline Object(const Object& /*other*/) { count_.store(0); }

private:
    mutable     std::atomic<size_t> count_;
};

//
//  Copyright (c) 2001, 2002 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/smart_ptr/ObjectPtr.html for documentation.
//
//
//  ObjectPtr
//
//  A smart pointer that uses intrusive reference counting.
//
//  Relies on unqualified calls to
//
//          (p != 0)
//
//  The object is responsible for destroying itself.
//

template<class T>
class ObjectPtr
{
private:

    typedef ObjectPtr ThisType;

public:

    typedef T ElementType;

    ObjectPtr(): px( 0 )	{}

    ObjectPtr( T * p, bool add_ref = true ): px( p ) {
        if( px != 0 && add_ref ) px->grab();
    }

    template<class U>
    ObjectPtr( ObjectPtr<U> const & rhs )
        : px( rhs.get() ) {
        if( px != 0 ) px->grab();
    }

    ObjectPtr(ObjectPtr const & rhs): px( rhs.px ) {
        if( px != 0 ) px->grab();
    }

    ~ObjectPtr() {
        if( px != 0 ) px->release();
    }

    template<class U> ObjectPtr & operator=(ObjectPtr<U> const & rhs) {
        ThisType(rhs).swap(*this);
        return *this;
    }

    // Move support

    //	ObjectPtr(ObjectPtr && rhs): px( rhs.px )
    //	{
    //		rhs.px = 0;
    //	}

    //	ObjectPtr & operator=(ObjectPtr && rhs)
    //	{
    //		ThisType( static_cast< ObjectPtr && >( rhs ) ).swap(*this);
    //		return *this;
    //	}

    ObjectPtr & operator=(ObjectPtr const & rhs) {
        ThisType(rhs).swap(*this);
        return *this;
    }

    ObjectPtr & operator=(T * rhs) {
        ThisType(rhs).swap(*this);
        return *this;
    }

    void reset() {
        ThisType().swap( *this );
    }

    void reset( T * rhs ) {
        ThisType( rhs ).swap( *this );
    }

    T * get() const {
        return px;
    }

    T & operator*() const {
        //		if( px == nullptr )
        //			throw "pointer is null";
        return *px;
    }

    T * operator->() const {
        //		if( px == nullptr )
        //			throw "pointer is null";
        return px;
    }

    typedef T * ThisType::*unspecified_bool_type;

    operator unspecified_bool_type() const { // never throws
        return px == 0? 0: &ThisType::px;
    }

    // operator! is redundant, but some compilers need it
    bool operator! () const {// never throws
        return px == 0;
    }

    void swap(ObjectPtr & rhs) {
        T * tmp = px;
        px = rhs.px;
        rhs.px = tmp;
    }

    // will create a pointer ambiguous
    //	operator T*()
    //	{
    //		return px;
    //	}

private:

    T*		px;

    template<typename U>
    friend struct ConstObjectPtr;
};

template<class T, class U>
inline bool operator==(ObjectPtr<T> const & a, ObjectPtr<U> const & b) {
    return a.get() == b.get();
}

template<class T, class U>
inline bool operator!=(ObjectPtr<T> const & a, ObjectPtr<U> const & b) {
    return a.get() != b.get();
}

template<class T, class U>
inline bool operator==(ObjectPtr<T> const & a, U * b) {
    return a.get() == b;
}

template<class T, class U>
inline bool operator!=(ObjectPtr<T> const & a, U * b) {
    return a.get() != b;
}

template<class T, class U>
inline bool operator==(T * a, ObjectPtr<U> const & b) {
    return a == b.get();
}

template<class T, class U>
inline bool operator!=(T * a, ObjectPtr<U> const & b) {
    return a != b.get();
}

#if __GNUC__ == 2 && __GNUC_MINOR__ <= 96

// Resolve the ambiguity between our op!= and the one in rel_ops

template<class T>
inline bool operator!=(ObjectPtr<T> const & a, ObjectPtr<T> const & b) {
    return a.get() != b.get();
}

#endif

template<class T>
inline bool operator<(ObjectPtr<T> const & a, ObjectPtr<T> const & b) {
    return (static_cast<size_t>(a.get()) < static_cast<size_t>(b.get()));
}

template<class T>
void swap(ObjectPtr<T> & lhs, ObjectPtr<T> & rhs) {
    lhs.swap(rhs);
}

// mem_fn support

template<class T>
T * get_pointer(ObjectPtr<T> const & p) {
    return p.get();
}

template<class T, class U>
ObjectPtr<T> static_pointer_cast(ObjectPtr<U> const & p) {
    return static_cast<T *>(p.get());
}

template<class T, class U>
ObjectPtr<T> const_pointer_cast(ObjectPtr<U> const & p) {
    return const_cast<T *>(p.get());
}

template<class T, class U>
ObjectPtr<T> dynamic_pointer_cast(ObjectPtr<U> const & p) {
    return dynamic_cast<T *>(p.get());
}


//
//  Copyright (c) 2001, 2002 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/smart_ptr/ConstObjectPtr.html for documentation.
//
//
//  ConstObjectPtr
//
//  A smart pointer that uses intrusive reference counting.
//
//  Relies on object
//
//  The object is responsible for destroying itself.
//

template<class T>
class ConstObjectPtr {
private:
    typedef ConstObjectPtr ThisType;

public:
    typedef T ElementType;

    ConstObjectPtr() : px(nullptr)	{}
    ConstObjectPtr(T* t): px( t )		{ if( px != 0 ) px->grab();	}
    ConstObjectPtr(const ConstObjectPtr& rhs): px( rhs.px )	{ if( px != 0 ) px->grab();	}
    ConstObjectPtr(ObjectPtr<T>& rhs)	: px(rhs.px)		{ if( px != 0 ) px->grab();	}

    ~ConstObjectPtr()			{ if( px != 0 ) px->release();	}

    ConstObjectPtr&			operator= (ConstObjectPtr const& rhs)	{
        ThisType(rhs).swap(*this);
        return *this;
    }

    ConstObjectPtr&			operator= (ObjectPtr<T>& rhs)	{
        ThisType(rhs).swap(*this);
        return *this;
    }

    //
    //	this one is ambigous because the compiler can create an "ConstObjectPtr" from casting the previous one (copy constructor)
    //
    // 	ConstObjectPtr&					operator= (const T& rhs)	{
    //							val*	nv(rhs);
    //							ThisType(nv).swap(*this);
    //							return *this;
    //						}

    const T*				get() const		{ return px == 0 ? 0 : px;	}


    const T&				operator*() const	{
        assert( px != 0 && "pointer is null");
        return *px;
    }

    const T*				operator->() const	{
        assert( px != 0 && "pointer is null");
        return px;
    }
    // operator! is redundant, but some compilers need it
    bool					operator! () const	{// never throws
        return px == 0;
    }



    // will create a pointer ambiguous
    //	operator T*()
    //	{
    //		return px;
    //	}

private:

    void					swap(ConstObjectPtr & rhs) {
        T* tmp = px;
        px = rhs.px;
        rhs.px = tmp;
    }

    T*					px;
};

template<class T>
inline bool operator==(ConstObjectPtr<T> const& a, ConstObjectPtr<T> const& b)	{	return a.get() == b.get();	}

template<class T>
inline bool operator!=(ConstObjectPtr<T> const& a, ConstObjectPtr<T> const& b)	{	return a.get() != b.get();	}

template<class T>
inline bool operator==(ConstObjectPtr<T> const& a, const T* b)			{	return a.get() == b;		}

template<class T>
inline bool operator!=(ConstObjectPtr<T> const& a, const T* b)			{	return a.get() != b;		}

template<class T>
inline bool operator==(const T* a, ConstObjectPtr<T> const& b)			{	return a == b.get();		}

template<class T>
inline bool operator!=(const T* a, ConstObjectPtr<T> const& b)			{	return a != b.get();		}


template<class T> void swap(ConstObjectPtr<T> & lhs, ConstObjectPtr<T> & rhs)	{	lhs.swap(rhs);			}

// mem_fn support

template<class T> const T * getPointer(ConstObjectPtr<T> const & p)			{	return p.get();			}

template<class T, class U> ConstObjectPtr<T> ConstObjectPtrStaticCast(ConstObjectPtr<U> const & p)	{	return static_cast<T *>(p.get());	}
template<class T, class U> ConstObjectPtr<T> ConstObjectPtrConstCast(ConstObjectPtr<U> const & p)	{	return const_cast<T *>(p.get());	}
template<class T, class U> ConstObjectPtr<T> ConstObjectPtrDynamicCast(ConstObjectPtr<U> const & p)	{	return dynamic_cast<T *>(p.get());	}


template <typename T>
struct Less< ConstObjectPtr<T> > {
    bool operator()(const ConstObjectPtr<T>& a, const ConstObjectPtr<T>& b) const	{		return size_t(a.get()) < size_t(b.get());	}
};

template <typename T>
struct Less< ObjectPtr<T> > {
    bool operator()(const ObjectPtr<T>& a, const ObjectPtr<T>& b) const	{		return size_t(a.get()) < size_t(b.get());	}
};

}	// namespace BmCpp
