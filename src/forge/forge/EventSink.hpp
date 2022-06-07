#pragma once

#include <forge/ForgeEventSink.hpp>

namespace sweet
{

namespace forge
{

class Forge;

class EventSink : public ForgeEventSink
{
private:
    void forge_output( Forge* forge, const char* message );
    void forge_warning( Forge* forge, const char* message );
    void forge_error( Forge* forge, const char* message );
};

}

}
