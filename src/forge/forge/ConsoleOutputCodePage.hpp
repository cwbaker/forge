#pragma once

namespace sweet
{

namespace forge
{

// Switches the console output code page for the lifetime of this object and
// restores the prior code page on destruction.  Pairs with the UTF-8 active
// code page manifest embedded in forge.exe so that non-ASCII characters in
// target identifiers and paths render correctly.  No-op on non-Windows
// platforms.
class ConsoleOutputCodePage
{
public:
    explicit ConsoleOutputCodePage( unsigned int code_page );
    ~ConsoleOutputCodePage();

private:
    unsigned int prior_code_page_;

    ConsoleOutputCodePage( ConsoleOutputCodePage&& ) = delete;
    ConsoleOutputCodePage( const ConsoleOutputCodePage& ) = delete;
    ConsoleOutputCodePage& operator=( ConsoleOutputCodePage&& ) = delete;
    ConsoleOutputCodePage& operator=( const ConsoleOutputCodePage& ) = delete;
};

}

}
