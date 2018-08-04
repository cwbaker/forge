//
// Executor.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "Executor.hpp"
#include "Error.hpp"
#include "BuildTool.hpp"
#include "Target.hpp"
#include "Context.hpp"
#include "Reader.hpp"
#include "Scheduler.hpp"
#include <sweet/process/Process.hpp>
#include <sweet/process/Environment.hpp>
#include <sweet/assert/assert.hpp>
#include <stdlib.h>

#if defined BUILD_OS_WINDOWS
#include <windows.h>
#endif

using std::max;
using std::find;
using std::string;
using std::vector;
using std::unique_ptr;
using namespace sweet;
using namespace sweet::process;
using namespace sweet::build_tool;

Executor::Executor( BuildTool* build_tool )
: build_tool_( build_tool ),
  jobs_mutex_(),
  jobs_empty_condition_(),
  jobs_ready_condition_(),
  jobs_(),
  build_hooks_library_(),
  maximum_parallel_jobs_( 1 ),
  threads_(),
  active_jobs_( 0 ),
  done_( false )
{
    SWEET_ASSERT( build_tool_ );
    initialize_build_hooks_windows();
}

Executor::~Executor()
{
    stop();
}

const std::string& Executor::build_hooks_library() const
{
    return build_hooks_library_;
}

int Executor::maximum_parallel_jobs() const
{
    return maximum_parallel_jobs_;
}

int Executor::active_jobs() const
{
    return active_jobs_;
}

void Executor::set_build_hooks_library( const std::string& build_hooks_library )
{
    build_hooks_library_ = build_hooks_library;
}

void Executor::set_maximum_parallel_jobs( int maximum_parallel_jobs )
{
    stop();
    maximum_parallel_jobs_ = max( 1, maximum_parallel_jobs );
}

void Executor::execute( const std::string& command, const std::string& command_line, process::Environment* environment, Filter* dependencies_filter, Filter* stdout_filter, Filter* stderr_filter, Arguments* arguments, Context* context )
{
    SWEET_ASSERT( !command.empty() );
    SWEET_ASSERT( context );

    if ( !command.empty() )
    {
        start();
        std::unique_lock<std::mutex> lock( jobs_mutex_ );
        jobs_.push_back( std::bind(&Executor::thread_execute, this, command, command_line, environment, dependencies_filter, stdout_filter, stderr_filter, arguments, context->working_directory(), context) );
        ++active_jobs_;
        jobs_ready_condition_.notify_all();
    }
}

int Executor::thread_main( void* context )
{
    Executor* executor = reinterpret_cast<Executor*>( context );
    SWEET_ASSERT( executor );
    executor->thread_process();
    return EXIT_SUCCESS;
}

void Executor::thread_process()
{
    std::unique_lock<std::mutex> lock( jobs_mutex_ );
    while ( !done_ )
    {
        if ( jobs_.empty() )
        {
            jobs_empty_condition_.notify_all();
            jobs_ready_condition_.wait( lock );
        }

        if ( !jobs_.empty() )
        {
            std::function<void()> function = jobs_.front();
            jobs_.pop_front();
            lock.unlock();
            function();
            lock.lock();
            --active_jobs_;
        }
    }
}

void Executor::thread_execute( const std::string& command, const std::string& command_line, process::Environment* environment, Filter* dependencies_filter, Filter* stdout_filter, Filter* stderr_filter, Arguments* arguments, Target* working_directory, Context* context )
{
    SWEET_ASSERT( build_tool_ );
    
    try
    {
        environment = inject_build_hooks_linux( environment, dependencies_filter != NULL );
        environment = inject_build_hooks_macosx( environment, dependencies_filter != NULL );
        if ( environment )
        {
            environment->prepare();
        }

        Process process;
        process.executable( command.c_str() );
        process.directory( working_directory->path().c_str() );
        process.environment( environment );
        process.start_suspended( true );

        intptr_t read_dependencies_pipe = dependencies_filter && !build_hooks_library_.empty() ? process.pipe( PIPE_USER_0 ) : -1;
        intptr_t write_dependencies_pipe = (intptr_t) process.write_pipe( 0 );
        intptr_t stdout_pipe = process.pipe( PIPE_STDOUT );
        intptr_t stderr_pipe = process.pipe( PIPE_STDERR );
        process.run( command_line.c_str() );
        inject_build_hooks_windows( &process, write_dependencies_pipe );
        process.resume();

        if ( dependencies_filter && !build_hooks_library_.empty() )
        {
            build_tool_->reader()->read( read_dependencies_pipe, dependencies_filter, arguments, working_directory );
        }

        build_tool_->reader()->read( stdout_pipe, stdout_filter, arguments, working_directory );
        build_tool_->reader()->read( stderr_pipe, stderr_filter, arguments, working_directory );
        process.wait();
        build_tool_->scheduler()->push_execute_finished( process.exit_code(), context, environment );
    }

    catch ( const std::exception& exception )
    {
        Scheduler* scheduler = build_tool_->scheduler();
        scheduler->push_errorf( "%s", exception.what() );
        scheduler->push_execute_finished( EXIT_FAILURE, context, environment );
    }
}

