//
// ScriptInterface.cpp
// Copyright (c) Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "ScriptInterface.hpp"
#include "Error.hpp"
#include "TargetPrototype.hpp"
#include "Target.hpp"
#include "Arguments.hpp"
#include "Graph.hpp"
#include "BuildTool.hpp"
#include "Context.hpp"
#include "Scheduler.hpp"
#include "OsInterface.hpp"
#include <sweet/lua/lua_functions.ipp>
#include <sweet/lua/filesystem.hpp>
#include <sweet/lua/vector.hpp>
#include <sweet/lua/Lua.hpp>
#include <sweet/process/Environment.hpp>
#include <memory>
#include <stdlib.h>

using std::string;
using std::vector;
using std::unique_ptr;
using namespace sweet;
using namespace sweet::lua;
using namespace sweet::build_tool;

namespace sweet
{

namespace build_tool
{

template <class Type>
struct Deleter
{
    typedef void (ScriptInterface::* DestroyFunction)( Type* object );    
    DestroyFunction destroy_function_;
    ScriptInterface* script_interface_;
    
    public:
        Deleter( DestroyFunction destroy_function, ScriptInterface* script_interface )
        : destroy_function_( destroy_function ),
          script_interface_( script_interface )
        {
            SWEET_ASSERT( destroy_function_ );
            SWEET_ASSERT( script_interface_ );
        }
        
        void operator()( Type* object ) const
        {
            SWEET_ASSERT( destroy_function_ );
            SWEET_ASSERT( script_interface_ );
            SWEET_ASSERT( object );       
            (script_interface_->*destroy_function_)( object );
            delete object;
        }
};

}

}

ScriptInterface::ScriptInterface( OsInterface* os_interface, BuildTool* build_tool )
: os_interface_( os_interface ),
  build_tool_( build_tool ),
  lua_( build_tool->error_policy() ),
  target_prototype_metatable_( lua_ ),
  target_prototype_prototype_( lua_ ),
  target_metatable_( lua_ ),
  target_prototype_( lua_ ),
  target_prototypes_(),
  root_directory_(),
  initial_directory_(),
  executable_directory_()
{
    SWEET_ASSERT( os_interface_ );
    SWEET_ASSERT( build_tool_ );

    target_prototype_metatable_.members()
        ( "__index", target_prototype_prototype_ )
    ;

    target_prototype_prototype_.members()
        .type( SWEET_STATIC_TYPEID(TargetPrototype) )
        ( "id", &TargetPrototype::id )
    ;

    target_metatable_.members()
        ( "__index", target_prototype_ )
    ;

    target_prototype_.members()
        .type( SWEET_STATIC_TYPEID(Target) )
        ( "id", &Target::id )
        ( "path", &Target::path )
        ( "branch", &Target::branch )
        ( "directory", &Target::branch )
        ( "parent", &ScriptInterface::parent, this, _1 )
        ( "prototype", &Target::prototype )
        ( "set_required_to_exist", &Target::set_required_to_exist )
        ( "required_to_exist", &Target::required_to_exist )
        ( "set_always_bind", &Target::set_always_bind )
        ( "always_bind", &Target::always_bind )
        ( "set_cleanable", &Target::set_cleanable )
        ( "cleanable", &Target::cleanable )
        ( "timestamp", &Target::timestamp )
        ( "last_write_time", &Target::last_write_time )
        ( "outdated", &Target::outdated )
        ( "set_filename", raw(&ScriptInterface::set_filename) )
        ( "filename", raw(&ScriptInterface::filename) )
        ( "filenames", &Target::filenames )
        ( "set_working_directory", &Target::set_working_directory )
        ( "working_directory", &ScriptInterface::target_working_directory, this, _1 )
        ( "targets", raw(&ScriptInterface::targets), this )
        ( "add_dependency", &Target::add_explicit_dependency )
        ( "remove_dependency", &Target::remove_dependency )
        ( "add_implicit_dependency", &Target::add_implicit_dependency )
        ( "clear_implicit_dependencies", &Target::clear_implicit_dependencies )
        ( "dependency", raw(&ScriptInterface::dependency), this )
        ( "dependencies", raw(&ScriptInterface::dependencies), this )
    ;

    lua_.globals()
        ( "set_maximum_parallel_jobs", &BuildTool::set_maximum_parallel_jobs, build_tool_ )
        ( "maximum_parallel_jobs", &BuildTool::maximum_parallel_jobs, build_tool_ )
        ( "set_stack_trace_enabled", &BuildTool::set_stack_trace_enabled, build_tool_ )
        ( "stack_trace_enabled", &BuildTool::stack_trace_enabled, build_tool_ )
        ( "set_build_hooks_library", &BuildTool::set_build_hooks_library, build_tool_ )
        ( "build_hooks_library", &BuildTool::build_hooks_library, build_tool_ )
    ;

    lua_.globals()
        ( "TargetPrototype", raw(&ScriptInterface::target_prototype__), this )
        ( "file", raw(&ScriptInterface::file), this )
        ( "target", raw(&ScriptInterface::target), this )
        ( "find_target", &ScriptInterface::find_target, this )
        ( "absolute", raw(&ScriptInterface::absolute_), this )
        ( "relative", raw(&ScriptInterface::relative_), this )
        ( "root", raw(&ScriptInterface::root_), this )
        ( "initial", raw(&ScriptInterface::initial_), this )
        ( "home", raw(&ScriptInterface::home_), this )
        ( "executable", raw(&ScriptInterface::executable_), this )
        ( "anonymous", &ScriptInterface::anonymous, this )
        ( "is_absolute", &ScriptInterface::is_absolute, this )
        ( "is_relative", &ScriptInterface::is_relative, this )
        ( "cd", &ScriptInterface::cd, this )
        ( "pushd", &ScriptInterface::pushd, this )
        ( "popd", &ScriptInterface::popd, this )
        ( "pwd", &ScriptInterface::pwd, this )
        ( "working_directory", &ScriptInterface::working_directory, this )
        ( "lower", &ScriptInterface::lower, this )
        ( "upper", &ScriptInterface::upper, this )
        ( "native", &ScriptInterface::native, this )
        ( "branch", &ScriptInterface::branch, this )
        ( "leaf", &ScriptInterface::leaf, this )
        ( "basename", &ScriptInterface::basename, this )
        ( "extension", &ScriptInterface::extension, this )
        ( "exec", raw(&ScriptInterface::execute), this )
        ( "execute", raw(&ScriptInterface::execute), this )
        ( "print", &ScriptInterface::print, this )
        ( "operating_system", &ScriptInterface::operating_system, this )
        ( "exists", &ScriptInterface::exists, this )
        ( "is_file", &ScriptInterface::is_file, this )
        ( "is_directory", &ScriptInterface::is_directory, this )
        ( "ls", &ScriptInterface::ls, this )
        ( "find", &ScriptInterface::find, this )
        ( "mkdir", &ScriptInterface::mkdir, this )
        ( "cpdir", &ScriptInterface::cpdir, this )
        ( "rmdir", &ScriptInterface::rmdir, this )
        ( "cp", &ScriptInterface::cp, this )
        ( "rm", &ScriptInterface::rm, this )
        ( "getenv", raw(&ScriptInterface::getenv_), this )
        ( "sleep", &ScriptInterface::sleep, this )
        ( "ticks", &ScriptInterface::ticks, this )
        ( "buildfile", &ScriptInterface::buildfile, this )
        ( "postorder", raw(&ScriptInterface::postorder), this )
        ( "print_dependencies", &ScriptInterface::print_dependencies, this )
        ( "print_namespace", &ScriptInterface::print_namespace, this )
        ( "wait", &ScriptInterface::wait, this )
        ( "clear", &ScriptInterface::clear, this )
        ( "load_xml", &ScriptInterface::load_xml, this )
        ( "save_xml", &ScriptInterface::save_xml, this )
        ( "load_binary", &ScriptInterface::load_binary, this )
        ( "save_binary", &ScriptInterface::save_binary, this )
    ;
}

