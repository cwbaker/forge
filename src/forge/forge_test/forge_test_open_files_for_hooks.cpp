
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

int main( int /*argc*/, char** /*argv*/ )
{
    {
        int result = chdir( TEST_DIRECTORY );
        error_if( result != 0, "changing to test directory '%s' failed", TEST_DIRECTORY );
    }

    {
        const char* filename = "hooks.lua";
        FILE* file = fopen( filename, "rb" );
        error_if( !file, "Opening '%s' failed", filename );
        fclose( file );
    }

    {
        const char* filename = TEST_DIRECTORY "hooks.lua";
        FILE* file = fopen( filename, "rb" );
        error_if( !file, "Opening '%s' failed", filename );
        fclose( file );
    }

    {
        const char* filename = "this-file-does-not-exist";
        FILE* file = fopen( filename, "rb" );
        error_if( file, "Opening '%s' was expected to fail", filename );
    }

    {
        const char* filename = TEST_DIRECTORY "this-file-does-not-exist";
        FILE* file = fopen( filename, "rb" );
        error_if( file, "Opening '%s' was expected to fail", filename );
    }

#if defined __linux__ || defined __APPLE__
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
        const char* filename = "this-file-does-not-exist";
        int fd = open( filename, O_RDONLY );
        error_if( fd != -1, "Opening '%s' was expected to fail", filename );    
    }

    {
        const char* filename = TEST_DIRECTORY "this-file-does-not-exist";
        int fd = open( filename, O_RDONLY );
        error_if( fd != -1, "Opening '%s' was expected to fail", filename );    
    }
#endif

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
        error_if( fd < 0, "Opening '%s' failed", filename );    
        close( fd );
    }

    {
        const char* filename = "hooks.lua";
        FILE* file = fopen64( filename, "rb" );
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
        const char* filename = "this-file-does-not-exist";
        FILE* file = fopen64( filename, "rb" );
        error_if( file, "Opening '%s' was expected to fail", filename );
    }

    {
        const char* filename = TEST_DIRECTORY "this-file-does-not-exist";
        FILE* file = fopen64( filename, "rb" );
        error_if( file, "Opening '%s' was expected to fail", filename );
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
        const char* filename = "this-file-does-not-exist";
        int fd = open64( filename, O_RDONLY );
        error_if( fd != -1, "Opening '%s' was expected to fail", filename );    
    }

    {
        const char* filename = TEST_DIRECTORY "this-file-does-not-exist";
        int fd = open64( filename, O_RDONLY );
        error_if( fd != -1, "Opening '%s' was expected to fail", filename );
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

#elif defined _WIN32
    {
        LPCSTR filename = "hooks.lua";
        HANDLE file = CreateFileA(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        error_if( file == INVALID_HANDLE_VALUE, "Opening '%s' failed", filename );
        CloseHandle( file );
    }

    {
        LPCSTR filename = TEST_DIRECTORY "hooks.lua";
        HANDLE file = CreateFileA( filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        error_if( file == INVALID_HANDLE_VALUE, "Opening '%s' failed", filename );
        CloseHandle( file );
    }

    {
        LPCWSTR filename = L"hooks.lua";
        HANDLE file = CreateFileW( filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        error_if( file == INVALID_HANDLE_VALUE, "Opening 'hooks.lua' failed" );
        CloseHandle( file );
    }

    {
        LPCWSTR filename = WIDE_TEST_DIRECTORY L"hooks.lua";
        HANDLE file = CreateFileW( filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        error_if( file == INVALID_HANDLE_VALUE, "Opening 'hooks.lua' failed" );
        CloseHandle( file );
    }

    {
        LPCSTR filename = "hooks.lua";
        HANDLE file = CreateFileTransactedA(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL, NULL, NULL, NULL);
        error_if(file == INVALID_HANDLE_VALUE, "Opening '%s' failed", filename);
        CloseHandle(file);
    }

    {
        LPCSTR filename = TEST_DIRECTORY "hooks.lua";
        HANDLE file = CreateFileTransactedA(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL, NULL, NULL, NULL);
        error_if(file == INVALID_HANDLE_VALUE, "Opening '%s' failed", filename);
        CloseHandle(file);
    }

    {
        LPCWSTR filename = L"hooks.lua";
        HANDLE file = CreateFileTransactedW(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL, NULL, NULL, NULL);
        error_if(file == INVALID_HANDLE_VALUE, "Opening 'hooks.lua' failed");
        CloseHandle(file);
    }

    {
        LPCWSTR filename = WIDE_TEST_DIRECTORY L"hooks.lua";
        HANDLE file = CreateFileTransactedW(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL, NULL, NULL, NULL);
        error_if(file == INVALID_HANDLE_VALUE, "Opening 'hooks.lua' failed");
        CloseHandle(file);
    }
#endif

    return EXIT_SUCCESS;
}
