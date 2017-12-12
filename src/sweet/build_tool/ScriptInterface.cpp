//
// ScriptInterface.cpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "ScriptInterface.hpp"
#include "Error.hpp"
#include "Scanner.hpp"
#include "TargetPrototype.hpp"
#include "Target.hpp"
#include "Arguments.hpp"
#include "Graph.hpp"
#include "BuildTool.hpp"
#include "Environment.hpp"
#include "Scheduler.hpp"
#include "OsInterface.hpp"
#include <sweet/lua/lua_functions.ipp>
#include <sweet/lua/filesystem.hpp>
#include <sweet/lua/vector.hpp>
#include <sweet/lua/ptr.hpp>
#include <sweet/lua/Lua.hpp>
#include <stdlib.h>

using std::string;
using std::vector;
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
  scanner_metatable_( lua_ ),
  scanner_prototype_( lua_ ),
  target_metatable_( lua_ ),
  target_prototype_( lua_ ),
  target_prototypes_(),
  environments_(),
  root_directory_(),
  initial_directory_()
{
    SWEET_ASSERT( os_interface_ );
    SWEET_ASSERT( build_tool_ );

    target_prototype_metatable_.members()
        ( "__index", target_prototype_prototype_ )
    ;

    target_prototype_prototype_.members()
        .type( SWEET_STATIC_TYPEID(TargetPrototype) )
        ( "id", &TargetPrototype::get_id )
    ;

    scanner_metatable_.members()
        ( "__index", scanner_prototype_ )
    ;

    scanner_prototype_.members()
        .type( SWEET_STATIC_TYPEID(Scanner) )
        ( "set_initial_lines", &Scanner::set_initial_lines )
        ( "initial_lines", &Scanner::initial_lines )
        ( "set_later_lines", &Scanner::set_later_lines )
        ( "later_lines", &Scanner::later_lines )
        ( "set_maximum_matches", &Scanner::set_maximum_matches )
        ( "maximum_matches", &Scanner::maximum_matches )
    ;

    target_metatable_.members()
        ( "__index", target_prototype_ )
    ;

    target_prototype_.members()
        .type( SWEET_STATIC_TYPEID(Target) )
        ( "id", &Target::get_id )
        ( "path", &Target::get_path )
        ( "branch", &Target::get_branch )
        ( "directory", &Target::get_branch )
        ( "parent", &ScriptInterface::get_parent, this, _1 )
        ( "prototype", &Target::get_prototype )
        ( "set_required_to_exist", &Target::set_required_to_exist )
        ( "is_required_to_exist", &Target::is_required_to_exist )
        ( "set_timestamp", &Target::set_timestamp )
        ( "get_timestamp", &Target::get_timestamp )
        ( "get_last_write_time", &Target::get_last_write_time )
        ( "set_outdated", &Target::set_outdated )
        ( "is_outdated", &Target::is_outdated )
        ( "set_filename", &Target::set_filename )
        ( "get_filename", &Target::get_filename )
        ( "set_working_directory", &Target::set_working_directory )
        ( "get_working_directory", &ScriptInterface::get_working_directory, this, _1 )
        ( "get_targets", raw(&ScriptInterface::get_targets), this )
        ( "add_dependency", &Target::add_dependency )
        ( "remove_dependency", &Target::remove_dependency )
        ( "dependency", raw(&ScriptInterface::dependency), this )
        ( "get_dependencies", raw(&ScriptInterface::get_dependencies), this )
    ;

    lua_.globals()
        ( "set_maximum_parallel_jobs", &BuildTool::set_maximum_parallel_jobs, build_tool_ )
        ( "get_maximum_parallel_jobs", &BuildTool::get_maximum_parallel_jobs, build_tool_ )
        ( "set_stack_trace_enabled", &BuildTool::set_stack_trace_enabled, build_tool_ )
        ( "is_stack_trace_enabled", &BuildTool::is_stack_trace_enabled, build_tool_ )
    ;

    lua_.globals()
        ( "Scanner", raw(&ScriptInterface::scanner), this )
        ( "TargetPrototype", raw(&ScriptInterface::target_prototype__), this )
        ( "file", raw(&ScriptInterface::file), this )
        ( "target", raw(&ScriptInterface::target), this )
        ( "find_target", &ScriptInterface::find_target, this )
        ( "absolute", raw(&ScriptInterface::absolute_), this )
        ( "relative", raw(&ScriptInterface::relative_), this )
        ( "root", raw(&ScriptInterface::root_), this )
        ( "initial", raw(&ScriptInterface::initial_), this )
        ( "home", raw(&ScriptInterface::home_), this )
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
        ( "scan", raw(&ScriptInterface::scan), this )
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
        ( "putenv", &ScriptInterface::putenv, this )
        ( "getenv", raw(&ScriptInterface::getenv_), this )
        ( "sleep", &ScriptInterface::sleep, this )
        ( "ticks", &ScriptInterface::ticks, this )
        ( "buildfile", &ScriptInterface::buildfile, this )
        ( "bind", &ScriptInterface::bind, this )
        ( "preorder", raw(&ScriptInterface::preorder), this )
        ( "postorder", raw(&ScriptInterface::postorder), this )
        ( "mark_implicit_dependencies", &ScriptInterface::mark_implicit_dependencies, this )
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

lua::Lua& ScriptInterface::get_lua()
{
    return lua_;
}

void ScriptInterface::push_environment( ptr<Environment> environment )
{
    SWEET_ASSERT( environment );
    environments_.push_back( environment );
    build_tool_->error_policy().push_errors();
}

int ScriptInterface::pop_environment()
{
    SWEET_ASSERT( !environments_.empty() );
    int errors = 0;
    if ( !environments_.empty() )
    {
        environments_.pop_back();
        errors = build_tool_->error_policy().pop_errors();
    }
    return errors;
}

ptr<Environment> ScriptInterface::get_environment() const
{
    SWEET_ASSERT( !environments_.empty() );
    SWEET_ASSERT( environments_.back() );
    return environments_.back();
}

void ScriptInterface::set_root_directory( const path::Path& root_directory )
{
    SWEET_ASSERT( root_directory.is_absolute() );
    root_directory_ = root_directory;
}

const path::Path& ScriptInterface::get_root_directory() const
{
    return root_directory_;
}

void ScriptInterface::set_initial_directory( const path::Path& initial_directory )
{
    SWEET_ASSERT( initial_directory.is_absolute() );
    initial_directory_ = initial_directory;
}

const path::Path& ScriptInterface::get_initial_directory() const
{
    return initial_directory_;
}

void ScriptInterface::create_prototype( ptr<TargetPrototype> target_prototype )
{
    SWEET_ASSERT( target_prototype );
    
    lua_.members( target_prototype )
        .type( SWEET_STATIC_TYPEID(TargetPrototype) )
        .metatable( target_prototype_metatable_ )
        .this_pointer( target_prototype.get() )
        ( lua::PTR_KEYWORD, value(target_prototype) )
        ( "id", &Target::get_id )
        ( "path", &Target::get_path )
        ( "branch", &Target::get_branch )
        ( "directory", &Target::get_branch )
        ( "parent", &ScriptInterface::get_parent, this, _1 )
        ( "prototype", &Target::get_prototype )
        ( "set_required_to_exist", &Target::set_required_to_exist )
        ( "is_required_to_exist", &Target::is_required_to_exist )
        ( "set_timestamp", &Target::set_timestamp )
        ( "get_timestamp", &Target::get_timestamp )
        ( "get_last_write_time", &Target::get_last_write_time )
        ( "set_outdated", &Target::set_outdated )
        ( "is_outdated", &Target::is_outdated )
        ( "set_filename", &Target::set_filename )
        ( "get_filename", &Target::get_filename )
        ( "set_working_directory", &Target::set_working_directory )
        ( "get_working_directory", &ScriptInterface::get_working_directory, this, _1 )
        ( "get_targets", raw(&ScriptInterface::get_targets), this )
        ( "add_dependency", &Target::add_dependency )
        ( "remove_dependency", &Target::remove_dependency )
        ( "dependency", raw(&ScriptInterface::dependency), this )
        ( "get_dependencies", raw(&ScriptInterface::get_dependencies), this )
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

void ScriptInterface::create_scanner( ptr<Scanner> scanner )
{
    SWEET_ASSERT( scanner );
    lua_.create( scanner );   
    lua_.members( scanner )
        .type( SWEET_STATIC_TYPEID(Scanner) )
        .metatable( scanner_metatable_ )
        .this_pointer( scanner.get() )
        ( lua::PTR_KEYWORD, value(scanner) )
    ;
}

void ScriptInterface::destroy_scanner( Scanner* scanner )
{
    SWEET_ASSERT( scanner );
    lua_.destroy( scanner );
}

void ScriptInterface::create_target( ptr<Target> target )
{
    SWEET_ASSERT( target );

    if ( !target->is_referenced_by_script() )
    {
        lua_.create( target );
        target->set_referenced_by_script( true );
        recover_target( target );
    }
}

void ScriptInterface::recover_target( ptr<Target> target )
{
    SWEET_ASSERT( target );
    lua_.members( target )
        .type( SWEET_STATIC_TYPEID(Target) )
        .metatable( target_metatable_ )
        .this_pointer( target.get() )
        ( lua::PTR_KEYWORD, value(target) )
    ;
}

void ScriptInterface::update_target( ptr<Target> target, ptr<TargetPrototype> target_prototype )
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

ptr<TargetPrototype> ScriptInterface::target_prototype( const std::string& id )
{   
    SWEET_ASSERT( !id.empty() );

    vector<ptr<TargetPrototype> >::const_iterator i = target_prototypes_.begin(); 
    while ( i != target_prototypes_.end() && (*i)->get_id() != id )
    {
        ++i;
    }

    ptr<TargetPrototype> target_prototype;
    if ( i == target_prototypes_.end() )
    {
        target_prototype.reset( new TargetPrototype(id, build_tool_), Deleter<TargetPrototype>(&ScriptInterface::destroy_prototype, this) );
        target_prototypes_.push_back( target_prototype );
    }
    else
    {
        target_prototype = *i;
    }    
    return target_prototype;
}

ptr<Target> ScriptInterface::find_target( const std::string& id )
{
    Environment* environment = get_environment().get();
    SWEET_ASSERT( environment );
    ptr<Target> target = build_tool_->get_graph()->find_target( id, environment->working_directory() );
    if ( target && !target->is_referenced_by_script() )
    {
        create_target( target );
    }
    return target;
}

std::string ScriptInterface::absolute( const std::string& path, const path::Path& working_directory )
{
    if ( path::Path(path).is_absolute() )
    {
        return path;
    }
    else if ( working_directory.empty() )
    {
        Environment* environment = get_environment().get();
        SWEET_ASSERT( environment );
        path::Path absolute_path( environment->directory() );
        absolute_path /= path;
        absolute_path.normalize();
        return absolute_path.string();
    }
    else
    {
        path::Path absolute_path( working_directory );
        absolute_path /= path;
        absolute_path.normalize();
        return absolute_path.string();
    }
}

std::string ScriptInterface::relative( const std::string& path, const path::Path& working_directory )
{
    if ( working_directory.empty() )
    {
        Environment* environment = get_environment().get();
        SWEET_ASSERT( environment );
        return environment->directory().relative( path::Path(path) ).string();
    }
    else
    {
        return working_directory.relative( path::Path(path) ).string();
    }
}

std::string ScriptInterface::root( const std::string& path ) const
{
    if ( path::Path(path).is_absolute() )
    {
        return path;
    }

    path::Path absolute_path( root_directory_ );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path.string();
}

std::string ScriptInterface::initial( const std::string& path ) const
{
    if ( path::Path(path).is_absolute() )
    {
        return path;
    }

    path::Path absolute_path( initial_directory_ );
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
    
    if ( path::Path(path).is_absolute() )
    {
        return path;
    }

    path::Path absolute_path( home );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path.string();
}

std::string ScriptInterface::anonymous() const
{
    Environment* environment = get_environment().get();
    SWEET_ASSERT( environment );
    ptr<Target> working_directory = environment->working_directory();
    SWEET_ASSERT( working_directory );
    char anonymous [256];
    unsigned int length = sprintf( anonymous, "$$%d", working_directory->anonymous() );
    return string( anonymous, length );
}

bool ScriptInterface::is_absolute( const std::string& path )
{
    return path::Path(path).is_absolute();
}

bool ScriptInterface::is_relative( const std::string& path )
{
    return path::Path(path).is_relative();
}

void ScriptInterface::cd( const std::string& path )
{
    Environment* environment = get_environment().get();
    SWEET_ASSERT( environment );
    environment->change_directory( path );
}

void ScriptInterface::pushd( const std::string& path )
{
    Environment* environment = get_environment().get();
    SWEET_ASSERT( environment );
    environment->push_directory( path );
}

void ScriptInterface::popd()
{
    Environment* environment = get_environment().get();
    SWEET_ASSERT( environment );
    environment->pop_directory();
}

const std::string& ScriptInterface::pwd() const
{
    Environment* environment = get_environment().get();
    SWEET_ASSERT( environment );
    return environment->directory().string();
}

ptr<Target> ScriptInterface::working_directory()
{
    Environment* environment = get_environment().get();
    SWEET_ASSERT( environment );
    
    ptr<Target> target = environment->working_directory();
    if ( target && !target->is_referenced_by_script() )
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
    return path::Path( path ).leaf();
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
    Environment* environment = get_environment().get();
    SWEET_ASSERT( environment );
    return os_interface_->cpdir( absolute(from), absolute(to), environment->directory().branch() );
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

void ScriptInterface::putenv( const std::string& attribute, const std::string& value )
{
    os_interface_->putenv( attribute, value );
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

void ScriptInterface::buildfile( const std::string& filename, ptr<Target> target )
{
    SWEET_ASSERT( build_tool_ );
    return build_tool_->get_graph()->buildfile( filename, target );
}

int ScriptInterface::bind( ptr<Target> target )
{
    SWEET_ASSERT( build_tool_ );
    return build_tool_->get_graph()->bind( target );
}

void ScriptInterface::mark_implicit_dependencies()
{
    SWEET_ASSERT( build_tool_ );
    return build_tool_->get_graph()->mark_implicit_dependencies();
}

void ScriptInterface::print_dependencies( ptr<Target> target )
{
    SWEET_ASSERT( build_tool_ );
    return build_tool_->get_graph()->print_dependencies( target, get_environment()->directory().string() );
}

void ScriptInterface::print_namespace( ptr<Target> target )
{
    SWEET_ASSERT( build_tool_ );
    return build_tool_->get_graph()->print_namespace( target );
}

void ScriptInterface::wait()
{
    SWEET_ASSERT( build_tool_ );
    build_tool_->get_scheduler()->wait();
}

void ScriptInterface::clear()
{
    Environment* environment = environments_.back().get();
    string working_directory = environment->working_directory()->get_path();
    Graph* graph = build_tool_->get_graph();
    graph->clear();
    environment->reset_directory( working_directory );
}

Target* ScriptInterface::load_xml( const std::string& filename )
{
    SWEET_ASSERT( build_tool_ );
    SWEET_ASSERT( build_tool_->get_graph() );

    Environment* environment = environments_.back().get();
    string working_directory = environment->working_directory()->get_path();
    ptr<Target> cache_target = build_tool_->get_graph()->load_xml( absolute(filename) );
    environment->reset_directory( working_directory );
    if ( cache_target )
    {
        create_target( cache_target );
    }
    return cache_target.get();
}

void ScriptInterface::save_xml()
{
    SWEET_ASSERT( build_tool_ );
    build_tool_->get_graph()->save_xml();
}

Target* ScriptInterface::load_binary( const std::string& filename )
{
    SWEET_ASSERT( build_tool_ );
    SWEET_ASSERT( build_tool_->get_graph() );
        
    Environment* environment = environments_.back().get();
    string working_directory = environment->working_directory()->get_path();
    ptr<Target> cache_target = build_tool_->get_graph()->load_binary( absolute(filename) );
    environment->reset_directory( working_directory );
    if ( cache_target )
    {
        create_target( cache_target );
    }
    return cache_target.get();
}

void ScriptInterface::save_binary()
{
    SWEET_ASSERT( build_tool_ );
    build_tool_->get_graph()->save_binary();
}

ptr<Target> ScriptInterface::get_parent( ptr<Target> target )
{
    SWEET_ASSERT( target );
    
    ptr<Target> parent;
    if ( target )
    {
        parent = target->get_parent();
        if ( parent && !parent->is_referenced_by_script() )
        {
            create_target( parent );
        }
    }
    return parent;
}

ptr<Target> ScriptInterface::get_working_directory( ptr<Target> target )
{
    SWEET_ASSERT( target );
    
    ptr<Target> working_directory;
    if ( target )
    {
        working_directory = target->get_working_directory();
        if ( !working_directory->is_referenced_by_script() )
        {
            create_target( working_directory );
        }
    }
    return working_directory;
}

ptr<Target> ScriptInterface::add_target( lua_State* lua_state )
{
    Environment* environment = get_environment().get();
    SWEET_ASSERT( environment );

    Graph* graph = build_tool_->get_graph();
    SWEET_ASSERT( graph );

    ptr<Target> working_directory = environment->working_directory();
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
    ptr<TargetPrototype> target_prototype = LuaConverter<ptr<TargetPrototype> >::to( lua_state, PROTOTYPE_PARAMETER );

    ptr<Target> target = graph->target( id, target_prototype, working_directory );        
    if ( !target->is_referenced_by_script() )
    {
        const int TABLE_PARAMETER = 3;
        if ( !lua_isnoneornil(lua_state, TABLE_PARAMETER) )
        {        
            if ( !lua_istable(lua_state, TABLE_PARAMETER) )
            {
                SWEET_ERROR( RuntimeError("Table or nothing expected as third parameter when creating a target") );
            }
            
            lua_pushvalue( lua_state, TABLE_PARAMETER );
            lua_create_object_with_existing_table( lua_state, target.get() );
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
        
        ptr<TargetPrototype> target_prototype = script_interface->target_prototype( id );
        SWEET_ASSERT( target_prototype );        
        lua_pushvalue( lua_state, TARGET_PROTOTYPE_PARAMETER );
        lua_create_object_with_existing_table( lua_state, target_prototype.get() );
        script_interface->create_prototype( target_prototype );

        LuaConverter<ptr<TargetPrototype> >::push( lua_state, target_prototype );
        return 1;
    }
    
    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );
    }
}

struct GetTargetsTargetReferencedByScript
{
    ScriptInterface* script_interface_;
    
    GetTargetsTargetReferencedByScript( ScriptInterface* script_interface )
    : script_interface_( script_interface )
    {
        SWEET_ASSERT( script_interface_ );
    }
    
    bool operator()( lua_State* lua_state, ptr<Target> target ) const
    {
        SWEET_ASSERT( target );
        if ( !target->is_referenced_by_script() )
        {
            script_interface_->create_target( target );
        }
        return true;
    }
};

int ScriptInterface::get_targets( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    try
    {
        ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( script_interface );

        const int TARGET_PARAMETER = 1;
        ptr<Target> target = LuaConverter<ptr<Target> >::to( lua_state, TARGET_PARAMETER );
        if ( !target )
        {
            SWEET_ERROR( RuntimeError("Target is null when iterating over children") );
        }

        const vector<ptr<Target> >& dependencies = target->get_targets();
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
    ptr<Target> target = LuaConverter<ptr<Target> >::to( lua_state, TARGET );
    if ( !target )
    {
        SWEET_ERROR( RuntimeError("Null Target passed to 'Target.dependency()'") );
        return lua_error( lua_state );
    }

    int index = lua_isnumber( lua_state, INDEX ) ? lua_tointeger( lua_state, INDEX ) : 1;
    if ( index < 1 )
    {
        SWEET_ERROR( RuntimeError("Index of less than 1 passed to 'Target.dependency()'; index=%d", index) );
        return lua_error( lua_state );
    }

    Target* dependency = target->dependency( index - 1 );
    if ( dependency )
    {
        if ( !dependency->is_referenced_by_script() )
        {
            ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
            SWEET_ASSERT( script_interface );
            script_interface->create_target( dependency->ptr_from_this() );
        }
        LuaConverter<ptr<Target> >::push( lua_state, dependency->ptr_from_this() );
    }
    else
    {
        lua_pushnil( lua_state );
    }
    return 1;
}

struct GetDependenciesTargetReferencedByScript
{
    ScriptInterface* script_interface_;
    
    GetDependenciesTargetReferencedByScript( ScriptInterface* script_interface )
    : script_interface_( script_interface )
    {
        SWEET_ASSERT( script_interface_ );
    }
    
    bool operator()( lua_State* lua_state, Target* target ) const
    {
        SWEET_ASSERT( target );
        if ( !target->is_referenced_by_script() )
        {
            script_interface_->create_target( target->ptr_from_this() );
        }
        return true;
    }
};

int ScriptInterface::get_dependencies( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    try
    {
        ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( script_interface );

        const int TARGET_PARAMETER = 1;
        ptr<Target> target = LuaConverter<ptr<Target> >::to( lua_state, TARGET_PARAMETER );
        if ( !target )
        {
            SWEET_ERROR( RuntimeError("Null Target passed to 'Target.get_dependencies()'") );
        }

        const vector<Target*>& dependencies = target->get_dependencies();
        lua_push_iterator( lua_state, dependencies.begin(), dependencies.end(), GetDependenciesTargetReferencedByScript(script_interface) );
        return 1;
    }

    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );        
    }
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
        path::Path working_directory;    
        if ( !lua_isnoneornil(lua_state, WORKING_DIRECTORY) )
        {
            if ( lua_type(lua_state, WORKING_DIRECTORY) != LUA_TSTRING )
            {
                SWEET_ERROR( RuntimeError("The second parameter of 'absolute()' is not a string as expected") );
            }
            working_directory = path::Path( lua_tostring(lua_state, WORKING_DIRECTORY) );
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
        
        path::Path working_directory;    
        if ( !lua_isnoneornil(lua_state, WORKING_DIRECTORY) )
        {
            if ( lua_type(lua_state, WORKING_DIRECTORY) != LUA_TSTRING )
            {
                SWEET_ERROR( RuntimeError("The second parameter of 'relative()' is not a string as expected") );
            }
            working_directory = path::Path( lua_tostring(lua_state, WORKING_DIRECTORY) );
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

int ScriptInterface::getenv_( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const char* name = NULL;
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
        ptr<Target> target = script_interface->add_target( lua_state );
        target->set_filename( target->get_path() );
        LuaConverter<ptr<Target> >::push( lua_state, target );
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
        ptr<Target> target = script_interface->add_target( lua_state );
        LuaConverter<ptr<Target> >::push( lua_state, target );
        return 1;
    }

    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );
    }
}

int ScriptInterface::scanner( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );
    
    try 
    {
        const int SCANNER_PARAMETER = 1;
        if ( !lua_istable(lua_state, SCANNER_PARAMETER) )
        {
            SWEET_ERROR( RuntimeError("Table not passed when creating a Scanner") );
        }
        
        ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( script_interface );

        const unsigned int PATTERNS_RESERVE = 8;
        ptr<Scanner> scanner( new Scanner(PATTERNS_RESERVE, script_interface->build_tool_), Deleter<Scanner>(&ScriptInterface::destroy_scanner, script_interface) );
        script_interface->create_scanner( scanner );
        
        lua_pushnil( lua_state );
        while ( lua_next(lua_state, SCANNER_PARAMETER) != 0 )
        {
            if ( lua_isstring(lua_state, -2) && lua_isfunction(lua_state, -1) )
            {
                std::string regex = lua_tostring( lua_state, -2 );
                scanner->add_pattern( regex, script_interface->lua_, lua_state, -1 );
            }
            lua_pop( lua_state, 1 );
        }
        
        lua_push( lua_state, scanner );
        return 1;
    }
    
    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );
    }
}