ScriptInterface::~ScriptInterface()
{
    while ( !target_prototypes_.empty() )
    {
        delete target_prototypes_.back();
        target_prototypes_.pop_back();
    }
}

lua::Lua& ScriptInterface::lua()
{
    return lua_;
}

Context* ScriptInterface::context() const
{
    return build_tool_->scheduler()->context();
}

void ScriptInterface::set_root_directory( const fs::Path& root_directory )
{
    SWEET_ASSERT( root_directory.is_absolute() );
    root_directory_ = root_directory;
}

const fs::Path& ScriptInterface::root_directory() const
{
    return root_directory_;
}

void ScriptInterface::set_initial_directory( const fs::Path& initial_directory )
{
    SWEET_ASSERT( initial_directory.is_absolute() );
    initial_directory_ = initial_directory;
}

const fs::Path& ScriptInterface::initial_directory() const
{
    return initial_directory_;
}

void ScriptInterface::set_executable_directory( const fs::Path& executable_directory )
{
    executable_directory_ = executable_directory;
}

const fs::Path& ScriptInterface::executable_directory() const
{
    return executable_directory_;
}

void ScriptInterface::create_prototype( TargetPrototype* target_prototype )
{
    SWEET_ASSERT( target_prototype );
    
    lua_.members( target_prototype )
        .type( SWEET_STATIC_TYPEID(TargetPrototype) )
        .metatable( target_prototype_metatable_ )
        .this_pointer( target_prototype )
        ( lua::PTR_KEYWORD, value(target_prototype) )
        ( "id", &Target::id )
        ( "path", &Target::path )
        ( "branch", &Target::branch )
        ( "directory", &Target::branch )
        ( "parent", &ScriptInterface::parent, this, _1 )
        ( "prototype", &Target::prototype )
        ( "set_required_to_exist", &Target::set_required_to_exist )
        ( "required_to_exist", &Target::required_to_exist )
        ( "set_always_bind", &Target::set_always_bind )
        ( "always_bind", &Target::always_bind )
        ( "set_cleanable", &Target::set_cleanable )
        ( "cleanable", &Target::cleanable )
        ( "timestamp", &Target::timestamp )
        ( "last_write_time", &Target::last_write_time )
        ( "outdated", &Target::outdated )
        ( "set_filename", raw(&ScriptInterface::set_filename) )
        ( "filename", raw(&ScriptInterface::filename) )
        ( "filenames", &Target::filenames )
        ( "set_working_directory", &Target::set_working_directory )
        ( "working_directory", &ScriptInterface::target_working_directory, this, _1 )
        ( "targets", raw(&ScriptInterface::targets), this )
        ( "add_dependency", &Target::add_explicit_dependency )
        ( "remove_dependency", &Target::remove_dependency )
        ( "add_implicit_dependency", &Target::add_implicit_dependency )
        ( "clear_implicit_dependencies", &Target::clear_implicit_dependencies )
        ( "dependency", raw(&ScriptInterface::dependency), this )
        ( "dependencies", raw(&ScriptInterface::dependencies), this )
    ;
    
    lua_.members( target_prototype )
        ( "__index", target_prototype )
    ;
}

