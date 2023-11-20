
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <limits.h>

#define DYLD_INTERPOSE(_replacement,_replacee) \
    __attribute__((used)) static struct{ const void* replacement; const void* replacee; } _interpose_##_replacee \
    __attribute__ ((section ("__DATA,__interpose"))) = { (const void*)(unsigned long)&_replacement, (const void*)(unsigned long)&_replacee };

namespace
{

static const int FILE_DESCRIPTOR = 3;

static void log_open( int fd, int oflag )
{
    if ( fd >= 0 )
    {
        struct stat stat;
        if ( fstat(fd, &stat) == 0 && (stat.st_mode & S_IFREG) != 0 ) 
        {
            char path [PATH_MAX];
            ssize_t size = fcntl( fd, F_GETPATH, path );
            if ( size >= 0 && size_t(size) < sizeof(path) )
            {
                path[size] = 0;
                const bool read_only = (oflag & (O_WRONLY | O_RDWR)) == 0;
                if ( read_only )
                {
                    struct iovec iovecs[] =
                    {
                        { (void*) "== read '", 9 },
                        { (void*) path, strlen(path) },
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
                        { (void*) path, strlen(path) },
                        { (void*) "'\n", 2 }
                    };
                    size_t written = writev( FILE_DESCRIPTOR, iovecs, 3 );
                    (void) written;
                }
            }
        }
    }
}

}

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
        log_open( fd, oflag );
    }

    return fd;
}
DYLD_INTERPOSE( open_interpose, open );

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
        log_open( fd, oflag );
    }

    return fd;
}
DYLD_INTERPOSE( open$NOCANCEL_interpose, open$NOCANCEL );
#endif

}
