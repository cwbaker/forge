
#include "path_functions.hpp"
#include <sweet/assert/assert.hpp>

namespace sweet
{
	
namespace build_tool
{

/**
// Prepend \e base_path to \e path to create an absolute path.
//
// @return
//  The absolute path created by prepending \e base_path to \e path.
*/
boost::filesystem::path absolute( const boost::filesystem::path& path, const boost::filesystem::path& base_path )
{
    if ( path.is_absolute() )
    {
        return path;
    }

    boost::filesystem::path absolute_path( base_path );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path;
}

/**
// Express \e path as a path relative to \e base_path.
//
// If this \e base_path is empty or \e base_path and \e path are on different
// drives then no conversion is done and \e path is returned.
//
// @param path
//  The path to get a relative path to.
//
// @param base_path
//  The path to convert \e path to be relative from.
// 
// @return
//  The path \e path expressed relative to \e base_path.
*/
boost::filesystem::path relative( const boost::filesystem::path& path, const boost::filesystem::path& base_path )
{
    // If the base path is empty or the path are on different drives then no 
    // conversion is done.
    if ( base_path.empty() || (base_path.has_root_name() && path.has_root_name() && base_path.root_name() != path.root_name()) )
    {
        return path;
    }

    // Find the first elements that are different in both of the paths.
    boost::filesystem::path::const_iterator i = base_path.begin();
    boost::filesystem::path::const_iterator j = path.begin();
    while ( i != base_path.end() && j != path.end() && *i == *j )
    {
        ++i;
        ++j;
    }
    
    // Add a leading parent element ("..") for each element that remains in the 
    // base path.
    const char* PARENT = "..";
    const char* SEPARATOR = "/";
    std::string relative_path;
    while ( i != base_path.end() )
    {
        relative_path.append( PARENT );
        relative_path.append( SEPARATOR );
        ++i;
    }

    // Add the remaining elements from the related path.
    if ( j != path.end() )
    {
        relative_path.append( j->generic_string() );
        ++j;
        while ( j != path.end() ) 
        {
            relative_path.append( SEPARATOR );
            relative_path.append( j->generic_string() );
            ++j;
        }
    }
    
    return boost::filesystem::path( relative_path );
}

boost::filesystem::path make_drive_uppercase( std::string path )
{
#if defined BUILD_OS_WINDOWS
    SWEET_ASSERT( path.size() >= 2 );
    SWEET_ASSERT( path[1] == ':' );
    if ( path.size() >= 2 && path[1] == ':' )
    {        
        path[0] = toupper( path[0] );
    }
    return boost::filesystem::path( path );
#else
    return boost::filesystem::path( path );
#endif
}

}

}