void ScriptInterface::destroy_prototype( TargetPrototype* target_prototype )
{
    SWEET_ASSERT( target_prototype );
    lua_.destroy( target_prototype );
}

void ScriptInterface::create_target( Target* target )
{
    SWEET_ASSERT( target );

    if ( !target->referenced_by_script() )
    {
        lua_.create( target );
        target->set_referenced_by_script( true );
        recover_target( target );
    }
}

void ScriptInterface::recover_target( Target* target )
{
    SWEET_ASSERT( target );
    lua_.members( target )
        .type( SWEET_STATIC_TYPEID(Target) )
        .metatable( target_metatable_ )
        .this_pointer( target )
        ( lua::PTR_KEYWORD, value(target) )
    ;
}

void ScriptInterface::update_target( Target* target, TargetPrototype* target_prototype )
{
    SWEET_ASSERT( target );
    if ( target_prototype )
    {
        lua_.members( target )
            .metatable( target_prototype )
        ;        
    }
    else
    {
        lua_.members( target )
            .metatable( target_metatable_ )
        ;
    }
}

void ScriptInterface::destroy_target( Target* target )
{
    SWEET_ASSERT( target );
    lua_.destroy( target );
    target->set_referenced_by_script( false );
}

TargetPrototype* ScriptInterface::target_prototype( const std::string& id )
{   
    SWEET_ASSERT( !id.empty() );

    vector<TargetPrototype* >::const_iterator i = target_prototypes_.begin(); 
    while ( i != target_prototypes_.end() && (*i)->id() != id )
    {
        ++i;
    }

    TargetPrototype* target_prototype = NULL;
    if ( i == target_prototypes_.end() )
    {
        unique_ptr<TargetPrototype> new_target_prototype( new TargetPrototype(id, build_tool_) );
        target_prototype = new_target_prototype.get();
        target_prototypes_.push_back( new_target_prototype.release() );
    }
    else
    {
        target_prototype = *i;
    }    
    return target_prototype;
}

Target* ScriptInterface::find_target( const std::string& id )
{
    Context* context = ScriptInterface::context();
    SWEET_ASSERT( context );
    Target* target = build_tool_->graph()->find_target( id, context->working_directory() );
    if ( target && !target->referenced_by_script() )
    {
        create_target( target );
    }
    return target;
}

std::string ScriptInterface::absolute( const std::string& path, const fs::Path& working_directory )
{
    if ( fs::Path(path).is_absolute() )
    {
        return path;
    }
    else if ( working_directory.empty() )
    {
        Context* context = ScriptInterface::context();
        SWEET_ASSERT( context );
        fs::Path absolute_path( context->directory() );
        absolute_path /= path;
        absolute_path.normalize();
        return absolute_path.string();
    }
    else
    {
        fs::Path absolute_path( working_directory );
        absolute_path /= path;
        absolute_path.normalize();
        return absolute_path.string();
    }
}

std::string ScriptInterface::relative( const std::string& path, const fs::Path& working_directory )
{
    if ( working_directory.empty() )
    {
        Context* context = ScriptInterface::context();
        SWEET_ASSERT( context );
        return context->directory().relative( fs::Path(path) ).string();
    }
    else
    {
        return working_directory.relative( fs::Path(path) ).string();
    }
}

std::string ScriptInterface::root( const std::string& path ) const
{
    if ( fs::Path(path).is_absolute() )
    {
        return path;
    }

    fs::Path absolute_path( root_directory_ );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path.string();
}

std::string ScriptInterface::initial( const std::string& path ) const
{
    if ( fs::Path(path).is_absolute() )
    {
        return path;
    }

    fs::Path absolute_path( initial_directory_ );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path.string();
}

std::string ScriptInterface::home( const std::string& path ) const
{
#if defined (BUILD_OS_WINDOWS)
    const char* HOME = "USERPROFILE";
#elif defined (BUILD_OS_MACOSX)
    const char* HOME = "HOME";
#else
#error "ScriptInterface::home() is not implemented for this platform"
#endif
    
    const char* home = ::getenv( HOME );
    if ( !home )
    {
        SWEET_ERROR( EnvironmentVariableNotFoundError("The environment variable '%s' could not be found", HOME) );
    }        
    
    if ( fs::Path(path).is_absolute() )
    {
        return path;
    }

    fs::Path absolute_path( home );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path.string();
}

