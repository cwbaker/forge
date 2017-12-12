
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/stat.h>

static const int FILE_DESCRIPTOR = 3;

extern "C" 
{

int open( const char* filename, int oflag, ... )
{
	typedef int (*OpenFunction)(const char*, int, ...);
    static OpenFunction original_open = nullptr;
    if ( !original_open )
    {
        original_open = (OpenFunction) dlsym( RTLD_NEXT, "open" );
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
        struct stat stat;
        if ( fstat(fd, &stat) == 0 && (stat.st_mode & S_IFREG) != 0 ) 
        {
            if ( (oflag & (O_WRONLY | O_RDWR)) == 0 )
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
    }
    return fd;
}

}
