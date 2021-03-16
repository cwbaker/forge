
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/stat.h>

namespace
{

static const int FILE_DESCRIPTOR = 3;

static void log_file_access( const char* filename, bool read_only )
{
    if ( read_only )
    {
        struct iovec iovecs[] =
        {
            { (void*) "== read '", 9 },
            { (void*) filename, strlen(filename) },
            { (void*) "'\n", 2 }
        };
        size_t written = writev( FILE_DESCRIPTOR, iovecs, 3 );
        (void) written;
    }
    else
    {
        struct iovec iovecs[] =
        {
            { (void*) "== write '", 10 },
            { (void*) filename, strlen(filename) },
            { (void*) "'\n", 2 }
        };
        size_t written = writev( FILE_DESCRIPTOR, iovecs, 3 );
        (void) written;
    }
}

static void log_file_access( int fd, const char* filename, int oflag )
{
    if ( fd >= 0 )
    {
        struct stat stat;
        if ( fstat(fd, &stat) == 0 && (stat.st_mode & S_IFREG) != 0 ) 
        {
            log_file_access( filename, (oflag & (O_WRONLY | O_RDWR)) == 0 );
        }
    }
}

}

extern "C" 
{

int open( const char* filename, int oflag, ... )
{
    typedef int (*OpenFunction)( const char*, int, ... );
    static OpenFunction original_open = nullptr;
    if ( !original_open )
    {
        original_open = (OpenFunction) dlsym( RTLD_NEXT, "open" );
    }

    int fd = -1;
    if ( oflag & (O_CREAT | O_TMPFILE) )
    {
        va_list args;
        va_start( args, oflag );
        mode_t mode = va_arg( args, int );
        fd = original_open( filename, oflag, mode );
        va_end( args );
    }
    else
    {
        fd = original_open( filename, oflag );
    }
    log_file_access( fd, filename, oflag );
    return fd;
}

int open64( const char* filename, int oflag, ... )
{
    typedef int (*Open64Function)( const char*, int, ... );
    static Open64Function original_open64 = nullptr;
    if ( !original_open64 )
    {
        original_open64 = (Open64Function) dlsym( RTLD_NEXT, "open64" );
    }

    int fd = -1;
    if ( oflag & (O_CREAT | O_TMPFILE) )
    {
        va_list args;
        va_start( args, oflag );
        mode_t mode = va_arg( args, int );
        fd = original_open64( filename, oflag, mode );
        va_end( args );
    }
    else
    {
        fd = original_open64( filename, oflag );
    }
    log_file_access( fd, filename, oflag );
    return fd;
}

int openat( int dirfd, const char* filename, int oflag, ... )
{
    typedef int (*OpenAtFunction)( int, const char*, int, ... );
    static OpenAtFunction original_openat = nullptr;
    if ( !original_openat )
    {
        original_openat = (OpenAtFunction) dlsym( RTLD_NEXT, "openat" );
    }

    int fd = -1;
    if ( oflag & (O_CREAT | O_TMPFILE) )
    {
        va_list args;
        va_start( args, oflag );
        mode_t mode = va_arg( args, int );
        fd = original_openat( dirfd, filename, oflag, mode );
        va_end( args );
    }
    else
    {
        fd = original_openat( dirfd, filename, oflag );
    }
    log_file_access( fd, filename, oflag );
    return fd;    
}

FILE* fopen( const char* filename, const char* mode )
{
    typedef FILE* (*FopenFunction)( const char*, const char* );
    static FopenFunction original_fopen = (FopenFunction) dlsym( RTLD_NEXT, "fopen" );

    FILE* file = original_fopen( filename, mode );
    if ( file )
    {
        log_file_access( filename, mode[0] == 'r' );
    }
    return file;
}

FILE* fopen64( const char* filename, const char* mode )
{
    typedef FILE* (*Fopen64Function)( const char*, const char* );
    static Fopen64Function original_fopen64 = (Fopen64Function) dlsym( RTLD_NEXT, "fopen64" );

    FILE* file = original_fopen64( filename, mode );
    if ( file )
    {
        log_file_access( filename, mode[0] == 'r' );
    }
    return file;
}

}
