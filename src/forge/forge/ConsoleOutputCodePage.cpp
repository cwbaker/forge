//
// ConsoleOutputCodePage.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "ConsoleOutputCodePage.hpp"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

using namespace sweet::forge;

ConsoleOutputCodePage::ConsoleOutputCodePage( unsigned int code_page )
#ifdef _WIN32
: prior_code_page_( GetConsoleOutputCP() )
{
    SetConsoleOutputCP( code_page );
}
#else
: prior_code_page_( 0 )
{
    (void) code_page;
    (void) prior_code_page_;
}
#endif

ConsoleOutputCodePage::~ConsoleOutputCodePage()
{
#ifdef _WIN32
    SetConsoleOutputCP( prior_code_page_ );
#endif
}
