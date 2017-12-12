#ifndef SWEET_BUILD_TOOL_PATH_FUNCTIONS_HPP_INCLUDED
#define SWEET_BUILD_TOOL_PATH_FUNCTIONS_HPP_INCLUDED

#include <boost/filesystem/path.hpp>
#include <string>

namespace sweet
{

namespace build_tool
{

boost::filesystem::path absolute( const boost::filesystem::path& path, const boost::filesystem::path& base_path );
boost::filesystem::path relative( const boost::filesystem::path& path, const boost::filesystem::path& base_path );
boost::filesystem::path make_drive_uppercase( std::string path );

}

}

#endif
