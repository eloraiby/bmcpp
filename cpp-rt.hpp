#ifndef CPPRT_HPP
#define CPPRT_HPP

#include <cstdint>
#include <cstdlib>
#include <cassert>

static inline void fatal(const char* s) { fprintf(stderr, s); abort(); }

// This is a hack for .SO to work when they are compiled against a program/library that uses libstdc++
// these stubs are emitted for default pure virtuals, they are present in the libstdc++ runtime. We will
// have to redirect them to the phantom handler
extern "C" {
static inline __attribute__((noreturn)) void __phantom_handler(void) { fprintf(stderr, "a stub was not implemented!!!\n"); abort(); }

void __cxa_pure_virtual (void) __attribute__ ((weak, alias("__phantom_handler")));
void __cxa_deleted_virtual (void) __attribute__ ((weak, alias("__phantom_handler")));
}

inline void* operator new(size_t, void* p) noexcept { return p; }

namespace BmCpp {

struct BaseAllocation {
    inline void*    operator new(size_t len) noexcept { return malloc(len); }
    inline void*    operator new[](size_t len) noexcept { return malloc(len); }
    inline void     operator delete(void* p) noexcept { return free(p); }
    inline void     operator delete[](void* p) noexcept { return free(p); }
};

class NonCopyable {
protected:
    NonCopyable() {}
    ~NonCopyable() {}
private:  // emphasize the following members are private
    NonCopyable( const NonCopyable& );
    const NonCopyable& operator=( const NonCopyable& );
};


//
// This is utterly some crappy C++ magic, but we need it
// Explaination: pattern matching on type signature and removal of ref indirections
//
template<class _Ty>
struct _RemoveReference
{   // remove reference
    typedef _Ty _type;
};

template<class _Ty>
struct _RemoveReference<_Ty&>
{   // remove reference
    typedef _Ty _type;
};

template<class _Ty>
struct _RemoveReference<_Ty&&>
{   // remove rvalue reference
    typedef _Ty _type;
};

template <typename T>
typename _RemoveReference<T>::_type&& move(T&& arg)
{
    return static_cast<typename _RemoveReference<T>::_type&&>(arg);
}

template<bool Cond, class T = void> struct EnableIf {};
template<class T> struct EnableIf<true, T> { typedef T type; };


template<typename K>
uint32_t    hashFn(const K&);

/**
 * uint32_t -> uint32_t hash, useful for when you're about to trucate this hash but you
 * suspect its low bits aren't well mixed.
 *
 * This is the Murmur3 finalizer.
 */
template<>
inline
uint32_t
hashFn<uint32_t>(const uint32_t& hash) {
    uint32_t    h   = hash;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

}


#endif // CPPRT_HPP
