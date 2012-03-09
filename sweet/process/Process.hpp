//
// Process.hpp
// Copyright (c) 2008 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PROCESS_PROCESS_HPP_INCLUDED
#define SWEET_PROCESS_PROCESS_HPP_INCLUDED

#include "declspec.hpp"

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


/**
// An operating system process.
*/
class SWEET_PROCESS_DECLSPEC Process
{
    int flags_; ///< The ProcessFlags for this Process.
    void* process_; ///< The handle to this Process.
    void* stdout_; ///< The handle to the pipe used to read from stdout and stderr for this Process.

    public:
        Process();
        Process( const char* command, const char* arguments, const char* directory, int flags );
        ~Process();

        void wait();
        int exit_code();
        size_t read( void* buffer, size_t length );
};

}

}

#endif
