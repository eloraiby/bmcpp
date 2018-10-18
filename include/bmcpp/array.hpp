#pragma once
#include "cpp-rt.hpp"

namespace BmCpp
{

template<typename T>
struct Array : public BaseAllocation {
    enum
    {
        MIN_VEC_RES_SIZE	= 4
    };

    Array() : count(0), reserved(0), data(nullptr) {
    }

    Array(size_t reserved) : count(0), reserved(reserved) {
        data	= static_cast<T*>(calloc(reserved, sizeof(T)));
    }

    Array(size_t n, const T* elems) : count(n), reserved(n), data(nullptr) {
        if( n ) {
            reserved	= n;
            data	= static_cast<T*>(calloc(reserved, sizeof(T)));
            for( size_t i = 0; i < count; ++i )
                new(&(data[i])) T(elems[i]);
        }
    }

    Array(const Array<T>& v) {
        reserved	= v.reserved;
        count		= v.count;

        data		= static_cast<T*>(calloc(reserved, sizeof(T)));
        for( size_t i = 0; i < count; ++i )
            new(&(data[i])) T(v.data[i]);
    }

    Array(Array<T>&& v) {
        reserved	= v.reserved;
        count		= v.count;
        data        = v.data;
        v.count     = 0;
        v.reserved  = 0;
        v.data      = nullptr;
    }


    ~Array() {
        if( data != nullptr) {
            for( size_t i = 0; i < count; ++i )
            {
                (data[i]).~T();
            }
            free(data);
            count		= 0;
            reserved	= 0;
        }
    }

    const T*
    get() const	{ return data; }

    T*
    get()		{ return data; }

    void
    pushBack(const T& t)	{
        if( count == reserved ) {	// we have reached the limit
            reserved    = (reserved == 0) ? static_cast<size_t>(MIN_VEC_RES_SIZE) : (reserved * 2);

            T*	newData	= static_cast<T*>(calloc(reserved, sizeof(T)));
            for( size_t i = 0; i < count; ++i )
                new(&(newData[i])) T(data[i]);

            // remove old data
            for( size_t i = 0; i < count; ++i )
                (data[i]).~T();

            free(data);

            data	= newData;
        }

        new(&(data[count])) T(t);
        ++count;
    }

    void
    popBack() {
        if( count ) {
            --count;
            (data[count]).~T();
        }
    }

    size_t		size() const			{ return count;	}
    const T&	operator[] (size_t i) const	{ return data[i];	}

    T&		operator[] (size_t i)		{ return data[i];	}

    Array&
    operator = (const Array<T>& v) {
        this->~Array();
        reserved	= v.reserved;
        count		= v.count;

        data		= static_cast<T*>(calloc(reserved, sizeof(T)));
        for( size_t i = 0; i < count; ++i )
            new(&(data[i])) T(v.data[i]);
        return *this;
    }

    Array&
    operator = (Array<T>&& v) {
        this->~Array();
        reserved	= v.reserved;
        count		= v.count;

        data        = v.data;
        v.count     = 0;
        v.reserved  = 0;

        v.data      = nullptr;        return *this;
    }

    void
    clear()	{
        for( size_t i = 0; i < count; ++i ) {
            (data[i]).~T();
        }
        count	= 0;
    }

    void
    resize(size_t newSize)	{
        if( newSize > reserved ) {
            // expand
            reserved	= newSize;

            T*	newData	= static_cast<T*>(calloc(reserved, sizeof(T)));
            assert(newData != 0);
            for( size_t i = 0; i < count; ++i )
            {
                new(&(newData[i])) T(data[i]);
            }

            // remove old data
            for( size_t i = 0; i < count; ++i )
            {
                (data[i]).~T();
            }
            free(data);

            data	= newData;

            // initialize the new new allocated elements
            for( size_t i = count; i < reserved; ++i )
            {
                new(&(data[i])) T();
            }

            count	= reserved;
        } else if( newSize < count ) {
            // shrink and remove data
            for( size_t i = newSize; i < count; ++i ) {
                (data[i]).~T();
            }

            count	= newSize;
        } else if( newSize > count ) {	// and new_size <= __v_reserved

            // initialize new elements in reserved
            for( size_t i = count; i < newSize; ++i )
            {
                new(&(data[i])) T();
            }
            count	= newSize;
        }
    }


private:
    size_t		count;
    size_t		reserved;
    T*			data;
};	// struct Array
}	// BmCpp
