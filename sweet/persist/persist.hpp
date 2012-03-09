//
// persist.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_HPP_INCLUDED
#define SWEET_PERSIST_HPP_INCLUDED

#include "types.hpp"
#include "functions.hpp"
#include "objects.ipp"
#include "void_pointers.ipp"
#include "pointers.ipp"
#include "PathFilter.hpp"
#include "EnumFilter.hpp"
#include "MaskFilter.hpp"
#include "TextWriter.hpp"
#include "XmlReader.hpp"
#include "XmlWriter.hpp"
#include "JsonReader.hpp"
#include "JsonWriter.hpp"
#include "LuaReader.hpp"
#include "LuaWriter.hpp"
#include "BinaryReader.hpp"
#include "BinaryWriter.hpp"
#include "Error.hpp"
#include "basic-types.ipp"
#include "strings.ipp"
#include "sequences.ipp"
#include "arrays.ipp"
#include "ReaderType.ipp"
#include "Reader.ipp"
#include "WriterType.ipp"
#include "Writer.ipp"
#include "Resolver.ipp"
#include "ObjectGuard.ipp"
#include "TextReader.ipp"
#include "TextWriter.ipp"
#include "XmlReader.ipp"
#include "XmlWriter.ipp"
#include "JsonReader.ipp"
#include "JsonWriter.ipp"
#include "LuaReader.ipp"
#include "LuaWriter.ipp"
#include "BinaryReader.ipp"
#include "BinaryWriter.ipp"

#ifndef BUILD_MODULE_PERSIST
#pragma comment( lib, "persist" BUILD_LIBRARY_SUFFIX )
#endif

namespace sweet
{

/**
// Persistence library.
// 
// A library that provides a simple way to %persist the state 
// of objects to and from files and streams.
*/
namespace persist
{

/**
// Limits.
*/
enum Limits
{
    MAX_FORMAT_LENGTH = 256 ///< The maximum length of the format string for an Archive.
};

}

}

#endif

