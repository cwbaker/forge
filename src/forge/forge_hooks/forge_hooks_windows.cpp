
#include "ImportDescriptor.hpp"
#include <windows.h>
#include <winternl.h>
#include <psapi.h>
#include <stdio.h>

using namespace sweet::forge;

typedef HANDLE (WINAPI *CreateFileAFunction)( 
    LPCSTR filename, 
    DWORD desired_access, 
    DWORD share_mode, 
    LPSECURITY_ATTRIBUTES security_attributes, 
    DWORD creation_disposition, 
    DWORD flags, 
    HANDLE template_file
);

typedef HANDLE (WINAPI *CreateFileWFunction)( 
    LPCWSTR filename, 
    DWORD desired_access, 
    DWORD share_mode, 
    LPSECURITY_ATTRIBUTES security_attributes, 
    DWORD creation_disposition, 
    DWORD flags, 
    HANDLE template_file 
);

typedef HANDLE (WINAPI *CreateFileTransactedAFunction)(
    LPCSTR filename,
    DWORD desired_access,
    DWORD share_mode,
    LPSECURITY_ATTRIBUTES security_attributes,
    DWORD creation_disposition,
    DWORD flags_and_attributes,
    HANDLE template_file,
    HANDLE transaction,
    PUSHORT mini_version,
    PVOID extended_parameter
);

typedef HANDLE (WINAPI *CreateFileTransactedWFunction)(
    LPCWSTR lpFileName,
    DWORD desired_access,
    DWORD share_mode,
    LPSECURITY_ATTRIBUTES security_attributes,
    DWORD creation_disposition,
    DWORD flags_and_attributes,
    HANDLE template_file,
    HANDLE transaction,
    PUSHORT mini_version,
    PVOID extended_parameter
);

typedef NTSTATUS (WINAPI *NtOpenFileFunction)(
    PHANDLE file_handle,
    ACCESS_MASK desired_access,
    POBJECT_ATTRIBUTES object_attributes,
    PIO_STATUS_BLOCK io_status_block,
    ULONG share_access,
    ULONG open_options
);

typedef NTSTATUS (WINAPI *NtCreateFileFunction)(
    PHANDLE file_handle,
    ACCESS_MASK desired_access,
    POBJECT_ATTRIBUTES object_attributes,
    PIO_STATUS_BLOCK io_status_block,
    PLARGE_INTEGER allocation_size,
    ULONG file_attributes,
    ULONG share_access,
    ULONG creation_disposition,
    ULONG create_options,
    PVOID ea_buffer,
    ULONG ea_length
);

static HANDLE build_hooks_dependencies_pipe = INVALID_HANDLE_VALUE;
static CreateFileAFunction original_create_file_a = NULL;
static CreateFileWFunction original_create_file_w = NULL;
static CreateFileTransactedAFunction original_create_file_transacted_a = NULL;
static CreateFileTransactedWFunction original_create_file_transacted_w = NULL;
static NtOpenFileFunction original_nt_open_file = NULL;
static NtCreateFileFunction original_nt_create_file = NULL;

// Skip the "\\?\" or "\??\" prefixes used to disable string parsing in
// Windows APIs.  Forge scripts that handle these are expecting paths
// without these prefixes.
// See https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file#win32-file-namespaces.
static const char* skip_win32_file_namespace( const char* path )
{
    if ( path[0] == '\\' && (path[1] == '\\' || path[1] == '?') && path[2] == '?' && path[3] == '\\' )
    {
        return path + 4;
    }
    return path;
}

static void log_access( HANDLE file, bool read_only )
{
    char namespaced_path [32767];
    DWORD result = GetFinalPathNameByHandleA( file, namespaced_path, sizeof(namespaced_path), VOLUME_NAME_DOS );
    if (result > 0)
    {
        DWORD bytes_written = 0;
        HANDLE pipe = build_hooks_dependencies_pipe;
        const char* path = skip_win32_file_namespace( namespaced_path );
        const char* access = read_only ? "== read '" : "== write '";
        WriteFile( pipe, access, (DWORD) strlen(access), &bytes_written, NULL );
        WriteFile( pipe, path, (DWORD) strlen(path), &bytes_written, NULL );
        WriteFile( pipe, "'\n", 2, &bytes_written, NULL );    
    }
}

static HANDLE WINAPI create_file_a_hook( LPCSTR filename, DWORD desired_access, DWORD share_mode, LPSECURITY_ATTRIBUTES security_attributes, DWORD creation_disposition, DWORD flags, HANDLE template_file )
{
    HANDLE handle = (*original_create_file_a)(
        filename,
        desired_access,
        share_mode,
        security_attributes,
        creation_disposition,
        flags,
        template_file
    );
    if ( handle != INVALID_HANDLE_VALUE && (flags & FILE_ATTRIBUTE_TEMPORARY) == 0 )
    {
        const bool read_only = (desired_access & GENERIC_WRITE) == 0;
        log_access( handle, read_only );
    }
    return handle;
}