std::string ScriptInterface::executable( const std::string& path ) const
{
    if ( fs::Path(path).is_absolute() )
    {
        return path;
    }
    
    fs::Path absolute_path( executable_directory_ );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path.string();
}

std::string ScriptInterface::anonymous() const
{
    Context* context = ScriptInterface::context();
    SWEET_ASSERT( context );
    Target* working_directory = context->working_directory();
    SWEET_ASSERT( working_directory );
    char anonymous [256];
    unsigned int length = sprintf( anonymous, "$$%d", working_directory->next_anonymous_index() );
    return string( anonymous, length );
}

bool ScriptInterface::is_absolute( const std::string& path )
{
    return fs::Path(path).is_absolute();
}

bool ScriptInterface::is_relative( const std::string& path )
{
    return fs::Path(path).is_relative();
}

void ScriptInterface::cd( const std::string& path )
{
    Context* context = ScriptInterface::context();
    SWEET_ASSERT( context );
    context->change_directory( path );
}

void ScriptInterface::pushd( const std::string& path )
{
    Context* context = ScriptInterface::context();
    SWEET_ASSERT( context );
    context->push_directory( path );
}

void ScriptInterface::popd()
{
    Context* context = ScriptInterface::context();
    SWEET_ASSERT( context );
    context->pop_directory();
}

const std::string& ScriptInterface::pwd() const
{
    Context* context = ScriptInterface::context();
    SWEET_ASSERT( context );
    return context->directory().string();
}

Target* ScriptInterface::working_directory()
{
    Context* context = ScriptInterface::context();
    SWEET_ASSERT( context );
    
    Target* target = context->working_directory();
    if ( target && !target->referenced_by_script() )
    {
        create_target( target );
    }
    return target;
}

std::string ScriptInterface::lower( const std::string& value )
{
    std::string lower_case_value;
    lower_case_value.reserve( value.length() );
    for ( std::string::const_iterator i = value.begin(); i != value.end(); ++i )
    {
        lower_case_value.push_back( tolower(*i) );
    }
    return lower_case_value;
}

std::string ScriptInterface::upper( const std::string& value )
{
    std::string upper_case_value;
    upper_case_value.reserve( value.length() );
    for ( std::string::const_iterator i = value.begin(); i != value.end(); ++i )
    {
        upper_case_value.push_back( toupper(*i) );
    }
    return upper_case_value;
}

std::string ScriptInterface::native( const std::string& path )
{
    boost::filesystem::path native_path( path );
	native_path.make_preferred();
    return native_path.string();
}

std::string ScriptInterface::branch( const std::string& path ) 
{
    return boost::filesystem::path( path ).branch_path().string();
}

std::string ScriptInterface::leaf( const std::string& path )
{
    return fs::Path( path ).leaf();
}

std::string ScriptInterface::basename( const std::string& path )
{
    return boost::filesystem::basename( path );
}

std::string ScriptInterface::extension( const std::string& path )
{
    return boost::filesystem::extension( path );
}

void ScriptInterface::print( const std::string& text )
{
    SWEET_ASSERT( build_tool_ );
    build_tool_->output( text.c_str() );
}

bool ScriptInterface::exists( const std::string& path )
{
    return os_interface_->exists( absolute(path) );
}

bool ScriptInterface::is_file( const std::string& path )
{
    return os_interface_->is_file( absolute(path) );
}

bool ScriptInterface::is_directory( const std::string& path )
{
    return os_interface_->is_directory( absolute(path) );
}

boost::filesystem::directory_iterator ScriptInterface::ls( const std::string& path )
{
    return os_interface_->ls( absolute(path) );
}

boost::filesystem::recursive_directory_iterator ScriptInterface::find( const std::string& path )
{
    return os_interface_->find( absolute(path) );
}

void ScriptInterface::mkdir( const std::string& path )
{
    return os_interface_->mkdir( absolute(path) );
}

void ScriptInterface::cpdir( const std::string& from, const std::string& to )
{
    return os_interface_->cpdir( absolute(from), absolute(to) );
}

void ScriptInterface::rmdir( const std::string& path )
{
    return os_interface_->rmdir( absolute(path) );
}

void ScriptInterface::cp( const std::string& from, const std::string& to )
{
    return os_interface_->cp( absolute(from), absolute(to) );
}

void ScriptInterface::rm( const std::string& path )
{
    return os_interface_->rm( absolute(path) );
}

std::string ScriptInterface::operating_system()
{
    return os_interface_->operating_system();
}

const char* ScriptInterface::getenv( const char* name )
{
    return os_interface_->getenv( name );
}

void ScriptInterface::sleep( float milliseconds )
{
    os_interface_->sleep( milliseconds );
}

float ScriptInterface::ticks()
{
    return os_interface_->ticks();
}

void ScriptInterface::buildfile( const std::string& filename )
{
    SWEET_ASSERT( build_tool_ );
    return build_tool_->graph()->buildfile( filename );
}

