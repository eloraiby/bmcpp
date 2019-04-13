#ifndef STRING_HPP
#define STRING_HPP
#include <cstring>
#include "array.hpp"
namespace BmCpp {

///
/// RTK string implementation
///
struct String
{
    inline String()	{ data.pushBack('\0');	}

    inline String(const String& other) : data(other.data)	{}

    inline String(const char* other) {
        if( other ) {
            size_t len	= strlen(other);

            data.resize(len + 1);
            strcpy(&(data[0]), other);
        } else {
            data.pushBack('\0');
        }
    }

    inline String(char s) {
        data.pushBack(s);
        data.pushBack('\0');
    }

    inline ~String() {}

    inline void
    clear()	{
        data.resize(1);
        data[0]	= '\0';
    }

    inline String&
    operator = (const String& s) {
        if( &s != this )	// an idiot is trying to copy himself ?
            data	= s.data;
        return *this;
    }

    inline String&
    operator += (const String& s) {
        if( &s == this )
        {
            String	scpy(s);
            *this	+= scpy;
        }
        else
        {
            size_t	len	= data.size();
            data.resize(data.size() + s.data.size() - 1);
            strcpy(&(data[len - 1]), &(s.data[0]));
        }

        return *this;
    }

    inline String&
    operator = (const char* s)
    {
        size_t len	= strlen(s);

        data.resize(len + 1);
        strcpy(&(data[0]), s);
        return *this;
    }

    inline String&
    operator += (const char* s)
    {
        size_t	slen	= strlen(s);
        size_t	len     = data.size();
        data.resize(data.size() + slen);
        strcpy(&(data[len - 1]), s);

        return *this;
    }

    inline String&
    operator = (char s)
    {
        data.resize(2);
        data[0]	= s;
        data[1]	= '\0';
        return *this;
    }

    inline String&
    operator += (char s)
    {
        data[data.size() - 1]	= s;
        data.pushBack('\0');
        return *this;
    }

    inline bool
    operator == (const String& s) const
    {
        return (strcmp(&(s.data[0]), &(data[0])) == 0);
    }

    inline bool
    operator != (const String& s) const
    {
        return (strcmp(&(s.data[0]), &(data[0])) != 0);
    }

    inline bool
    operator < (const String& s) const
    {
        return (strcmp(&(data[0]), &(s.data[0])) < 0 );
    }

    inline bool
    operator > (const String& s) const
    {
        return (strcmp(&(data[0]), &(s.data[0])) > 0 );
    }

    inline String
    operator + (const String& s) const
    {
        String	temp(*this);
        return (temp += s);
    }

    inline String
    operator + (const char* s) const
    {
        String	temp(*this);
        return (temp += s);
    }

    inline size_t		length() const	{	return data.size() - 1;	}
    inline size_t		size() const	{	return data.size() - 1;	}

    inline char		operator[] (size_t i) const	{		return data[i];	}
    inline char&		operator[] (size_t i)		{		return data[i];	}

    inline const char*	c_str() const			{	return &(data[0]);		}

private:
    Array<char>		data;		///< the actual string data
};	// struct string

inline String operator + (const char* cstr, const String& str) {	return (String(cstr) + str);	}


///
/// make a string upper case
/// @param str the string
/// @return the upper cased string
///
inline String toUpper(const String& str)
{
    String res;
    for( size_t i = 0; i < str.size(); ++i )
        if( str[i] >= 'a' && str[i] <= 'z' )
            res	+= (str[i] - 'a') + 'A';
        else
            res	+= str[i];
    return res;

}

///
/// make a string lower case
/// @param str the string
/// @return the lower cased string
///
inline String toLower(const String& str)
{
    String res;
    for( size_t i = 0; i < str.size(); ++i )
        if( str[i] >= 'A' && str[i] <= 'Z' )
            res	+= (str[i] - 'A') + 'a';
        else
            res	+= str[i];
    return res;
}

template<>
inline
uint32_t
hashFn<String>(const String& s) {
    uint32_t    h   = 0;
    for( size_t i = 0; i < s.size(); ++i ) {
        h ^= s[i] >> 16;
        h *= 0x85ebca6b;
        h ^= h >> 13;
        h *= 0xc2b2ae35;
        h ^= h >> 16;
    }
    return h;
}
}   // namespace BmCpp

#endif // STRING_HPP
