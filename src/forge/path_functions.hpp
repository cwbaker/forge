#ifndef FORGE_PATH_FUNCTIONS_HPP_INCLUDED
#define FORGE_PATH_FUNCTIONS_HPP_INCLUDED

#include <filesystem>
#include <string>

namespace sweet
{

namespace forge
{

std::filesystem::path absolute( const std::filesystem::path& path, const std::filesystem::path& base_path );
std::filesystem::path relative( const std::filesystem::path& path, const std::filesystem::path& base_path );
std::filesystem::path make_drive_uppercase( std::string path );
std::filesystem::path search_up_for_root_directory( const std::string& directory, const std::string& filename );

}

}

#endif
