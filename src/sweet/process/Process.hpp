#ifndef SWEET_PROCESS_PROCESS_HPP_INCLUDED
#define SWEET_PROCESS_PROCESS_HPP_INCLUDED

#include "declspec.hpp"

#if defined(BUILD_OS_MACOSX)
#include <sys/types.h>
#endif

namespace sweet
{

namespace process
{

/**
// Process flags.
*/
enum ProcessFlags
{
    PROCESS_FLAG_NONE = 0x00,
    PROCESS_FLAG_PROVIDE_STDOUT_AND_STDERR = 0x02
};

class Environment;

/**
// An operating system process.
*/
class SWEET_PROCESS_DECLSPEC Process
{
    int flags_; ///< The ProcessFlags for this Process.

#if defined(BUILD_OS_WINDOWS)
    void* process_; ///< The handle to this Process.
    void* stdout_; ///< The handle to the pipe used to read from stdout and stderr for this Process.
#endif

#if defined(BUILD_OS_MACOSX)
    pid_t process_;
    int stdout_;
    int exit_code_;
#endif

    public:
        Process();
        Process( const char* command, const char* arguments, const char* directory, const Environment* environment, int flags );
        ~Process();

        void wait();
        int exit_code();
        unsigned int read( void* buffer, unsigned int length );
};

}

}

#endif
