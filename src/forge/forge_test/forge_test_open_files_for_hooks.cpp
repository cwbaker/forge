#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#if defined __linux__
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#endif

static void error_if( bool condition, const char* format, ... )
{
    if ( condition )
    {
        va_list args;
        va_start( args, format );
        fputs( "forge_test_open_files_for_hooks: ", stderr );
        vfprintf( stderr, format, args );
#if defined __linux__
        fputs( " - ", stderr );
        fputs( strerror(errno), stderr );
#endif
        fputs( "\n", stderr );
        va_end( args );
        exit( EXIT_FAILURE );
    }
}

int main( int /*argc*/, char** /*argv*/ )
{
    {
        const char* filename = "hooks.lua";
        FILE* file = fopen( "hooks.lua", "rb" );
        error_if( !file, "Opening '%s' failed", filename );
        fclose( file );
    }

    {
        const char* filename = TEST_DIRECTORY "hooks.lua";
        FILE* file = fopen( filename, "rb" );
        error_if( !file, "Opening '%s' failed", filename );
        fclose( file );
    }

#if defined __linux__
    {
        const char* filename = "hooks.lua";
        FILE* file = fopen64( "hooks.lua", "rb" );
        error_if( !file, "Opening '%s' failed", filename );
        fclose( file );
    }

    {
        const char* filename = TEST_DIRECTORY "hooks.lua";
        FILE* file = fopen64( filename, "rb" );
        error_if( !file, "Opening '%s' failed", filename );
        fclose( file );
    }

    {
        const char* filename = "hooks.lua";
        int fd = open( filename, O_RDONLY );
        error_if( fd < 0, "Opening '%s' failed", filename );    
        close( fd );
    }

    {
        const char* filename = TEST_DIRECTORY "hooks.lua";
        int fd = open( filename, O_RDONLY );
        error_if( fd < 0, "Opening '%s' failed", filename );    
        close( fd );
    }

    {
        const char* filename = "hooks.lua";
        int fd = open64( filename, O_RDONLY );
        error_if( fd < 0, "Opening '%s' failed", filename );    
        close( fd );
    }

    {
        const char* filename = TEST_DIRECTORY "hooks.lua";
        int fd = open64( filename, O_RDONLY );
        error_if( fd < 0, "Opening '%s' failed", filename );    
        close( fd );
    }

    {
        const char* filename = ".";
        int fd = open( filename, O_WRONLY | O_TMPFILE, S_IRUSR | S_IWUSR );
        error_if( fd < 0, "Opening '%s' failed", filename );    
        close( fd );
    }

    {
        const char* filename = TEST_DIRECTORY ".";
        int fd = open( filename, O_WRONLY | O_TMPFILE, S_IRUSR | S_IWUSR );
        error_if( fd < 0, "Opening '%s' failed", filename );    
        close( fd );
    }

    int dirfd = open( ".", O_RDONLY );
    error_if( dirfd < 0, "Opening '%s' failed", "." );
    int result = chdir( ".." );
    error_if( result != 0, "changing to parent directory failed" );

    {
        const char* filename = "hooks.lua";
        int fd = openat( dirfd, filename, O_RDONLY );
        error_if( fd < 0, "Opening '%s' failed", filename );    
        close( fd );
    }

    {
        const char* filename = TEST_DIRECTORY "hooks.lua";
        int fd = openat( dirfd, filename, O_RDONLY );
        error_if( fd < 0, "Opening '%s' failed", filename );    
        close( fd );
    }

    {
        const char* filename = TEST_DIRECTORY ".";
        int fd = openat( dirfd, filename, O_WRONLY | O_TMPFILE, S_IRUSR | S_IWUSR );
        error_if( fd < 0, "Opening '%s' failed", filename );    
        close( fd );
    }

    result = chdir( "forge_test" );
    error_if( result != 0, "changing directory to 'forge_test' failed" );
    close( dirfd );
    dirfd = -1;
#else
#endif

    return EXIT_SUCCESS;
}