void Executor::start()
{
    SWEET_ASSERT( maximum_parallel_jobs_ > 0 );

    if ( threads_.empty() )
    {
        std::unique_lock<std::mutex> lock( jobs_mutex_ );
        done_ = false;
        threads_.reserve( maximum_parallel_jobs_ );
        for ( int i = 0; i < maximum_parallel_jobs_; ++i )
        {
            unique_ptr<std::thread> thread( new std::thread(&Executor::thread_main, this) );
            threads_.push_back( thread.release() );
        }
    }
}

void Executor::stop()
{
    if ( !threads_.empty() )
    {
        {
            std::unique_lock<std::mutex> lock( jobs_mutex_ );
            if ( !jobs_.empty() )
            {
                jobs_empty_condition_.wait( lock );
            }
            done_ = true;
            jobs_ready_condition_.notify_all();
        }

        for ( vector<std::thread*>::iterator i = threads_.begin(); i != threads_.end(); ++i )
        {
            try
            {
                std::thread* thread = *i;
                SWEET_ASSERT( thread );
                thread->join();
            }

            catch ( const std::exception& exception )
            {
                build_tool_->errorf( "Failed to join thread - %s", exception.what() );
            }
        }
        
        while ( !threads_.empty() )
        {
            delete threads_.back();
            threads_.pop_back();
        }
    }
}

process::Environment* Executor::inject_build_hooks_linux( process::Environment* environment, bool dependencies_filter_exists ) const
{
#if defined(BUILD_OS_LINUX)
    if ( !build_hooks_library_.empty() && dependencies_filter_exists )
    {
        if ( !environment )
        {
            environment = new process::Environment;
        }
        environment->append( "LD_PRELOAD", build_hooks_library_.c_str() );
    }
#else
    (void) environment;
    (void) dependencies_filter_exists;
#endif
    return environment;
}

process::Environment* Executor::inject_build_hooks_macosx( process::Environment* environment, bool dependencies_filter_exists ) const
{
#if defined(BUILD_OS_MACOS)
    if ( !build_hooks_library_.empty() && dependencies_filter_exists )
    {
        if ( !environment )
        {
            environment = new process::Environment;
        }
        environment->append( "DYLD_FORCE_FLAT_NAMESPACE", "1" );
        environment->append( "DYLD_INSERT_LIBRARIES", build_hooks_library_.c_str() );
    }
#else
    (void) environment;
    (void) dependencies_filter_exists;
#endif
    return environment;
}

