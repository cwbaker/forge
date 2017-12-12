//
// ImportDescriptor.cpp
// Copyright (c) Charles Baker.  All rights reserved.
//

#include "ImportDescriptor.hpp"
#include <sweet/assert/assert.hpp>
#include <windows.h>
#include <winternl.h>
#include <psapi.h>
#include <stdio.h>

using std::uintptr_t;
using namespace sweet::build_tool;

ImportDescriptor::ImportDescriptor( std::uintptr_t base_address, const char* name )
: base_address_( base_address ),
  import_descriptor_( NULL )
{
	SWEET_ASSERT( base_address_ != 0 );
	SWEET_ASSERT( name );
	import_descriptor_ = find_import_descriptor_for_library( name );
}

bool ImportDescriptor::valid() const
{
	return base_address_ != 0 && import_descriptor_;
}

void ImportDescriptor::patch_function( const char* name, void* function, void** original_function ) const
{
    IMAGE_THUNK_DATA* thunk = find_thunk_for_function( name );
    if ( thunk )
    {
        DWORD original_protection;
        MEMORY_BASIC_INFORMATION memory_information;
        VirtualQuery( thunk, &memory_information, sizeof(memory_information) );
        VirtualProtect( memory_information.BaseAddress, memory_information.RegionSize, PAGE_EXECUTE_READWRITE, &original_protection );
        if ( *original_function == NULL )
        {
            *original_function = (void*) (uintptr_t) thunk->u1.Function;
        }
        thunk->u1.Function = (uintptr_t) function;
        VirtualProtect( memory_information.BaseAddress, memory_information.RegionSize, original_protection, NULL );
    }
}

IMAGE_IMPORT_DESCRIPTOR* ImportDescriptor::find_import_descriptor_for_library( const char* name ) const
{
	SWEET_ASSERT( name );	

    const unsigned short MZ = 0x5a4d;
    IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*) base_address_;
    if ( dos_header->e_magic != MZ )
    {
        return NULL;
    }

    IMAGE_NT_HEADERS* nt_headers = (IMAGE_NT_HEADERS*) address( dos_header->e_lfanew );
    if ( !nt_headers )
    {
        return NULL;
    }

    if ( nt_headers->OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_IMPORT )
    {
        return NULL;
    }
    
    IMAGE_IMPORT_DESCRIPTOR* import_descriptor = (IMAGE_IMPORT_DESCRIPTOR*) address( nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress );
    if ( import_descriptor )
    {
        while ( import_descriptor->Characteristics != 0 && strcmp(name, string(import_descriptor->Name)) != 0 )
        {
            ++import_descriptor;
        }
    }
    return import_descriptor && import_descriptor->Characteristics != 0 ? import_descriptor : NULL;
}

IMAGE_THUNK_DATA* ImportDescriptor::find_thunk_for_function( const char* name ) const
{
    IMAGE_THUNK_DATA* thunk = (IMAGE_THUNK_DATA*) address( import_descriptor_->FirstThunk );
    IMAGE_THUNK_DATA* original_thunk = (IMAGE_THUNK_DATA*) address( import_descriptor_->OriginalFirstThunk );
    while ( original_thunk->u1.Function != 0 )
    {
        if ( !(original_thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) )
        {
            IMAGE_IMPORT_BY_NAME* import_by_name = (IMAGE_IMPORT_BY_NAME*) address( original_thunk->u1.AddressOfData );
            if ( strcmp(import_by_name->Name, name) == 0 )
            {
                return thunk;
            }
        }
        ++thunk;
        ++original_thunk;
    }
    return NULL;
}

void* ImportDescriptor::address( intptr_t offset ) const
{
    return offset != 0 ? (void*) (base_address_ + offset) : NULL;
}

const char* ImportDescriptor::string( intptr_t offset ) const
{
    return (const char*) address( offset );
}
