#ifndef FORGE_IMAGEIMPORTDESCRIPTOR_HPP_INCLUDED
#define FORGE_IMAGEIMPORTDESCRIPTOR_HPP_INCLUDED

#include <cstdint>
#include <windows.h>
#include <winternl.h>
#include <psapi.h>

namespace sweet
{

namespace forge
{

class ImportDescriptor
{
	std::uintptr_t base_address_;
	IMAGE_IMPORT_DESCRIPTOR* import_descriptor_;

public:
	ImportDescriptor( std::uintptr_t base_address, const char* name );
	bool valid() const;
	void patch_function( const char* name, void* function, void** original_function ) const;

private:
	IMAGE_IMPORT_DESCRIPTOR* find_import_descriptor_for_library( const char* name ) const;
	IMAGE_THUNK_DATA* find_thunk_for_function( const char* name ) const;
	void* address( intptr_t offset ) const;
	const char* string( intptr_t offset ) const;
};

}

}

#endif