void ScriptInterface::print_dependencies( Target* target )
{
    SWEET_ASSERT( build_tool_ );
    return build_tool_->graph()->print_dependencies( target, context()->directory().string() );
}

void ScriptInterface::print_namespace( Target* target )
{
    SWEET_ASSERT( build_tool_ );
    return build_tool_->graph()->print_namespace( target );
}

void ScriptInterface::wait()
{
    SWEET_ASSERT( build_tool_ );
    build_tool_->scheduler()->wait();
}

void ScriptInterface::clear()
{
    Context* context = ScriptInterface::context();
    string working_directory = context->working_directory()->path();
    Graph* graph = build_tool_->graph();
    graph->clear();
    context->reset_directory( working_directory );
}

Target* ScriptInterface::load_xml( const std::string& filename )
{
    SWEET_ASSERT( build_tool_ );
    SWEET_ASSERT( build_tool_->graph() );

    Context* context = ScriptInterface::context();
    string working_directory = context->working_directory()->path();
    Target* cache_target = build_tool_->graph()->load_xml( absolute(filename) );
    context->reset_directory( working_directory );
    if ( cache_target )
    {
        create_target( cache_target );
    }
    return cache_target;
}

void ScriptInterface::save_xml()
{
    SWEET_ASSERT( build_tool_ );
    build_tool_->graph()->save_xml();
}

Target* ScriptInterface::load_binary( const std::string& filename )
{
    SWEET_ASSERT( build_tool_ );
    SWEET_ASSERT( build_tool_->graph() );
        
    Context* context = ScriptInterface::context();
    string working_directory = context->working_directory()->path();
    Target* cache_target = build_tool_->graph()->load_binary( absolute(filename) );
    context->reset_directory( working_directory );
    if ( cache_target )
    {
        create_target( cache_target );
    }
    return cache_target;
}

void ScriptInterface::save_binary()
{
    SWEET_ASSERT( build_tool_ );
    build_tool_->graph()->save_binary();
}

Target* ScriptInterface::parent( Target* target )
{
    SWEET_ASSERT( target );
    
    Target* parent = NULL;
    if ( target )
    {
        parent = target->parent();
        if ( parent && !parent->referenced_by_script() )
        {
            create_target( parent );
        }
    }
    return parent;
}

Target* ScriptInterface::target_working_directory( Target* target )
{
    SWEET_ASSERT( target );
    
    Target* working_directory = NULL;
    if ( target )
    {
        working_directory = target->working_directory();
        if ( !working_directory->referenced_by_script() )
        {
            create_target( working_directory );
        }
    }
    return working_directory;
}

Target* ScriptInterface::add_target( lua_State* lua_state )
{
    Context* context = ScriptInterface::context();
    SWEET_ASSERT( context );

    Graph* graph = build_tool_->graph();
    SWEET_ASSERT( graph );

    Target* working_directory = context->working_directory();
    SWEET_ASSERT( working_directory );

    const int ID_PARAMETER = 1;
    bool anonymous = false;
    string id = LuaConverter<string>::to( lua_state, ID_PARAMETER );
    if ( id.empty() )
    {
        anonymous = true;
        id = ScriptInterface::anonymous();
    }

    const int PROTOTYPE_PARAMETER = 2;
    TargetPrototype* target_prototype = LuaConverter<TargetPrototype* >::to( lua_state, PROTOTYPE_PARAMETER );

    Target* target = graph->target( id, target_prototype, working_directory );
    if ( !target->referenced_by_script() )
    {
        const int TABLE_PARAMETER = 3;
        if ( !lua_isnoneornil(lua_state, TABLE_PARAMETER) )
        {        
            if ( !lua_istable(lua_state, TABLE_PARAMETER) )
            {
                SWEET_ERROR( RuntimeError("Table or nothing expected as third parameter when creating a target") );
            }
            
            lua_pushvalue( lua_state, TABLE_PARAMETER );
            lua_create_object_with_existing_table( lua_state, target );
            target->set_referenced_by_script( true );
        // @todo
        //  I don't think I need to call Target::set_prototype() from 
        //  ScriptInterface::target() when a Target has just been created
        //  because the Graph will have set the TargetPrototype when it created the
        //  Target and it will be setup correctly in the script when 
        //  ScriptInterface::update_target() is called a bit later on.
            target->set_prototype( target_prototype );
            target->set_working_directory( working_directory );
            recover_target( target );
            update_target( target, target_prototype );
        }
        else
        {
            create_target( target );
            recover_target( target );
            update_target( target, target_prototype );
        }
    }
    return target;
}

