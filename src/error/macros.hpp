#ifndef SWEET_ERROR_ERROR_MACROS_HPP_INCLUDED
#define SWEET_ERROR_ERROR_MACROS_HPP_INCLUDED

#include <build.hpp>

#ifdef SWEET_EXCEPTIONS_ENABLED
#define SWEET_ERROR( e ) throw e;
#else
#include "functions.hpp"
#define SWEET_ERROR( e ) sweet::error::error( e );
#endif

#endif