void Executor::inject_build_hooks_windows( process::Process* pprocess, intptr_t write_dependencies_pipe ) const
{
#if defined(BUILD_OS_WINDOWS)
    bool inject_build_hooks =
        !build_hooks_library_.empty() &&
        write_dependencies_pipe != (intptr_t) INVALID_HANDLE_VALUE &&
        is_64_bit_process_windows( pprocess )
    ;

    if ( inject_build_hooks )
    {
        unsigned char inject_build_hooks[] = {
			0x48, 0x83, 0xEC, 0x08, // sub rsp,8
            0x48, 0x83, 0xE4, 0xF0, // and rsp, 0xfffffffffffffff0 ; align stack to 16 bytes.
            0xC7, 0x04, 0x24, 0x88, 0x77, 0x66, 0x55, // mov dword ptr [rsp],55667788h (0x04 + 3)
            0xC7, 0x44, 0x24, 0x04, 0x44, 0x33, 0x22, 0x11, // mov dword ptr [rsp+4],11223344h (0x0b + 3)
            0x9C, // pushfq
            0x41, 0x57, // push r15
            0x41, 0x56, // push r14
            0x41, 0x55, // push r13
            0x41, 0x54, // push r12
            0x41, 0x53, // push r11
            0x41, 0x52, // push r10
            0x41, 0x51, // push r9
            0x55, // push rbp
            0x57, // push rdi
            0x56, // push rsi
            0x52, // push rdx
            0x51, // push rcx
            0x53, // push rbx
            0x50, // push rax
            0x48, 0x83, 0xEC, 0x28, // sub rsp, 32 + 8 to realign the stack to a 16 byte boundary.
            0x48, 0xB9, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, // mov rcx, 1122334411223344h (0x31 + 2)
            0x48, 0xB8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, // mov rax, 1122334411223344h (0x3b + 2)
            0xFF, 0xD0, // call rax (LoadLibraryA("build_hooks.dll")).
            0x48, 0x8B, 0xC8, // mov rcx,rax
            0x48, 0xBA, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, // mov rdx,1122334455667788h (0x4a + 2)                    
            0x48, 0xB8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, // mov rax, 1122334455667788h (0x54 + 2)
            0xFF, 0xD0, // call rax (GetProcAddress(build_hooks_dll, "initialize"))
            0x48, 0xB9, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,  // mov rcx,1122334455667788h (0x60 + 2)                    
            0xFF, 0xD0, // call rax (initialize(write_dependencies_pipe))
            0x48, 0x83, 0xC4, 0x28, // add rsp, 32 + 8
            0x58, // pop rax
            0x5B, // pop rbx
            0x59, // pop rcx
            0x5A, // pop rdx
            0x5E, // pop rsi
            0x5F, // pop rdi
            0x5D, // pop rbp
            0x41, 0x59, // pop r9
            0x41, 0x5A, // pop r10
            0x41, 0x5B, // pop r11
            0x41, 0x5C, // pop r12
            0x41, 0x5D, // pop r13
            0x41, 0x5E, // pop r14
            0x41, 0x5F, // pop r15
            0x9D, // popfq
            0xC3 // ret
        };
        const char* initialize = "initialize";
        int inject_build_hooks_size = (sizeof(inject_build_hooks) + 0xf) & ~0xf;
        int build_hooks_library_size = int(build_hooks_library_.size()) + 1;
        int initialize_size = int(strlen(initialize)) + 1;
        int total_size = inject_build_hooks_size + build_hooks_library_size + initialize_size;

        HANDLE process = (HANDLE) pprocess->process();
        HANDLE thread = (HANDLE) pprocess->thread();

        CONTEXT context;
        context.ContextFlags = CONTEXT_CONTROL;
        BOOL result = ::GetThreadContext( thread, &context );
        SWEET_ASSERT( result );

        char* buffer = (char*) ::VirtualAllocEx( process, NULL, total_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
        SWEET_ASSERT( buffer );

        uintptr_t build_hooks_library_address = ((uintptr_t) buffer + inject_build_hooks_size);
        uintptr_t initialize_address = build_hooks_library_address + build_hooks_library_size;

        HMODULE kernel32 = GetModuleHandle( "kernel32.dll" );
		*((unsigned int*) &inject_build_hooks[11]) = (unsigned int) (context.Rip & 0xffffffff);
        *((unsigned int*) &inject_build_hooks[19]) = (unsigned int) (context.Rip >> 32);
        *((void**) &inject_build_hooks[51]) = (void*) build_hooks_library_address;
        *((void**) &inject_build_hooks[61]) = (void*) ::GetProcAddress( kernel32, "LoadLibraryA" );
        *((void**) &inject_build_hooks[76]) = (void*) initialize_address;
        *((void**) &inject_build_hooks[86]) = (void*) ::GetProcAddress( kernel32, "GetProcAddress" );
        *((void**) &inject_build_hooks[98]) = (void*) write_dependencies_pipe;

        char* position = buffer;
        int written = ::WriteProcessMemory( process, position, inject_build_hooks, inject_build_hooks_size, NULL );
        SWEET_ASSERT( written );
        position += inject_build_hooks_size;

        written = ::WriteProcessMemory( process, position, build_hooks_library_.c_str(), build_hooks_library_size, NULL );
        SWEET_ASSERT( written );
        position += build_hooks_library_size;

        written = ::WriteProcessMemory( process, position, initialize, initialize_size, NULL );
        SWEET_ASSERT( written );
        position += initialize_size;

        DWORD original_protection;
        result = VirtualProtectEx( process, buffer, total_size, PAGE_EXECUTE_READ, &original_protection );
        SWEET_ASSERT( result );

        result = ::FlushInstructionCache( process, buffer, total_size );
        SWEET_ASSERT( result );

        context.Rip = (uintptr_t) buffer;
        context.ContextFlags = CONTEXT_CONTROL;
        result = ::SetThreadContext( thread, &context );
        SWEET_ASSERT( result );
    }
#else
    (void) pprocess;
    (void) write_dependencies_pipe;    
#endif
}

#if defined BUILD_OS_WINDOWS 
typedef BOOL (WINAPI *IsWow64ProcessFunction)( HANDLE, PBOOL );
IsWow64ProcessFunction is_wow_64_process = nullptr;
#endif

void Executor::initialize_build_hooks_windows() const
{
#if defined BUILD_OS_WINDOWS 
    is_wow_64_process = (IsWow64ProcessFunction) GetProcAddress( GetModuleHandleA("kernel32"), "IsWow64Process" );
#endif
}

bool Executor::is_64_bit_process_windows( process::Process* process ) const
{
#if defined BUILD_OS_WINDOWS
    BOOL is_32_bit_process = TRUE;
    return 
        is_wow_64_process &&
        is_wow_64_process( (HANDLE) process->process(), &is_32_bit_process ) &&
        !is_32_bit_process
    ;
#else
    (void) process;
    return true;
#endif
}