int ScriptInterface::target_prototype__( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );
    
    try
    {
        const int TARGET_PROTOTYPE_PARAMETER = 1;
        if ( !lua_istable(lua_state, TARGET_PROTOTYPE_PARAMETER) )
        {
            SWEET_ERROR( RuntimeError("TargetPrototype call's first and only parameter is not a table") );
        }
        
        ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( script_interface );

        lua_rawgeti( lua_state, TARGET_PROTOTYPE_PARAMETER, 1 );
        if ( !lua_isstring(lua_state, -1) )
        {
            SWEET_ERROR( RuntimeError("TargetPrototype call's table expects a string to identify the prototype in the 1st field of the table") );
        }
        string id = lua_tostring( lua_state, -1 );
        lua_pop( lua_state, 1 );
        
        TargetPrototype* target_prototype = script_interface->target_prototype( id );
        SWEET_ASSERT( target_prototype );        
        lua_pushvalue( lua_state, TARGET_PROTOTYPE_PARAMETER );
        lua_create_object_with_existing_table( lua_state, target_prototype );
        script_interface->create_prototype( target_prototype );

        LuaConverter<TargetPrototype* >::push( lua_state, target_prototype );
        return 1;
    }
    
    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );
    }
}

int ScriptInterface::set_filename( lua_State* lua_state )
{
    const int TARGET = 1;
    const int FILENAME = 2;
    const int INDEX = 3;

    Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
    const char* filename = lua_tostring( lua_state, FILENAME );
    int index = lua_isnumber( lua_state, INDEX ) ? static_cast<int>( lua_tointeger(lua_state, INDEX) ) : 0;
    target->set_filename( string(filename), index );

    return 0;
}

int ScriptInterface::filename( lua_State* lua_state )
{
    const int TARGET = 1;
    const int INDEX = 2;

    Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
    if ( !target )
    {
        SWEET_ERROR( RuntimeError("Nil Target passed to 'Target.filename()'") );
        return lua_error( lua_state );
    }

    int index = lua_isnumber( lua_state, INDEX ) ? static_cast<int>( lua_tointeger(lua_state, INDEX) ) : 1;
    if ( index < 1 )
    {
        SWEET_ERROR( RuntimeError("Index of less than 1 passed to 'Target.filename()'; index=%d", index) );
        return lua_error( lua_state );
    }
    --index;

    if ( index < int(target->filenames().size()) )
    {
        const std::string& filename = target->filename( index );
        lua_pushlstring( lua_state, filename.c_str(), filename.length() );
    }
    else
    {
        lua_pushlstring( lua_state, "", 1 );
    }

    return 1;
}

struct GetTargetsTargetReferencedByScript
{
    ScriptInterface* script_interface_;
    
    GetTargetsTargetReferencedByScript( ScriptInterface* script_interface )
    : script_interface_( script_interface )
    {
        SWEET_ASSERT( script_interface_ );
    }
    
    bool operator()( lua_State* /*lua_state*/, Target* target ) const
    {
        SWEET_ASSERT( target );
        if ( !target->referenced_by_script() )
        {
            script_interface_->create_target( target );
        }
        return true;
    }
};

int ScriptInterface::targets( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    try
    {
        ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( script_interface );

        const int TARGET_PARAMETER = 1;
        Target* target = LuaConverter<Target* >::to( lua_state, TARGET_PARAMETER );
        if ( !target )
        {
            SWEET_ERROR( RuntimeError("Target is null when iterating over children") );
        }

        const vector<Target* >& dependencies = target->targets();
        lua_push_iterator( lua_state, dependencies.begin(), dependencies.end(), GetTargetsTargetReferencedByScript(script_interface) );
        return 1;
    }

    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );        
    }
}

int ScriptInterface::dependency( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = LuaConverter<Target* >::to( lua_state, TARGET );
    if ( !target )
    {
        SWEET_ERROR( RuntimeError("Nil Target passed to 'Target.dependency()'") );
        return lua_error( lua_state );
    }

    int index = lua_isnumber( lua_state, INDEX ) ? static_cast<int>( lua_tointeger(lua_state, INDEX) ) : 1;
    if ( index < 1 )
    {
        SWEET_ERROR( RuntimeError("Index of less than 1 passed to 'Target.dependency()'; index=%d", index) );
        return lua_error( lua_state );
    }
    --index;

    Target* dependency = target->dependency( index );
    if ( dependency )
    {
        if ( !dependency->referenced_by_script() )
        {
            ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
            SWEET_ASSERT( script_interface );
            script_interface->create_target( dependency );
        }
        LuaConverter<Target* >::push( lua_state, dependency );
    }
    else
    {
        lua_pushnil( lua_state );
    }
    return 1;
}

int ScriptInterface::dependencies_iterator( lua_State* lua_state )
{
    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
    int index = lua_tointeger( lua_state, INDEX );
    if ( target )
    {
        Target* dependency = target->dependency( index - 1 );
        if ( dependency )
        {
            if ( !dependency->referenced_by_script() )
            {
                ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
                SWEET_ASSERT( script_interface );
                script_interface->create_target( dependency );
            }
            lua_pushinteger( lua_state, index + 1 );
            LuaConverter<Target*>::push( lua_state, dependency );
            return 2;
        }
    }
    return 0;
}

int ScriptInterface::dependencies( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
    if ( !target )
    {
        SWEET_ERROR( RuntimeError("Nil Target passed to 'Target.dependencies()'") );
        return lua_error( lua_state );
    }

    ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( script_interface );    
    lua_pushlightuserdata( lua_state, script_interface );
    lua_pushcclosure( lua_state, &ScriptInterface::dependencies_iterator, 1 );
    LuaConverter<Target*>::push( lua_state, target );
    lua_pushinteger( lua_state, 1 );
    return 3;
}

