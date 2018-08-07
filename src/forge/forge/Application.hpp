#ifndef APPLICATION_HPP_INCLUDED
#define APPLICATION_HPP_INCLUDED

#include <forge/ForgeEventSink.hpp>

namespace sweet
{

namespace forge
{

class Forge;

class Application : public ForgeEventSink
{
    int result_;

    public:
        Application( int argc, char** argv );
        int get_result() const;

    private:
        void forge_output( Forge* forge, const char* message );
        void forge_warning( Forge* forge, const char* message );
        void forge_error( Forge* forge, const char* message );
};

}

}

#endif