int ScriptInterface::preorder( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    try 
    {
        ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( script_interface );
        
        const int FUNCTION_PARAMETER = 1;
        LuaValue function( script_interface->lua_, lua_state, FUNCTION_PARAMETER );

        const int TARGET_PARAMETER = 2;
        ptr<Target> target;
        if ( !lua_isnoneornil(lua_state, TARGET_PARAMETER) )
        {
            target = LuaConverter<ptr<Target> >::to( lua_state, TARGET_PARAMETER );
        }
        
        int failures = script_interface->build_tool_->get_scheduler()->preorder( function, target );
        lua_pushinteger( lua_state, failures );        
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
        
        const int FUNCTION_PARAMETER = 1;
        LuaValue function( script_interface->lua_, lua_state, FUNCTION_PARAMETER );

        const int TARGET_PARAMETER = 2;
        ptr<Target> target;
        if ( !lua_isnoneornil(lua_state, TARGET_PARAMETER) )
        {
            target = LuaConverter<ptr<Target> >::to( lua_state, TARGET_PARAMETER );
        }
        
        int failures = script_interface->build_tool_->get_scheduler()->postorder( function, target );
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

        const int COMMAND_PARAMETER = 1;
        string command = LuaConverter<string>::to( lua_state, COMMAND_PARAMETER );
        
        const int COMMAND_LINE_PARAMETER = 2;
        string command_line = LuaConverter<string>::to( lua_state, COMMAND_LINE_PARAMETER );
        
        const int SCANNER_PROTOTYPE_PARAMETER = 3;
        ptr<Scanner> scanner;
        if ( !lua_isnoneornil(lua_state, SCANNER_PROTOTYPE_PARAMETER) )
        {
            scanner = LuaConverter<ptr<Scanner> >::to( lua_state, SCANNER_PROTOTYPE_PARAMETER );
        }

        const int ARGUMENTS_PARAMETER = 4;
        ptr<Arguments> arguments;
        if ( lua_gettop(lua_state) >= ARGUMENTS_PARAMETER )
        {
            arguments.reset( new Arguments(script_interface->lua_, lua_state, ARGUMENTS_PARAMETER, lua_gettop(lua_state) + 1) );
        }

        script_interface->build_tool_->get_scheduler()->execute( command, command_line, scanner, arguments, script_interface->get_environment() );
        return lua_yield( lua_state, 0 );
    }

    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );
    }
}