int ScriptInterface::absolute_( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    try
    {
        const int PATH = 1;
        if ( lua_type(lua_state, PATH) != LUA_TSTRING )
        {
            SWEET_ERROR( RuntimeError("The first parameter of 'absolute()' is not a string as expected") );
        }
        std::string path = lua_tostring( lua_state, PATH );
        
        const int WORKING_DIRECTORY = 2;
        fs::Path working_directory;    
        if ( !lua_isnoneornil(lua_state, WORKING_DIRECTORY) )
        {
            if ( lua_type(lua_state, WORKING_DIRECTORY) != LUA_TSTRING )
            {
                SWEET_ERROR( RuntimeError("The second parameter of 'absolute()' is not a string as expected") );
            }
            working_directory = fs::Path( lua_tostring(lua_state, WORKING_DIRECTORY) );
        }
        
        ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( script_interface );
        std::string absolute_path = script_interface->absolute( path, working_directory );
        lua_pushlstring( lua_state, absolute_path.c_str(), absolute_path.length() );
        return 1;
    }

    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );        
    }
}

int ScriptInterface::relative_( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    try
    {
        const int PATH = 1;
        const int WORKING_DIRECTORY = 2;

        if ( lua_type(lua_state, PATH) != LUA_TSTRING )
        {
            SWEET_ERROR( RuntimeError("The first parameter of 'relative()' is not a string as expected") );
        }
        std::string path = lua_tostring( lua_state, PATH );
        
        fs::Path working_directory;    
        if ( !lua_isnoneornil(lua_state, WORKING_DIRECTORY) )
        {
            if ( lua_type(lua_state, WORKING_DIRECTORY) != LUA_TSTRING )
            {
                SWEET_ERROR( RuntimeError("The second parameter of 'relative()' is not a string as expected") );
            }
            working_directory = fs::Path( lua_tostring(lua_state, WORKING_DIRECTORY) );
        }
        
        ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( script_interface );
        std::string relative_path = script_interface->relative( path, working_directory );
        lua_pushlstring( lua_state, relative_path.c_str(), relative_path.length() );
        return 1;
    }

    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );        
    }
}

int ScriptInterface::root_( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    try
    {
        std::string path;
        if ( lua_gettop(lua_state) > 0 && !lua_isnoneornil(lua_state, -1) )
        {
            path = LuaConverter<std::string>::to( lua_state, -1 );        
            lua_pop( lua_state, 1 );
        }
        
        ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( script_interface );
        std::string absolute_path = script_interface->root( path );
        lua_pushlstring( lua_state, absolute_path.c_str(), absolute_path.length() );
        return 1;
    }

    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );        
    }
}

int ScriptInterface::initial_( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    try
    {
        std::string path;
        if ( lua_gettop(lua_state) > 0 && !lua_isnoneornil(lua_state, -1) )
        {
            path = LuaConverter<std::string>::to( lua_state, -1 );        
            lua_pop( lua_state, 1 );
        }
        
        ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( script_interface );
        std::string absolute_path = script_interface->initial( path );
        lua_pushlstring( lua_state, absolute_path.c_str(), absolute_path.length() );
        return 1;
    }

    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );        
    }
}

int ScriptInterface::home_( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    try
    {
        std::string path;
        if ( lua_gettop(lua_state) > 0 && !lua_isnoneornil(lua_state, -1) )
        {
            path = LuaConverter<std::string>::to( lua_state, -1 );        
            lua_pop( lua_state, 1 );
        }
        
        ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( script_interface );
        std::string absolute_path = script_interface->home( path );
        lua_pushlstring( lua_state, absolute_path.c_str(), absolute_path.length() );
        return 1;
    }

    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );        
    }
}

int ScriptInterface::executable_( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    try
    {
        std::string path;
        if ( lua_gettop(lua_state) > 0 && !lua_isnoneornil(lua_state, -1) )
        {
            path = LuaConverter<std::string>::to( lua_state, -1 );        
            lua_pop( lua_state, 1 );
        }
        
        ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( script_interface );
        std::string absolute_path = script_interface->executable( path );
        lua_pushlstring( lua_state, absolute_path.c_str(), absolute_path.length() );
        return 1;
    }

    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );        
    }
}

int ScriptInterface::getenv_( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    if ( lua_gettop(lua_state) > 0 && lua_isstring(lua_state, -1) )
    {
        const char* name = lua_tostring( lua_state, -1 );
        ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( script_interface );
        const char* value = script_interface->getenv( name );        
        lua_pop( lua_state, 1 );
        if ( value )
        {
            lua_pushstring( lua_state, value );
        }
        else
        {
            lua_pushnil( lua_state );
        }
    }
    else
    {
        lua_pushnil( lua_state );
    }
    
    return 1;
}

int ScriptInterface::file( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    try
    {
        ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( script_interface );        
        Target* target = script_interface->add_target( lua_state );
        target->set_filename( target->path() );
        LuaConverter<Target* >::push( lua_state, target );
        return 1;
    }

    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );
    }
}

