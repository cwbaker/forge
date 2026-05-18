//
// ConsoleOutputCodePage.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "stdafx.hpp"
#include "ConsoleOutputCodePage.hpp"

#ifdef _WIN32
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
