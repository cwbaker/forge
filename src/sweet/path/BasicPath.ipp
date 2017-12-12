//
// BasicPath.ipp
// Copyright (c) 2006 - 2012 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PATH_BASICPATH_IPP_INCLUDED
#define SWEET_PATH_BASICPATH_IPP_INCLUDED

#include <iterator>

namespace sweet
{

namespace path
{

/**
// Constructor.
*/
template <class String, class Traits>
BasicPath<String, Traits>::BasicPath()
: absolute_( false ),
  drive_(),
  elements_(),
  dirty_( true ),
  string_()
{
}

/**
// Copy constructor.
// 
// @param path
//  The BasicPath to copy.
*/
template <class String, class Traits>
BasicPath<String, Traits>::BasicPath( const BasicPath& path )
: absolute_( path.absolute_ ),
  drive_( path.drive_ ), 
  elements_( path.elements_ ),
  dirty_( true ),
  string_()
{
}

/**
// Assignment operator.
//
// @param path
//  The BasicPath to assign from.
// 
// @return
//  This BasicPath.
*/
template <class String, class Traits>
BasicPath<String, Traits>& 
BasicPath<String, Traits>::operator=( const BasicPath& path )
{
    if ( this != &path )
    {
        absolute_ = path.absolute_;
        drive_    = path.drive_;
        elements_ = path.elements_;
        dirty_    = true;
    }
    return *this;
}

/**
// Constructor.
// 
// @param path
//  The path to construct this BasicPath from.
*/
template <class String, class Traits>
BasicPath<String, Traits>::BasicPath( const String& path )
: absolute_( false ),
  drive_(),
  elements_(),
  dirty_( true ),
  string_()
{
//
// Break the path into elements delimited by '/' or '\' characters.
//
    typename String::const_iterator begin = path.begin();
    typename String::const_iterator end = path.begin();
    while ( end != path.end() && !in(*end, Traits::SEPARATORS) )
    {
        ++end;
    }

//
// The first element is treated specially in case it is the drive that the
// path begins on (e.g. C:\, D:\, etc).
//
    if ( begin != path.end() )
    {
        if ( std::find(begin, end, Traits::DRIVE) != end )
        {
            for ( typename String::const_iterator i = begin; i != end; ++i )
            {
                drive_.push_back( std::toupper(*i) );
            }
            
            elements_.push_back( drive_ );
            absolute_ = true;

            begin = end != path.end() ? ++end : end;
            while ( end != path.end() && !in(*end, Traits::SEPARATORS) )
            {
                ++end;
            }
        }
        else if ( begin == end )
        {
            absolute_ = true;

            begin = end != path.end() ? ++end : end;
            while ( end != path.end() && !in(*end, Traits::SEPARATORS) )
            {
                ++end;
            }
        }
    }

//
// If the BasicPath is absolute (i.e. begins at the root) then ignore any leading
// parent ("..") directory elements as these just evaluate to the root again.
//
    if ( absolute_ )
    {
        while ( begin != path.end() && compare(begin, end, Traits::PARENT) )
        {
            begin = end != path.end() ? ++end : end;
            while ( end != path.end() && !in(*end, Traits::SEPARATORS) )
            {
                ++end;
            }            
        }
    }

//
// Add each element to the vector of elements that is not a current (".")
// directory element.
//
    while ( begin != path.end() )
    {
        if ( !compare(begin, end, Traits::CURRENT) )
        {
            elements_.push_back( String() );
            elements_.back().assign( begin, end );
        }

        begin = end != path.end() ? ++end : end;
        while ( end != path.end() && !in(*end, Traits::SEPARATORS) )
        {
            ++end;
        }
    }
}

/**
// Is this BasicPath an absolute path?
//
// @return
//  True if this BasicPath is an absolute path otherwise false.
*/
template <class String, class Traits>
bool BasicPath<String, Traits>::is_absolute() const
{
    return absolute_;
}

/**
// Is this BasicPath a relative path?
//
// @return
//  True if this BasicPath is a relative path otherwise false.
*/
template <class String, class Traits>
bool BasicPath<String, Traits>::is_relative() const
{
    return !absolute_;
}

/**
// Is this BasicPath portable?
//
// A BasicPath is considered portable if it doesn't specify a drive at the 
// begining.
//
// @return
//  True if this BasicPath is portable otherwise false.
*/
template <class String, class Traits>
bool BasicPath<String, Traits>::is_portable() const
{
    return drive_.empty();
}

/**
// Is this BasicPath empty?
//
// A BasicPath is empty if it contains no elements.
//
// @return
//  True if this BasicPath is empty otherwise false.
*/
template <class String, class Traits>
bool BasicPath<String, Traits>::empty() const
{
    return elements_.empty();
}

/**
// Get this BasicPath as a string.
//
// @return
//  This BasicPath as a string.
*/
template <class String, class Traits>
const String& BasicPath<String, Traits>::string() const
{
//
// If this BasicPath has changed since the last time the cached string was built 
// then rebuild it by:
//
// 1. Appending all of this BasicPath's elements delimited with '/' characters.
// 2. Erasing the trailing '/'.
//
    if ( dirty_ )
    {
        string_.clear();

        if ( is_absolute() && drive_.empty() )
        {
            string_.append( Traits::SEPARATOR );
        }

        for ( typename std::vector<String>::const_iterator i = elements_.begin(); i != elements_.end(); ++i )
        {
            string_.append( *i );
            string_.append( Traits::SEPARATOR );
        }

        if ( !string_.empty() )
        {
            string_.erase( string_.end() - 1, string_.end() );
        }

        dirty_ = false;
    }

    return string_;
}

/**
// Get this BasicPath as a string native to the host platform.
//
// @return
//  The BasicPath as a native string.
*/
template <class String, class Traits>
String BasicPath<String, Traits>::native_string() const
{
//
// If this BasicPath has been changed since the last time the cached string was 
// built then rebuild it by:
//
// 1. Appending the drive to the native string.
// 1. Appending all of this BasicPath's elements delimited with '\' characters.
// 2. Erasing the trailing '\'.
//
    String native_string;

    if ( is_absolute() && drive_.empty() )
    {
        native_string.append( Traits::NATIVE_SEPARATOR );
    }

    for ( typename std::vector<String>::const_iterator i = elements_.begin(); i != elements_.end(); ++i )
    {
        native_string.append( *i );
        native_string.append( Traits::NATIVE_SEPARATOR );
    }

    if ( !native_string.empty() )
    {
        native_string.erase( native_string.end() - 1, native_string.end() );
    }

    return native_string;
}

/**
// Get the drive part of this BasicPath (including the ':' character).
//
// @return
//  The drive or an empty string if this BasicPath has no drive.
*/
template <class String, class Traits>
const String& BasicPath<String, Traits>::drive() const
{
    return drive_;
}

/**
// Get the branch part of this BasicPath (all but the last element).
//
// @return
//  The branch part.
*/
template <class String, class Traits>
BasicPath<String, Traits> BasicPath<String, Traits>::branch() const
{
    BasicPath path( *this );
    if ( !path.empty() )
    {
        path.elements_.pop_back();
    }

    return path;
}

/**
// Get the leaf part of this BasicPath (the last element).
//
// @return
//  The leaf part.
*/
template <class String, class Traits>
const String& BasicPath<String, Traits>::leaf() const
{
    if ( elements_.empty() )
    {
        string_.clear();
    }

    return elements_.empty() ? string_ : elements_.back();
}

/**
// Get the basename of this BasicPath (the last element stripped of any 
// characters after and including the last period).
//
// @return
//  The basename.
*/
template <class String, class Traits>
String BasicPath<String, Traits>::basename() const
{
    String basename;
    
    if ( !elements_.empty() )
    {
        const String& leaf = elements_.back();
        typename String::size_type pos = leaf.find_last_of( Traits::DELIMITER );
        if ( pos != String::npos )
        {
            basename.assign( leaf.begin(), leaf.begin() + leaf.find_last_of(Traits::DELIMITER) );
        }
        else
        {
            basename.assign( leaf.begin(), leaf.end() );
        }
    }

    return basename;
}

/**
// Get the extension of this BasicPath (the characters from the last element 
// after and including the last period).
//
// @return
//  The extension.
*/
template <class String, class Traits>
String BasicPath<String, Traits>::extension() const
{
    String extension;

    if ( !elements_.empty() )
    {
        const String& leaf = elements_.back();
        typename String::size_type pos = leaf.find_last_of( Traits::DELIMITER );
        if ( pos != String::npos )
        {
            extension.assign( leaf.begin() + leaf.find_last_of(Traits::DELIMITER), leaf.end() );
        }
    }

    return extension;
}

/**
// Get a BasicPath that expresses the same BasicPath as \e path but relative 
// to this BasicPath.
//
// If this BasicPath is empty or this BasicPath and \e path are on different 
// drives then no conversion is done and \e path is returned.
//
// @param path
//  The BasicPath to get a relative path to from this BasicPath.
// 
// @return
//  The relative BasicPath.
*/
template <class String, class Traits>
BasicPath<String, Traits> BasicPath<String, Traits>::relative( const BasicPath& path ) const
{
//
// If this BasicPath is empty or the BasicPaths are on different drives then no 
// conversion is done.
//
    if ( empty() || (!drive().empty() && !path.drive().empty() && drive() != path.drive()) )
    {
        return path;
    }

//
// Find the first elements that are different in both of the BasicPaths.
//
    typename std::vector<String>::const_iterator i = elements_.begin();
    typename std::vector<String>::const_iterator j = path.elements_.begin();

    while ( i != elements_.end() && j != path.elements_.end() && *i == *j )
    {
        ++i;
        ++j;
    }
    
//
// Add a leading parent element ("..") for each element that remains in the 
// base BasicPath.
//
    std::vector<String> elements;
    while ( i != elements_.end() )
    {
        elements.push_back( Traits::PARENT );
        ++i;
    }

//
// Add the remaining elements from the related BasicPath.
//
    while ( j != path.elements_.end() )
    {
        elements.push_back( *j );
        ++j;
    }

//
// Construct a BasicPath to return and set it's members correctly.
//
    BasicPath relative_path;
    relative_path.absolute_ = false;
    std::swap( relative_path.elements_, elements );

    return relative_path;
}

/**
// Normalize a path by removing any adjacent adjacent name and parent 
// elements.
//
// @return
//  This BasicPath.
*/
template <class String, class Traits>
BasicPath<String, Traits>& BasicPath<String, Traits>::normalize()
{
    iterator prev = elements_.begin();
    iterator curr = prev != elements_.end() ? prev + 1 : elements_.end();
    while ( curr != elements_.end() )
    {
        if ( *prev != Traits::PARENT && *curr == Traits::PARENT )
        {
            iterator next = elements_.erase( prev, curr + 1 );
            prev = next != elements_.begin() ? next - 1 : elements_.begin();
            curr = prev != elements_.end() ? prev + 1 : elements_.end();
        }
        else
        {
            ++prev;
            ++curr;
        }
    }

    return *this;
}

/**
// Append \e path to this BasicPath.
//
// @param path
//  The BasicPath to append to this BasicPath.
// 
// @return
//  This BasicPath.
*/
template <class String, class Traits>
BasicPath<String, Traits>& BasicPath<String, Traits>::operator/=( const BasicPath& path )
{
    dirty_ = true;
    elements_.insert( elements_.end(), path.elements_.begin(), path.elements_.end() );
    return *this;
}

/**
// Append \e path to the end of this BasicPath.
//
// If this BasicPath is empty then just \e path is returned.
//
// @param path
//  The BasicPath to append to this BasicPath.
// 
// @return
//  The resulting BasicPath.
*/
template <class String, class Traits>
BasicPath<String, Traits> BasicPath<String, Traits>::operator/( const BasicPath& path ) const
{
    return empty() ? path : BasicPath(*this) /= path;
}

/**
// Get an iterator to the begining element in this BasicPath.
//
// @return
//  An iterator to the begining element.
*/
template <class String, class Traits>
typename BasicPath<String, Traits>::iterator BasicPath<String, Traits>::begin()
{
    return elements_.begin();
}

/**
// Get an iterator to one past the last element in this BasicPath.
//
// @return
//  An iterator to one past the last element.
*/
template <class String, class Traits>
typename BasicPath<String, Traits>::iterator BasicPath<String, Traits>::end()
{
    return elements_.end();
}

/**
// Get a reverse iterator to the begining element in this BasicPath.
//
// @return
//  An iterator to the begining element.
*/
template <class String, class Traits>
typename BasicPath<String, Traits>::reverse_iterator BasicPath<String, Traits>::rbegin()
{
    return elements_.rbegin();
}

/**
// Get an iterator to one past the last element in this BasicPath.
//
// @return
//  A reverse iterator to one past the last element.
*/
template <class String, class Traits>
typename BasicPath<String, Traits>::reverse_iterator BasicPath<String, Traits>::rend()
{
    return elements_.rend();
}

/**
// Get a const_iterator to the begining element in this BasicPath.
//
// @return
//  A const_iterator to the begining element.
*/
template <class String, class Traits>
typename BasicPath<String, Traits>::const_iterator BasicPath<String, Traits>::begin() const
{
    return elements_.begin();
}

/**
// Get a const_iterator to one past the last element in this BasicPath.
//
// @return
//  A const_iterator to one past the last element.
*/
template <class String, class Traits>
typename BasicPath<String, Traits>::const_iterator BasicPath<String, Traits>::end() const
{
    return elements_.end();
}

/**
// Get a const_iterator to the begining element in this BasicPath.
//
// @return
//  A const_iterator to the begining element.
*/
template <class String, class Traits>
typename BasicPath<String, Traits>::const_reverse_iterator BasicPath<String, Traits>::rbegin() const
{
    return elements_.rbegin();
}

/**
// Get a const_iterator to one past the last element in this BasicPath.
//
// @return
//  A const_iterator to one past the last element.
*/
template <class String, class Traits>
typename BasicPath<String, Traits>::const_reverse_iterator BasicPath<String, Traits>::rend() const
{
    return elements_.rend();
}

/**
// Is \e character in \e separators?
//
// @param character
//  The character to look for in \e separators.
//
// @param separators
//  A null terminated sequence of characters to match \e character against.
//
// @return
//  True if \e character is contained in \e separators otherwise false.
*/
template <class String, class Traits>
bool BasicPath<String, Traits>::in( int character, const typename Traits::char_type* separators )
{
    SWEET_ASSERT( separators );    
    const typename Traits::char_type* i = separators;
    while ( *i != 0 && *i != character )
    {
        ++i;
    }    
    return *i == character;
}

/**
// Compare [\e begin, \e end) with the null terminated string \e value.
//
// @param begin
//  The iterator at the first character of the sequence to compare with
//  \e value.
//
// @param end
//  The iterator once past the last character of the sequence to compare with
//  \e value.
//
// @param value
//  The null terminated string to compare with [\e begin, \e end).
//
// @return
//  True if [\e begin, \e end) matches \e value otherwise false.
*/
template <class String, class Traits>
bool BasicPath<String, Traits>::compare( typename String::const_iterator begin, typename String::const_iterator end, const typename Traits::char_type* value )
{
    SWEET_ASSERT( value );    
    while ( begin != end && *value != 0 && *begin == *value )
    {
        ++begin;
        ++value;
    }    
    return begin == end;
}

}

}

#endif
