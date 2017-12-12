
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/uio.h>

#define DYLD_INTERPOSE(_replacement,_replacee) \
   __attribute__((used)) static struct{ const void* replacement; const void* replacee; } _interpose_##_replacee \
            __attribute__ ((section ("__DATA,__interpose"))) = { (const void*)(unsigned long)&_replacement, (const void*)(unsigned long)&_replacee };

static const int FILE_DESCRIPTOR = 3;

extern "C" 
{

int open_interpose( const char* filename, int oflag, ... )
{
    int fd = -1;
    if ( oflag & O_CREAT )
    {
        va_list args;
        va_start( args, oflag );
        mode_t mode = va_arg( args, int );
        fd = open( filename, oflag, mode );
        va_end( args );
    }
    else
    {
        fd = open( filename, oflag );
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
    }

    return fd;
}
DYLD_INTERPOSE( open_interpose, open );

FILE* fopen_interpose( const char* filename, const char* mode )
{
    FILE* file = fopen( filename, mode );
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
DYLD_INTERPOSE( fopen_interpose, fopen );

#ifdef __x86_64__
extern int open$NOCANCEL( const char* filename, int oflag, ... );
int open$NOCANCEL_interpose( const char* filename, int oflag, ... )
{
    int fd = -1;
    if ( oflag & O_CREAT )
    {
        va_list args;
        va_start( args, oflag );
        mode_t mode = va_arg( args, int );
        fd = open$NOCANCEL( filename, oflag, mode );
        va_end( args );
    }
    else
    {
        fd = open$NOCANCEL( filename, oflag );
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
    }

    return fd;
}
DYLD_INTERPOSE( open$NOCANCEL_interpose, open$NOCANCEL );
#endif

}
