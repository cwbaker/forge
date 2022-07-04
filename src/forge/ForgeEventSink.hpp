#pragma once

namespace sweet
{

namespace forge
{

class Forge;

/**
// An interface for classes that wish to receive events from a 
// Forge.
*/
class ForgeEventSink
{
    public:
        virtual ~ForgeEventSink();
        virtual void forge_output( Forge* forge, const char* message );
        virtual void forge_warning( Forge* forge, const char* message );
        virtual void forge_error( Forge* forge, const char* message );
};

}

}
