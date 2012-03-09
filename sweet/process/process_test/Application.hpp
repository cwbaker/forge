//
// Application.hpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved.
//

#ifndef APPLICATION_HPP_INCLUDED
#define APPLICATION_HPP_INCLUDED

namespace sweet
{

namespace process
{

/**
// Test application for the %Process library.
*/
class Application
{
    int m_result; ///< The result to return to the operating system.

    public:
        Application( int argc, char** argv );
        int result() const;

    private:
        void test_main();
        void test_print_to_stdout();
        void test_print_to_stderr();
};

}

}

#endif
