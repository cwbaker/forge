#ifndef FORGE_PATH_FUNCTIONS_HPP_INCLUDED
#define FORGE_PATH_FUNCTIONS_HPP_INCLUDED

#include <boost/filesystem/path.hpp>
#include <string>

namespace sweet
{

namespace forge
{

boost::filesystem::path absolute( const boost::filesystem::path& path, const boost::filesystem::path& base_path );
boost::filesystem::path relative( const boost::filesystem::path& path, const boost::filesystem::path& base_path );
boost::filesystem::path make_drive_uppercase( std::string path );
boost::filesystem::path search_up_for_root_directory( const std::string& directory, const std::string& filename );

}

}

#endif
