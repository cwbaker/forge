#ifndef SWEET_ERROR_FUNCTIONS_HPP_INCLUDED
#define SWEET_ERROR_FUNCTIONS_HPP_INCLUDED

namespace std
{

class exception;

}

namespace sweet
{

namespace error
{

class Error;
class ErrorPolicy;

void set_error_policy( ErrorPolicy* error_policy );
ErrorPolicy* get_error_policy();
void error( const Error& error );
void error( const std::exception& exception );
const char* format( int error, char* buffer, unsigned int length );

}

}

#endif
