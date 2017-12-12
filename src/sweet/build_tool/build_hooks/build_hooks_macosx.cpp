
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/uio.h>

static const int FILE_DESCRIPTOR = 3;

extern "C" 
{

typedef int (*OriginalOpenFunction)( const char* filename, int oflag, ... );
static OriginalOpenFunction original_open = NULL;
int open( const char* filename, int oflag, ... )
{
    if ( !original_open )
    {
        original_open = (OriginalOpenFunction) dlsym( RTLD_NEXT, "open" );
    }

    int fd = -1;
    if ( oflag & O_CREAT )
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

    if ( fd >= 0 )
    {
        if ( (oflag & (O_WRONLY | O_RDWR)) == 0 )
        {
            struct iovec iovecs[] =
            {
                { (void*) "== read '", 9 },
                { (void*) filename, strlen(filename) },
                { (void*) "'\n", 2 }
            };
            writev( FILE_DESCRIPTOR, iovecs, 3 );
        }
        else
        {
            struct iovec iovecs[] =
            {
                { (void*) "== write '", 10 },
                { (void*) filename, strlen(filename) },
                { (void*) "'\n", 2 }
            };
            writev( FILE_DESCRIPTOR, iovecs, 3 );
        }

        // printf( "== %s '%s'\n", oflag & (O_WRONLY | O_RDWR) ? "write" : "read",  filename );
    }

    return fd;
}

typedef FILE* (*OriginalFopenFunction)( const char* filename, const char* mode );
static OriginalFopenFunction original_fopen = NULL;
FILE* fopen( const char* filename, const char* mode )
{
    if ( !original_fopen )
    {
        original_fopen = (OriginalFopenFunction) dlsym( RTLD_NEXT, "fopen" );
    }

    FILE* file = original_fopen( filename, mode );
    if ( file )
    {
        if ( mode[0] == 'r' )
        {
            struct iovec iovecs[] =
            {
                { (void*) "== read '", 9 },
                { (void*) filename, strlen(filename) },
                { (void*) "'\n", 2 }
            };
            writev( FILE_DESCRIPTOR, iovecs, 3 );
        }
        else
        {
            struct iovec iovecs[] =
            {
                { (void*) "== write '", 10 },
                { (void*) filename, strlen(filename) },
                { (void*) "'\n", 2 }
            };
            writev( FILE_DESCRIPTOR, iovecs, 3 );
        }
    }
    return file;
}

}
