
#include "fs.hpp"
#if defined(BUILD_OS_WINDOWS)
#include <windows.h>
#elif defined(BUILD_OS_MACOSX)
#include <unistd.h>
#include <mach-o/dyld.h>
#endif
#include <stdlib.h>

using std::string;

namespace sweet
{
    
namespace fs
{

fs::Path absolute( const fs::Path& path, const fs::Path& base_path )
{
    if ( fs::Path(path).is_absolute() )
    {
        return path;
    }

    fs::Path absolute_path( base_path );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path;
}

fs::Path relative( const fs::Path& path, const fs::Path& base_path )
{
    return base_path.relative( path );
}

std::string executable( const std::string& path )
{
    if ( fs::Path(path).is_absolute() )
    {
        return path;
    }

#if defined(BUILD_OS_WINDOWS)
    char executable [MAX_PATH + 1];
    int size = ::GetModuleFileNameA( NULL, executable, sizeof(executable) );
    executable [sizeof(executable) - 1] = 0;
#elif defined(BUILD_OS_MACOSX)
    uint32_t size = 0;
    _NSGetExecutablePath( NULL, &size );
    char executable [size];
    _NSGetExecutablePath( executable, &size );
#else
    const char* executable = "";
#endif    

    fs::Path absolute_path( executable );
    absolute_path.pop_back();
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path.string();
}

std::string home( const std::string& path )
{
#if defined (BUILD_OS_WINDOWS)
    const char* HOME = "USERPROFILE";
#elif defined (BUILD_OS_MACOSX)
    const char* HOME = "HOME";
#else
    const char* HOME = "HOME";
#endif
    
    if ( fs::Path(path).is_absolute() )
    {
        return path;
    }

    const char* home = ::getenv( HOME );
    if ( !home )
    {
        return string();
    }

    fs::Path absolute_path( home );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path.string();
}
    
}

}
