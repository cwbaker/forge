//
// basic-types.ipp
// Copright (c) 2006  - 2010 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_PERSIST_BASIC_TYPES_IPP_INCLUDED
#define SWEET_PERSIST_PERSIST_BASIC_TYPES_IPP_INCLUDED

namespace sweet
{

namespace persist
{

template <class Archive>
void save( Archive& archive, int mode, const char* name, bool& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE, 1 );
    archive.value( "value", value );
}

template <class Archive>
void load( Archive& archive, int mode, const char* name, bool& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE );
    archive.value( "value", value );
}

template <class Archive>
void resolve( Archive& archive, int mode, bool& value )
{
    ObjectGuard<Archive> guard( archive, 0, 0, MODE_VALUE );
}

template <class Archive>
void save( Archive& archive, int mode, const char* name, char& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE, 1 );
    archive.value( "value", value );
}

template <class Archive>
void load( Archive& archive, int mode, const char* name, char& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE );
    archive.value( "value", value );
}

template <class Archive>
void resolve( Archive& archive, int mode, char& value )
{
    ObjectGuard<Archive> guard( archive, 0, 0, MODE_VALUE );
}

template <class Archive>
void save( Archive& archive, int mode, const char* name, signed char& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE, 1 );
    archive.value( "value", value );
}

template <class Archive>
void load( Archive& archive, int mode, const char* name, signed char& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE );
    archive.value( "value", value );
}

template <class Archive>
void resolve( Archive& archive, int mode, signed char& value )
{
    ObjectGuard<Archive> guard( archive, 0, 0, MODE_VALUE );
}

template <class Archive>
void save( Archive& archive, int mode, const char* name, unsigned char& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE, 1 );
    archive.value( "value", value );
}

template <class Archive>
void load( Archive& archive, int mode, const char* name, unsigned char& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE );
    archive.value( "value", value );
}

template <class Archive>
void resolve( Archive& archive, int mode, unsigned char& value )
{
    ObjectGuard<Archive> guard( archive, 0, 0, MODE_VALUE );
}

template <class Archive>
void save( Archive& archive, int mode, const char* name, wchar_t& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE, 1 );
    archive.value( "value", value );
}

template <class Archive>
void load( Archive& archive, int mode, const char* name, wchar_t& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE );
    archive.value( "value", value );
}

template <class Archive>
void resolve( Archive& archive, int mode, wchar_t& value )
{
    ObjectGuard<Archive> guard( archive, 0, 0, MODE_VALUE );
}

template <class Archive>
void save( Archive& archive, int mode, const char* name, short& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE, 1 );
    archive.value( "value", value );
}

template <class Archive>
void load( Archive& archive, int mode, const char* name, short& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE );
    archive.value( "value", value );
}

template <class Archive>
void resolve( Archive& archive, int mode, short& value )
{
    ObjectGuard<Archive> guard( archive, 0, 0, MODE_VALUE );
}

template <class Archive>
void save( Archive& archive, int mode, const char* name, unsigned short& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE, 1 );
    archive.value( "value", value );
}

template <class Archive>
void load( Archive& archive, int mode, const char* name, unsigned short& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE );
    archive.value( "value", value );
}

template <class Archive>
void resolve( Archive& archive, int mode, unsigned short& value )
{
    ObjectGuard<Archive> guard( archive, 0, 0, MODE_VALUE );
}

template <class Archive>
void save( Archive& archive, int mode, const char* name, int& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE, 1 );
    archive.value( "value", value );
}

template <class Archive>
void load( Archive& archive, int mode, const char* name, int& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE );
    archive.value( "value", value );
}

template <class Archive>
void resolve( Archive& archive, int mode, int& value )
{
    ObjectGuard<Archive> guard( archive, 0, 0, MODE_VALUE );
}

template <class Archive>
void save( Archive& archive, int mode, const char* name, unsigned int& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE, 1 );
    archive.value( "value", value );
}

template <class Archive>
void load( Archive& archive, int mode, const char* name, unsigned int& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE );
    archive.value( "value", value );
}

template <class Archive>
void resolve( Archive& archive, int mode, unsigned int& value )
{
    ObjectGuard<Archive> guard( archive, 0, 0, MODE_VALUE );
}

template <class Archive>
void save( Archive& archive, int mode, const char* name, long& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE, 1 );
    archive.value( "value", value );
}

template <class Archive>
void load( Archive& archive, int mode, const char* name, long& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE );
    archive.value( "value", value );
}

template <class Archive>
void resolve( Archive& archive, int mode, long& value )
{
    ObjectGuard<Archive> guard( archive, 0, 0, MODE_VALUE );
}

template <class Archive>
void save( Archive& archive, int mode, const char* name, unsigned long& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE, 1 );
    archive.value( "value", value );
}

template <class Archive>
void load( Archive& archive, int mode, const char* name, unsigned long& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE );
    archive.value( "value", value );
}

template <class Archive>
void resolve( Archive& archive, int mode, unsigned long& value )
{
    ObjectGuard<Archive> guard( archive, 0, 0, MODE_VALUE );
}

/*
template <class Archive>
void save( Archive& archive, int mode, const char* name, std::time_t& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE, 1 );
    archive.value( "value", value );
}
*/

/*
template <class Archive>
void load( Archive& archive, int mode, const char* name, time_t& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE );
    archive.value( "value", value );
}
*/

/*
template <class Archive>
void resolve( Archive& archive, int mode, std::time_t& value )
{
    ObjectGuard<Archive> guard( archive, 0, 0, MODE_VALUE );
}
*/

template <class Archive>
void save( Archive& archive, int mode, const char* name, float& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE, 1 );
    archive.value( "value", value );
}

template <class Archive>
void load( Archive& archive, int mode, const char* name, float& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE );
    archive.value( "value", value );
}

template <class Archive>
void resolve( Archive& archive, int mode, float& value )
{
    ObjectGuard<Archive> guard( archive, 0, 0, MODE_VALUE );
}

template <class Archive>
void save( Archive& archive, int mode, const char* name, double& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE, 1 );
    archive.value( "value", value );
}

template <class Archive>
void load( Archive& archive, int mode, const char* name, double& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );

    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE );
    archive.value( "value", value );
}

template <class Archive>
void resolve( Archive& archive, int mode, double& value )
{
    ObjectGuard<Archive> guard( archive, 0, 0, MODE_VALUE );
}

}

}

#endif