int ScriptInterface::target( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );
    
    try
    {
        ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( script_interface );        
        Target* target = script_interface->add_target( lua_state );
        LuaConverter<Target* >::push( lua_state, target );
        return 1;
    }

    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );
    }
}

int ScriptInterface::postorder( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    try 
    {
        ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( script_interface );

        Graph* graph = script_interface->build_tool_->graph();
        if ( graph->traversal_in_progress() )
        {
            SWEET_ERROR( PostorderCalledRecursivelyError("Postorder called from within another bind or postorder traversal") );
            return lua_error( lua_state );
        }
         
        const int FUNCTION_PARAMETER = 1;
        LuaValue function( script_interface->lua_, lua_state, FUNCTION_PARAMETER );

        const int TARGET_PARAMETER = 2;
        Target* target = NULL;
        if ( !lua_isnoneornil(lua_state, TARGET_PARAMETER) )
        {
            target = LuaConverter<Target* >::to( lua_state, TARGET_PARAMETER );
        }

        int bind_failures = script_interface->build_tool_->graph()->bind( target );
        if ( bind_failures > 0 )
        {
            lua_pushinteger( lua_state, bind_failures );
            return 1;
        }

        int failures = script_interface->build_tool_->scheduler()->postorder( function, target );
        lua_pushinteger( lua_state, failures );
        return 1;
    }
    
    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );
    }
}

int ScriptInterface::execute( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );
    
    try
    {
        ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( script_interface );

        const int COMMAND = 1;
        string command = LuaConverter<string>::to( lua_state, COMMAND );
        
        const int COMMAND_LINE = 2;
        string command_line = LuaConverter<string>::to( lua_state, COMMAND_LINE );
        
        const int ENVIRONMENT = 3;
        unique_ptr<process::Environment> environment;
        if ( !lua_isnoneornil(lua_state, ENVIRONMENT) )
        {
            if ( !lua_istable(lua_state, ENVIRONMENT) )
            {
                lua_pushstring( lua_state, "Expected an environment table or nil as 3rd parameter" );
                return lua_error( lua_state );
            }
            
            environment.reset( new process::Environment );
            lua_pushnil( lua_state );
            while ( lua_next(lua_state, ENVIRONMENT) )
            {
                if ( lua_isstring(lua_state, -2) )
                {
                    const char* key = lua_tostring( lua_state, -2 );
                    const char* value = lua_tostring( lua_state, -1 );
                    environment->append( key, value );
                }
                lua_pop( lua_state, 1 );
            }
        }

        const int DEPENDENCIES_FILTER = 4;
        unique_ptr<lua::LuaValue> dependencies_filter;
        if ( !lua_isnoneornil(lua_state, DEPENDENCIES_FILTER) )
        {
            if ( !lua_isfunction(lua_state, DEPENDENCIES_FILTER) && !lua_istable(lua_state, DEPENDENCIES_FILTER) )
            {
                lua_pushstring( lua_state, "Expected a function or callable table as 4th parameter (dependencies filter)" );
                return lua_error( lua_state );
            }
            dependencies_filter.reset( new lua::LuaValue(script_interface->lua_, lua_state, DEPENDENCIES_FILTER) );
        }

        const int STDOUT_FILTER = 5;
        unique_ptr<lua::LuaValue> stdout_filter;
        if ( !lua_isnoneornil(lua_state, STDOUT_FILTER) )
        {
            if ( !lua_isfunction(lua_state, STDOUT_FILTER) && !lua_istable(lua_state, STDOUT_FILTER) )
            {
                lua_pushstring( lua_state, "Expected a function or callable table as 4th parameter (dependencies filter)" );
                return lua_error( lua_state );
            }
            stdout_filter.reset( new lua::LuaValue(script_interface->lua_, lua_state, STDOUT_FILTER) );
        }

        const int STDERR_FILTER = 6;
        unique_ptr<lua::LuaValue> stderr_filter;
        if ( !lua_isnoneornil(lua_state, STDERR_FILTER) )
        {
            if ( !lua_isfunction(lua_state, STDERR_FILTER) && !lua_istable(lua_state, STDERR_FILTER) )
            {
                lua_pushstring( lua_state, "Expected a function or callable table as 4th parameter (dependencies filter)" );
                return lua_error( lua_state );
            }
            stderr_filter.reset( new lua::LuaValue(script_interface->lua_, lua_state, STDERR_FILTER) );
        }

        const int ARGUMENTS = 7;
        unique_ptr<Arguments> arguments;
        if ( lua_gettop(lua_state) >= ARGUMENTS )
        {
            arguments.reset( new Arguments(script_interface->lua_, lua_state, ARGUMENTS, lua_gettop(lua_state) + 1) );
        }

        script_interface->build_tool_->scheduler()->execute( 
            command, 
            command_line, 
            environment.release(), 
            dependencies_filter.release(), 
            stdout_filter.release(), 
            stderr_filter.release(), 
            arguments.release(), 
            script_interface->context() 
        );

        return lua_yield( lua_state, 0 );
    }

    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );
    }
}
