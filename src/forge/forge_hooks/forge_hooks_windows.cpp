
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

typedef BOOL (WINAPI *CreateProcessAFunction)( 
    LPCSTR application_name, 
    LPSTR command_line,
    LPSECURITY_ATTRIBUTES process_attributes,
    LPSECURITY_ATTRIBUTES thread_attributes,
    BOOL inherit_handles,
    DWORD creation_flags,
    LPVOID environment,
    LPCSTR current_directory,
    LPSTARTUPINFO startup_info,
    LPPROCESS_INFORMATION process_information
);

typedef BOOL (WINAPI *CreateProcessWFunction)( 
    LPCWSTR application_name, 
    LPWSTR command_line,
    LPSECURITY_ATTRIBUTES process_attributes,
    LPSECURITY_ATTRIBUTES thread_attributes,
    BOOL inherit_handles,
    DWORD creation_flags,
    LPVOID environment,
    LPCWSTR current_directory,
    LPSTARTUPINFO startup_info,
    LPPROCESS_INFORMATION process_information
);

typedef BOOL (WINAPI *CreateProcessAsUserAFunction)(
    HANDLE token,
    LPCSTR application_name,
    LPSTR command_line,
    LPSECURITY_ATTRIBUTES process_attributes,
    LPSECURITY_ATTRIBUTES thread_attributes,
    BOOL inherit_handles,
    DWORD creation_flags,
    LPVOID environment,
    LPCSTR current_directory,
    LPSTARTUPINFOA startup_info,
    LPPROCESS_INFORMATION process_information
);

typedef BOOL (WINAPI *CreateProcessAsUserWFunction)(
    HANDLE token,
    LPCWSTR application_name,
    LPWSTR command_line,
    LPSECURITY_ATTRIBUTES process_attributes,
    LPSECURITY_ATTRIBUTES thread_attributes,
    BOOL inherit_handles,
    DWORD creation_flags,
    LPVOID environment,
    LPCWSTR current_directory,
    LPSTARTUPINFOW startup_info,
    LPPROCESS_INFORMATION process_information
);

typedef BOOL (WINAPI *CreateProcessWithLogonWFunction)(
    LPCWSTR username,
    LPCWSTR domain,
    LPCWSTR password,
    DWORD logon_flags,
    LPCWSTR application_name,
    LPWSTR command_line,
    DWORD creation_flags,
    LPVOID environment,
    LPCWSTR current_directory,
    LPSTARTUPINFOW startup_info,
    LPPROCESS_INFORMATION process_information
);