static HANDLE WINAPI create_file_w_hook( LPCWSTR wide_filename, DWORD desired_access, DWORD share_mode, LPSECURITY_ATTRIBUTES security_attributes, DWORD creation_disposition, DWORD flags, HANDLE template_file )
{
    HANDLE handle = original_create_file_w(
        wide_filename,
        desired_access,
        share_mode,
        security_attributes,
        creation_disposition,
        flags,
        template_file
    );
    if ( handle != INVALID_HANDLE_VALUE && (flags & FILE_ATTRIBUTE_TEMPORARY) == 0 )
    {
        char filename [MAX_PATH + 1];
        int count = WideCharToMultiByte( CP_UTF8, 0, wide_filename, (int) wcslen(wide_filename), filename, (int) sizeof(filename), NULL, NULL );
        filename[count] = 0;
        const bool read_only = (desired_access & GENERIC_WRITE) == 0;
        log_access( handle, read_only );
    }
    return handle;
}

static HANDLE WINAPI create_file_transacted_a_hook( LPCSTR filename, DWORD desired_access, DWORD share_mode, LPSECURITY_ATTRIBUTES security_attributes, DWORD creation_disposition, DWORD flags, HANDLE template_file, HANDLE transaction, PUSHORT mini_version, PVOID extended_parameter )
{
    HANDLE handle = (*original_create_file_transacted_a)(
        filename,
        desired_access,
        share_mode,
        security_attributes,
        creation_disposition,
        flags,
        template_file,
        transaction,
        mini_version,
        extended_parameter
    );
    if ( handle != INVALID_HANDLE_VALUE && (flags & FILE_ATTRIBUTE_TEMPORARY) == 0 )
    {
        const bool read_only = (desired_access & GENERIC_WRITE) == 0;
        log_access( handle, read_only );
    }
    return handle;
}

static HANDLE WINAPI create_file_transacted_w_hook( LPCWSTR wide_filename, DWORD desired_access, DWORD share_mode, LPSECURITY_ATTRIBUTES security_attributes, DWORD creation_disposition, DWORD flags, HANDLE template_file, HANDLE transaction, PUSHORT mini_version, PVOID extended_parameter )
{
    HANDLE handle = (*original_create_file_transacted_w)(
        wide_filename,
        desired_access,
        share_mode,
        security_attributes,
        creation_disposition,
        flags,
        template_file,
        transaction,
        mini_version,
        extended_parameter
    );
    if ( handle != INVALID_HANDLE_VALUE && (flags & FILE_ATTRIBUTE_TEMPORARY) == 0 )
    {
        char filename [MAX_PATH + 1];
        int count = WideCharToMultiByte( CP_UTF8, 0, wide_filename, (int) wcslen(wide_filename), filename, (int) sizeof(filename), NULL, NULL );
        filename[count] = 0;
        const bool read_only = (desired_access & GENERIC_WRITE) == 0;
        log_access( handle, read_only );
    }
    return handle;
}

static NTSTATUS WINAPI nt_create_file_hook( PHANDLE file_handle, ACCESS_MASK desired_access, POBJECT_ATTRIBUTES object_attributes, PIO_STATUS_BLOCK io_status_block, PLARGE_INTEGER allocation_size, ULONG file_attributes, ULONG share_access, ULONG creation_disposition, ULONG create_options, PVOID ea_buffer, ULONG ea_length )
{
    NTSTATUS status = (*original_nt_create_file)(
        file_handle,
        desired_access,
        object_attributes,
        io_status_block,
        allocation_size,
        file_attributes,
        share_access,
        creation_disposition,
        create_options,
        ea_buffer,
        ea_length
    );
    if ( NT_SUCCESS(status) )
    {
        LPCWSTR wide_path = object_attributes->ObjectName->Buffer;
        char path [MAX_PATH + 1];
        int count = WideCharToMultiByte( CP_UTF8, 0, wide_path, (int) wcslen(wide_path), path, (int) sizeof(path), NULL, NULL );
        path[count] = 0;
        const bool read_only = (desired_access & (FILE_WRITE_DATA | FILE_APPEND_DATA)) == 0;
        log_access( path, read_only );
    }
    return status;
}

static void patch_iat( HMODULE module )
{
    HMODULE modules [256];
    DWORD size = 0;
    BOOL success = EnumProcessModulesEx( GetCurrentProcess(), modules, sizeof(modules), &size, LIST_MODULES_64BIT );
    int count = size / sizeof(HMODULE);
    for ( int i = 0; i < count - 1; ++i )
    {
        char filename [MAX_PATH];
        if ( GetModuleFileName(modules[i], filename, sizeof(filename)) )
        {
            uintptr_t base_address = (uintptr_t) modules[i];            
            ImportDescriptor kernel32( base_address, "KERNEL32.DLL" );
            if ( kernel32.valid() )
            {
                kernel32.patch_function( "CreateFileA", (void*) &create_file_a_hook, (void**) &original_create_file_a );
                kernel32.patch_function( "CreateFileW", (void*) &create_file_w_hook, (void**) &original_create_file_w );
                kernel32.patch_function( "CreateFileTransactedA", (void*) &create_file_transacted_a_hook, (void**) &original_create_file_transacted_a );
                kernel32.patch_function( "CreateFileTransactedW", (void*) &create_file_transacted_w_hook, (void**) &original_create_file_transacted_w );
            }

            ImportDescriptor ntdll( base_address, "ntdll.dll" );
            if ( ntdll.valid() )
            {
                ntdll.patch_function( "NtCreateFile", (void*) &nt_create_file_hook, (void**) &original_nt_create_file );
            }
        }
    }
}

extern "C" 
{

__declspec(dllexport) void initialize( HANDLE dependencies_pipe )
{
    build_hooks_dependencies_pipe = dependencies_pipe;
    patch_iat( GetModuleHandle(NULL) );
}

}
