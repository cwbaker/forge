//
// BasicPath.cpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include <sweet/path/path.hpp>

using namespace sweet::path;

/**
// Get the current working directory.
//
// @return
//  The current working directory.
*/
WidePath sweet::path::current_working_directory()
{
#if defined BUILD_PLATFORM_MSVC || defined BUILD_PLATFORM_MINGW
//
// The length returned by GetCurrentDirectoryW includes the terminating
// null character so the wstring is initialized to have one less character 
// than that.
//   
    DWORD length = ::GetCurrentDirectoryW( 0, NULL );
    std::wstring directory( length - 1, L'\0' );
    ::GetCurrentDirectoryW( length, &directory[0] );
    return WidePath( directory );
#else    
#error "The function sweet::path::current_working_directory() is not implemented for this platform."
#endif
}