int ScriptInterface::scan( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );
    
    try
    {
        ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( script_interface );

        const int TARGET_PARAMETER = 1;
        ptr<Target> target = LuaConverter<ptr<Target> >::to( lua_state, TARGET_PARAMETER );
        
        if ( target )
        {
            const int SCANNER_PROTOTYPE_PARAMETER = 2;
            ptr<Scanner> scanner = LuaConverter<ptr<Scanner> >::to( lua_state, SCANNER_PROTOTYPE_PARAMETER );
            if ( !scanner )
            {
                SWEET_ERROR( NullScannerError("The scan() function was called on '%s' without a valid Scanner", target->get_filename().c_str()) );
            }

            const int ARGUMENTS_PARAMETER = 3;
            ptr<Arguments> arguments;
            if ( lua_gettop(lua_state) >= ARGUMENTS_PARAMETER )
            {
                arguments.reset( new Arguments(script_interface->lua_, lua_state, ARGUMENTS_PARAMETER, lua_gettop(lua_state) + 1) );
            }

            target->bind_to_file();
            if ( target->is_required_to_exist() && target->get_last_write_time() == 0 )
            {
                SWEET_ERROR( ScannedFileNotFoundError("The scanned file '%s' was not found", target->get_filename().c_str()) );
            }
            
            if ( target->get_last_write_time() != target->get_last_scan_time() )
            {
                target->clear_implicit_dependencies();
                target->set_last_scan_time( target->get_last_write_time() );
                ptr<Environment> environment = script_interface->get_environment();
                SWEET_ASSERT( environment );
                script_interface->build_tool_->get_scheduler()->scan( target, scanner, arguments, environment->working_directory(), environment );
            }    
        }

        return 0;
    }
    
    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );
    }
}
