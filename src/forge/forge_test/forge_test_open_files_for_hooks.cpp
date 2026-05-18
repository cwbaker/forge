
#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#if defined __linux__ || defined __APPLE__
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#elif defined _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <windows.h>
#include <direct.h>
#define chdir _chdir
#endif

static void error_if( bool condition, const char* format, ... )
{
    if ( condition )
    {
        va_list args;
        va_start( args, format );
        fputs( "forge_test_open_files_for_hooks: ", stderr );
        vfprintf( stderr, format, args );
#if defined __linux__ || defined __APPLE__
        fputs( " - ", stderr );
        fputs( strerror(errno), stderr );
#elif defined _WIN32
        fputs( " - ", stderr );
        fputs( strerror(GetLastError()), stderr );
#endif
        fputs( "\n", stderr );
        va_end( args );
        exit( EXIT_FAILURE );
    }
}

int main( int argc, char** argv )
{
    {
        int result = chdir( TEST_DIRECTORY );
        error_if( result != 0, "changing to test directory '%s' failed", TEST_DIRECTORY );
    }

    for ( int i = 1; i < argc; ++i )
    {
        const char* filename = argv[i];

        {
            FILE* file = fopen( filename, "rb" );
            if ( file ) fclose( file );
        }

#if defined __linux__ || defined __APPLE__
        {
            int fd = open( filename, O_RDONLY );
            if ( fd >= 0 ) close( fd );
        }
#endif

#if defined __linux__
        {
            FILE* file = fopen64( filename, "rb" );
            if ( file ) fclose( file );
        }

        {
            int fd = open64( filename, O_RDONLY );
            if ( fd >= 0 ) close( fd );
        }
#endif

#if defined _WIN32
        {
            HANDLE file = CreateFileA( filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
            if ( file != INVALID_HANDLE_VALUE ) CloseHandle( file );
        }

        {
            wchar_t wfilename[32768];
            MultiByteToWideChar( CP_UTF8, 0, filename, -1, wfilename, 32768 );
            HANDLE file = CreateFileW( wfilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
            if ( file != INVALID_HANDLE_VALUE ) CloseHandle( file );
        }

        {
            HANDLE file = CreateFileTransactedA( filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL, NULL, NULL, NULL );
            if ( file != INVALID_HANDLE_VALUE ) CloseHandle( file );
        }

        {
            wchar_t wfilename[32768];
            MultiByteToWideChar( CP_UTF8, 0, filename, -1, wfilename, 32768 );
            HANDLE file = CreateFileTransactedW( wfilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL, NULL, NULL, NULL );
            if ( file != INVALID_HANDLE_VALUE ) CloseHandle( file );
        }
#endif
    }

#if defined __linux__
    {
        const char* filename = ".";
        int fd = open( filename, O_WRONLY | O_TMPFILE, S_IRUSR | S_IWUSR );
        error_if( fd < 0, "Opening '%s' failed", filename );
        close( fd );
    }

    {
        const char* filename = TEST_DIRECTORY ".";
        int fd = open( filename, O_WRONLY | O_TMPFILE, S_IRUSR | S_IWUSR );
        error_if( fd < 0, "opening '%s' failed", filename );
        close( fd );
    }

    int dirfd = open( ".", O_RDONLY );
    error_if( dirfd < 0, "opening '%s' failed", "." );
    int result = chdir( ".." );
    error_if( result != 0, "changing to parent directory failed" );

    for ( int i = 1; i < argc; ++i )
    {
        const char* filename = argv[i];
        int fd = openat( dirfd, filename, O_RDONLY );
        if ( fd >= 0 ) close( fd );
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
#endif

    return EXIT_SUCCESS;
}
