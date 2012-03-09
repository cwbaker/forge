//
// BasicPath.hpp
// Copyright (c) 2006 - 2012 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PATH_PATH_BASICPATH_HPP_INCLUDED
#define SWEET_PATH_PATH_BASICPATH_HPP_INCLUDED

#include <string>

namespace sweet
{

namespace path
{

/**
// A filesystem %path.
*/
template <class String, class Traits>
class BasicPath
{
    public:
        typedef typename std::vector<String>::iterator iterator; ///< An iterator over elements in a BasicPath.
        typedef typename std::vector<String>::iterator reverse_iterator; ///< A reverse iterator over elements in a BasicPath.
        typedef typename std::vector<String>::const_iterator const_iterator; ///< A const iterator over elements in a BasicPath.
        typedef typename std::vector<String>::const_reverse_iterator const_reverse_iterator; ///< A const reverse iterator over elements in a BasicPath.

    private:
        bool absolute_; ///< Whether or not this BasicPath is absolute.
        String drive_; ///< The drive that this BasicPath is for.
        std::vector<String> elements_; ///< The elements of this BasicPath including drive if it has one.
        mutable bool dirty_; ///< Whether or not \e string_ is in synch with \e drive_ and \e elements_.
        mutable String string_; ///< The cached string returned by BasicPath::string().

    public:
        BasicPath();
        BasicPath( const BasicPath& path );
        BasicPath& operator=( const BasicPath& path );
        BasicPath( const String& path );

        bool is_absolute() const;
        bool is_relative() const;
        bool is_portable() const;
        bool empty() const;

        const String& string() const;
        String native_string() const;
        const String& drive() const;
        BasicPath branch() const;
        const String& leaf() const;
        String basename() const;
        String extension() const;
        BasicPath relative( const BasicPath& path ) const;

        BasicPath& normalize();
        BasicPath& operator/=( const BasicPath& path );
        BasicPath operator/( const BasicPath& path ) const;

        iterator begin();
        iterator end();
        reverse_iterator rbegin();
        reverse_iterator rend();
        const_iterator begin() const;
        const_iterator end() const;
        const_reverse_iterator rbegin() const;
        const_reverse_iterator rend() const;
        
    private:
        static bool in( int character, const typename Traits::char_type* separators );
        static bool compare( typename String::const_iterator begin, typename String::const_iterator end, const typename Traits::char_type* value );
};

typedef BasicPath<std::string, BasicPathTraits<char> > Path;
typedef BasicPath<std::wstring, BasicPathTraits<wchar_t> > WidePath;

WidePath SWEET_PATH_DECLSPEC current_working_directory();

}

}

#endif