typedef BOOL (WINAPI *CreateProcessWithTokenWFunction)(
    HANDLE token,
    DWORD logon_flags,
    LPCWSTR application_name,
    LPWSTR command_line,
    DWORD creation_flags,
    LPVOID environment,
    LPCWSTR current_directory,
    LPSTARTUPINFOW startup_info,
    LPPROCESS_INFORMATION process_information
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

typedef NTSTATUS (WINAPI *NtCreateUserProcessFunction)(
    PHANDLE process_handle,
    PHANDLE thread_handle,
    ACCESS_MASK process_desired_access,
    ACCESS_MASK thread_desired_access,
    POBJECT_ATTRIBUTES process_object_attributes,
    POBJECT_ATTRIBUTES thread_object_attributes,
    ULONG process_flags,
    ULONG thread_flags,
    PRTL_USER_PROCESS_PARAMETERS process_parameters,
    ULONG_PTR create_info,
    ULONG_PTR attribute_list
);

typedef HMODULE (WINAPI *LoadLibraryAFunction)(
    LPCSTR filename
);

typedef HMODULE (WINAPI *LoadLibraryWFunction)(
    LPCWSTR filename
);

typedef HMODULE (WINAPI *LoadLibraryExAFunction)(
    LPCSTR filename,
    HANDLE file,
    DWORD flags
);

typedef HMODULE (WINAPI *LoadLibraryExWFunction)(
    LPCWSTR filename,
    HANDLE file,
    DWORD flags
);

static HANDLE build_hooks_dependencies_pipe = INVALID_HANDLE_VALUE;
static CreateFileAFunction original_create_file_a = NULL;
static CreateFileWFunction original_create_file_w = NULL;
static CreateFileTransactedAFunction original_create_file_transacted_a = NULL;
static CreateFileTransactedWFunction original_create_file_transacted_w = NULL;
static CreateProcessAFunction original_create_process_a = NULL;
static CreateProcessWFunction original_create_process_w = NULL;
static CreateProcessAsUserAFunction original_create_process_as_user_a = NULL;
static CreateProcessAsUserWFunction original_create_process_as_user_w = NULL;
static CreateProcessWithLogonWFunction original_create_process_with_logon_w = NULL;
static CreateProcessWithTokenWFunction original_create_process_with_token_w = NULL;
static NtOpenFileFunction original_nt_open_file = NULL;
static NtCreateFileFunction original_nt_create_file = NULL;
static NtCreateUserProcessFunction original_nt_create_user_process = NULL;
static LoadLibraryAFunction original_load_library_a = NULL;
static LoadLibraryWFunction original_load_library_w = NULL;
static LoadLibraryExAFunction original_load_library_ex_a = NULL;
static LoadLibraryExWFunction original_load_library_ex_w = NULL;

// Skip the "\\?\" or "\??\" prefixes used to disable string parsing in
// Windows APIs.  Forge scripts that handle these are expecting paths
// without these prefixes.
// See https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file#win32-file-namespaces.
static const char* skip_win32_file_namespace( const char* filename )
{
    if ( filename[0] == '\\' && (filename[1] == '\\' || filename[1] == '?') && filename[2] == '?' && filename[3] == '\\' )
    {
        return filename + 4;
    }
    return filename;
}

static void log_file_access( const char* namespaced_path, int write )
{
    DWORD bytes_written = 0;
    HANDLE pipe = build_hooks_dependencies_pipe;
    const char* access = write ? "== write '" : "== read '";
    const char* path = skip_win32_file_namespace( namespaced_path );
    WriteFile( pipe, access, (DWORD) strlen(access), &bytes_written, NULL );
    WriteFile( pipe, path, (DWORD) strlen(path), &bytes_written, NULL );
    WriteFile( pipe, "'\n", 2, &bytes_written, NULL );
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
    if ( handle != INVALID_HANDLE_VALUE )
    {
        log_file_access( filename, desired_access & GENERIC_WRITE );
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
    if ( handle != INVALID_HANDLE_VALUE )
    {
        char filename [MAX_PATH + 1];
        int count = WideCharToMultiByte( CP_UTF8, 0, wide_filename, (int) wcslen(wide_filename), filename, (int) sizeof(filename), NULL, NULL );
        filename[count] = 0;
        log_file_access( filename, desired_access & GENERIC_WRITE );
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
    if ( handle != INVALID_HANDLE_VALUE )
    {
        log_file_access( filename, desired_access & GENERIC_WRITE );
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
    if ( handle != INVALID_HANDLE_VALUE )
    {
        char filename [MAX_PATH + 1];
        int count = WideCharToMultiByte( CP_UTF8, 0, wide_filename, (int) wcslen(wide_filename), filename, (int) sizeof(filename), NULL, NULL );
        filename[count] = 0;
        log_file_access( filename, desired_access & GENERIC_WRITE );
    }
    return handle;
}

static BOOL WINAPI create_process_a_hook( LPCSTR application_name, LPSTR command_line, LPSECURITY_ATTRIBUTES process_attributes, LPSECURITY_ATTRIBUTES thread_attributes, BOOL inherit_handles, DWORD creation_flags, LPVOID environment, LPCSTR current_directory, LPSTARTUPINFO startup_info, LPPROCESS_INFORMATION process_information )
{
    BOOL success = (*original_create_process_a)( 
        application_name, 
        command_line, 
        process_attributes, 
        thread_attributes, 
        inherit_handles, 
        creation_flags,
        environment,
        current_directory,
        startup_info,
        process_information 
    );
    return success;
}

static BOOL WINAPI create_process_w_hook( LPCWSTR application_name, LPWSTR command_line, LPSECURITY_ATTRIBUTES process_attributes, LPSECURITY_ATTRIBUTES thread_attributes, BOOL inherit_handles, DWORD creation_flags, LPVOID environment, LPCWSTR current_directory, LPSTARTUPINFO startup_info, LPPROCESS_INFORMATION process_information )
{
    BOOL success = (*original_create_process_w)( 
        application_name, 
        command_line, 
        process_attributes, 
        thread_attributes, 
        inherit_handles, 
        creation_flags,
        environment,
        current_directory,
        startup_info,
        process_information 
    );
    char filename [MAX_PATH + 1];
    int count = WideCharToMultiByte( CP_UTF8, 0, application_name, (int) wcslen(application_name), filename, (int) sizeof(filename), NULL, NULL );
    filename[count] = 0;
    return success;
}

static BOOL WINAPI create_process_as_user_a_hook( HANDLE token, LPCSTR application_name, LPSTR command_line, LPSECURITY_ATTRIBUTES process_attributes, LPSECURITY_ATTRIBUTES thread_attributes, BOOL inherit_handles, DWORD creation_flags, LPVOID environment, LPCSTR current_directory, LPSTARTUPINFOA startup_info, LPPROCESS_INFORMATION process_information )
{
    BOOL success = (*original_create_process_as_user_a)(
        token,
        application_name,
        command_line,
        process_attributes,
        thread_attributes,
        inherit_handles,
        creation_flags,
        environment,
        current_directory,
        startup_info,
        process_information
    );
    return success;
}

static BOOL WINAPI create_process_as_user_w_hook( HANDLE token, LPCWSTR application_name, LPWSTR command_line, LPSECURITY_ATTRIBUTES process_attributes, LPSECURITY_ATTRIBUTES thread_attributes, BOOL inherit_handles, DWORD creation_flags, LPVOID environment, LPCWSTR current_directory, LPSTARTUPINFOW startup_info, LPPROCESS_INFORMATION process_information )
{
    BOOL success = (*original_create_process_as_user_w)(
        token,
        application_name,
        command_line,
        process_attributes,
        thread_attributes,
        inherit_handles,
        creation_flags,
        environment,
        current_directory,
        startup_info,
        process_information
    );
    char filename [MAX_PATH + 1];
    int count = WideCharToMultiByte( CP_UTF8, 0, application_name, (int) wcslen(application_name), filename, (int) sizeof(filename), NULL, NULL );
    filename[count] = 0;
    return success;
}

static BOOL WINAPI create_process_with_logon_w_hook( LPCWSTR username, LPCWSTR domain, LPCWSTR password, DWORD logon_flags, LPCWSTR application_name, LPWSTR command_line, DWORD creation_flags, LPVOID environment, LPCWSTR current_directory, LPSTARTUPINFOW startup_info, LPPROCESS_INFORMATION process_information )
{
    BOOL success = (*original_create_process_with_logon_w)(
        username,
        domain,
        password,
        logon_flags,
        application_name,
        command_line,
        creation_flags,
        environment,
        current_directory,
        startup_info,
        process_information        
    );
    char filename [MAX_PATH + 1];
    int count = WideCharToMultiByte( CP_UTF8, 0, application_name, (int) wcslen(application_name), filename, (int) sizeof(filename), NULL, NULL );
    filename[count] = 0;
    return success;
}

static BOOL WINAPI create_process_with_token_w_hook( HANDLE token, DWORD logon_flags, LPCWSTR application_name, LPWSTR command_line, DWORD creation_flags, LPVOID environment, LPCWSTR current_directory, LPSTARTUPINFOW startup_info, LPPROCESS_INFORMATION process_information )
{
    BOOL success = (*original_create_process_with_token_w)(
        token, 
        logon_flags, 
        application_name, 
        command_line, 
        creation_flags, 
        environment, 
        current_directory, 
        startup_info, 
        process_information    
    );
    char filename [MAX_PATH + 1];
    int count = WideCharToMultiByte( CP_UTF8, 0, application_name, (int) wcslen(application_name), filename, (int) sizeof(filename), NULL, NULL );
    filename[count] = 0;
    return success;
}

static NTSTATUS WINAPI nt_open_file_hook( PHANDLE file_handle, ACCESS_MASK desired_access, POBJECT_ATTRIBUTES object_attributes, PIO_STATUS_BLOCK io_status_block, ULONG share_access, ULONG open_options )
{
    NTSTATUS status = (*original_nt_open_file)(        
        file_handle,
        desired_access,
        object_attributes,
        io_status_block,
        share_access,
        open_options
    );
    return status;
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
        log_file_access( path, desired_access & (FILE_WRITE_DATA | FILE_APPEND_DATA) );
    }
    return status;
}

static NTSTATUS WINAPI nt_create_user_process_hook( PHANDLE process_handle, PHANDLE thread_handle, ACCESS_MASK process_desired_access, ACCESS_MASK thread_desired_access, POBJECT_ATTRIBUTES process_object_attributes, POBJECT_ATTRIBUTES thread_object_attributes, ULONG process_flags, ULONG thread_flags, PRTL_USER_PROCESS_PARAMETERS process_parameters, ULONG_PTR create_info, ULONG_PTR attribute_list )
{
    NTSTATUS status = (*original_nt_create_user_process)(
        process_handle,
        thread_handle,
        process_desired_access,
        thread_desired_access,
        process_object_attributes,
        thread_object_attributes,
        process_flags,
        thread_flags,
        process_parameters,
        create_info,
        attribute_list
    );
    return status;
}

static HMODULE WINAPI load_library_a_hook( LPCSTR filename )
{
    HMODULE module = (*original_load_library_a)( 
        filename
    );
    // patch_iat( module );
    return module;
}

static HMODULE WINAPI load_library_w_hook( LPCWSTR wide_filename )
{
    HMODULE module = (*original_load_library_w)( 
        wide_filename
    );
    char filename [MAX_PATH + 1];
    int count = WideCharToMultiByte( CP_UTF8, 0, wide_filename, (int) wcslen(wide_filename), filename, (int) sizeof(filename), NULL, NULL );
    filename[count] = 0;
    // patch_iat( module );
    return module;
}

static HMODULE WINAPI load_library_ex_a_hook( LPCSTR filename, HANDLE file, DWORD flags )
{
    HMODULE module = (*original_load_library_ex_a)( 
        filename,
        file,
        flags
    );
    // patch_iat( module );
    return module;
}

static HMODULE WINAPI load_library_ex_w_hook( LPCWSTR wide_filename, HANDLE file, DWORD flags )
{
    HMODULE module = (*original_load_library_ex_w)( 
        wide_filename,
        file,
        flags
    );
    char filename [MAX_PATH + 1];
    int count = WideCharToMultiByte( CP_UTF8, 0, wide_filename, (int) wcslen(wide_filename), filename, (int) sizeof(filename), NULL, NULL );
    filename[count] = 0;
    // patch_iat( module );
    return module;
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
                kernel32.patch_function( "CreateProcessA", (void*) &create_process_a_hook, (void**) &original_create_process_a );
                kernel32.patch_function( "CreateProcessW", (void*) &create_process_w_hook, (void**) &original_create_process_w );
                kernel32.patch_function( "CreateProcessAsUserA", (void*) &create_process_as_user_a_hook, (void**) &original_create_process_as_user_a );
                kernel32.patch_function( "CreateProcessAsUserW", (void*) &create_process_as_user_w_hook, (void**) &original_create_process_as_user_w );
                kernel32.patch_function( "CreateProcessWithLogonW", (void*) &create_process_with_logon_w_hook, (void**) &original_create_process_with_logon_w );
                kernel32.patch_function( "CreateProcessWithTokenW", (void*) &create_process_with_token_w_hook, (void**) &original_create_process_with_token_w );
                kernel32.patch_function( "LoadLibraryA", (void*) &load_library_a_hook, (void**) &original_load_library_a );
                kernel32.patch_function( "LoadLibraryW", (void*) &load_library_w_hook, (void**) &original_load_library_w );
                kernel32.patch_function( "LoadLibraryExA", (void*) &load_library_ex_a_hook, (void**) &original_load_library_ex_a );
                kernel32.patch_function( "LoadLibraryExW", (void*) &load_library_ex_w_hook, (void**) &original_load_library_ex_w );
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
