//
// Error.hpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved.
//

#ifndef SWEET_THREAD_ERROR_HPP_INCLUDED
#define SWEET_THREAD_ERROR_HPP_INCLUDED

#include "declspec.hpp"
#include <sweet/error/Error.hpp>
#include <sweet/error/ErrorTemplate.hpp>

namespace sweet
{

namespace thread
{

/**
// Unique codes for the errors that occur in the %Thread library.
*/
enum ErrorCode
{
    THREAD_ERROR_NONE, ///< No error occured.
    THREAD_ERROR_CREATING_THREAD_FAILED, ///< Creating a thread failed.
    THREAD_ERROR_JOINING_THREAD_FAILED, ///< Joining a thread failed.
    THREAD_ERROR_EXIT_CODE_THREAD_FAILED ///< Getting the exit code of a thread failed.
};

/**
// The base class for errors thrown from the %Thread library.
*/
class SWEET_THREAD_DECLSPEC Error : public error::Error
{
    public:
        Error( int error );
};

/** 
// Creating a thread failed. 
*/
typedef error::ErrorTemplate<THREAD_ERROR_CREATING_THREAD_FAILED, Error> CreatingThreadFailedError;

/** 
// Joining a thread failed. 
*/
typedef error::ErrorTemplate<THREAD_ERROR_JOINING_THREAD_FAILED, Error> JoiningThreadFailedError;

/** 
// Getting the exit code of a thread failed. 
*/
typedef error::ErrorTemplate<THREAD_ERROR_EXIT_CODE_THREAD_FAILED, Error> ExitCodeThreadFailedError;

}

}

#endif
