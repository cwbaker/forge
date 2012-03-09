//
// Error.hpp
// Copyright (c) 2008 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PROCESS_ERROR_HPP_INCLUDED
#define SWEET_PROCESS_ERROR_HPP_INCLUDED

#include "declspec.hpp"
#include <sweet/error/Error.hpp>
#include <sweet/error/ErrorTemplate.hpp>

namespace sweet
{

namespace process
{

/**
// Uniquely identifies errors that may occur in the %Process library.
*/
enum ErrorCode
{
    PROCESS_ERROR_NONE,                           ///< No error.
    PROCESS_ERROR_CREATING_PIPE_FAILED,           ///< Creating a pipe failed.
    PROCESS_ERROR_DUPLICATING_HANDLE_FAILED,      ///< Duplicating a handle failed.
    PROCESS_ERROR_EXECUTING_PROCESS_FAILED,       ///< Executing a process failed.
    PROCESS_ERROR_WAIT_FOR_PROCESS_FAILED,        ///< Waiting for a process to exit failed.
    PROCESS_ERROR_EXIT_CODE_FOR_PROCESS_FAILED,   ///< Getting the exit code from a process failed.
    PROCESS_ERROR_READING_PIPE_FAILED             ///< Reading from a pipe failed.
};

/**
// The base class for exceptions thrown from the %Process library.
*/
class SWEET_PROCESS_DECLSPEC Error : public error::Error
{
    public:
        Error( int error );
};

/**
// Creating a pipe failed.
*/
typedef error::ErrorTemplate<PROCESS_ERROR_CREATING_PIPE_FAILED, Error> CreatingPipeFailedError;

/**
// Duplicating a handle failed.
*/
typedef error::ErrorTemplate<PROCESS_ERROR_DUPLICATING_HANDLE_FAILED, Error> DuplicatingHandleFailedError;

/**
// Executing a process failed.
*/
typedef error::ErrorTemplate<PROCESS_ERROR_EXECUTING_PROCESS_FAILED, Error> ExecutingProcessFailedError;

/**
// Waiting for a process to exit failed.
*/
typedef error::ErrorTemplate<PROCESS_ERROR_WAIT_FOR_PROCESS_FAILED, Error> WaitForProcessFailedError;

/**
// Getting the exit code of a process failed.
*/
typedef error::ErrorTemplate<PROCESS_ERROR_EXIT_CODE_FOR_PROCESS_FAILED, Error> ExitCodeForProcessFailedError;

/**
// Reading from a pipe failed.
*/
typedef error::ErrorTemplate<PROCESS_ERROR_READING_PIPE_FAILED, Error> ReadingPipeFailedError;

}

}

#endif
