#ifndef SWEET_BUILD_TOOL_ERROR_HPP_INCLUDED
#define SWEET_BUILD_TOOL_ERROR_HPP_INCLUDED

#include <sweet/error/Error.hpp>
#include <sweet/error/ErrorTemplate.hpp>

namespace sweet
{

namespace build_tool
{

/**
// Uniquely identifies errors that may occur in the build library.
*/
enum ErrorCode
{
    BUILD_TOOL_ERROR_NONE, ///< No error.
    BUILD_TOOL_ERROR_SCANNING_FILE_FAILED, ///< Scanning a source file failed.
    BUILD_TOOL_ERROR_GRAPH_VERSION_INVALID, ///< The version of a saved Graph is too low to load.
    BUILD_TOOL_ERROR_ROOT_FILE_NOT_FOUND, ///< The root file (probably build.lua) could not be found.
    BUILD_TOOL_ERROR_SCANNED_FILE_NOT_FOUND, ///< A Target passed to a scan for a file that doesn't exist.
    BUILD_TOOL_ERROR_ENVIRONMENT_VARIABLE_NOT_FOUND, ///< An environment variable was not found.
    BUILD_TOOL_ERROR_OPERATING_SYSTEM_CALL_FAILED, ///< A call to the operating system failed.
    BUILD_TOOL_ERROR_INVALID_BIND_TYPE, ///< An invalid BindType was passed to create a TargetPrototype or set BindType for a Target.
    BUILD_TOOL_ERROR_PROTOTYPE_CONFLICT, ///< The same Target has been created with two different TargetPrototypes.
    BUILD_TOOL_ERROR_INITAL_TARGET_NOT_FOUND, ///< The initial target passed to construct a Graph was not found.
    BUILD_TOOL_ERROR_POSTORDER_CALLED_RECURSIVELY, ///< The postorder() function has been called recursively.
    BUILD_TOOL_ERROR_PREORDER_CALLED_RECURSIVELY, ///< The preorder() function has been called recursively.
    BUILD_TOOL_ERROR_RUNTIME_ERROR, ///< An error occured while executing a Lua script.
    BUILD_TOOL_ERROR_READING_PIPE_FAILED ///< Reading from a pipe failed.
};

/**
// The base class for exceptions thrown from the Build Tool library.
*/
class Error : public error::Error
{
    public:
        Error( int error );
};

/**
// Scanning a source file failed.
*/
typedef error::ErrorTemplate<BUILD_TOOL_ERROR_SCANNING_FILE_FAILED, Error> ScanningFileFailedError;

/**
// The version of a saved Graph is too low to load.
*/
typedef error::ErrorTemplate<BUILD_TOOL_ERROR_GRAPH_VERSION_INVALID, Error> GraphVersionInvalidError;

/**
// The root file (probably Buildfile.default) could not be found between the 
// initial directory and the root of the filesystem.
*/
typedef error::ErrorTemplate<BUILD_TOOL_ERROR_ROOT_FILE_NOT_FOUND, Error> RootFileNotFoundError;

/**
// A Target passed to a scan for a file that doesn't exist.
*/
typedef error::ErrorTemplate<BUILD_TOOL_ERROR_SCANNED_FILE_NOT_FOUND, Error> ScannedFileNotFoundError;

/**
// An environment variable was not found.
*/
typedef error::ErrorTemplate<BUILD_TOOL_ERROR_ENVIRONMENT_VARIABLE_NOT_FOUND, Error> EnvironmentVariableNotFoundError;

/**
// A call to the operating system failed.
*/
typedef error::ErrorTemplate<BUILD_TOOL_ERROR_OPERATING_SYSTEM_CALL_FAILED, Error> OperatingSystemCallFailedError;

/**
// An invalid BindType was passed to create a TargetPrototype or set BindType for a Target.
*/
typedef error::ErrorTemplate<BUILD_TOOL_ERROR_INVALID_BIND_TYPE, Error> InvalidBindTypeError;

/**
// The same target has been created with two different prototypes.
*/
typedef error::ErrorTemplate<BUILD_TOOL_ERROR_PROTOTYPE_CONFLICT, Error> PrototypeConflictError;

/**
// The initial target passed to construct a Graph was not found.
*/
typedef error::ErrorTemplate<BUILD_TOOL_ERROR_INITAL_TARGET_NOT_FOUND, Error> InitialTargetNotFoundError;

/**
// The postorder() function has been called recursively.
*/
typedef error::ErrorTemplate<BUILD_TOOL_ERROR_POSTORDER_CALLED_RECURSIVELY, Error> PostorderCalledRecursivelyError;

/**
// The preorder() function has been called recursively.
*/
typedef error::ErrorTemplate<BUILD_TOOL_ERROR_PREORDER_CALLED_RECURSIVELY, Error> PreorderCalledRecursivelyError;

/**
// An error occured while executing a Lua script.
*/
typedef error::ErrorTemplate<BUILD_TOOL_ERROR_RUNTIME_ERROR, Error> RuntimeError;

/**
// Reading from a pipe failed.
*/
typedef error::ErrorTemplate<BUILD_TOOL_ERROR_READING_PIPE_FAILED, Error> ReadingPipeFailedError;

}

}

#endif
