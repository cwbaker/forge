//
// ScriptInterface.cpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "ScriptInterface.hpp"
#include "Error.hpp"
#include "Scanner.hpp"
#include "Rule.hpp"
#include "Target.hpp"
#include "Arguments.hpp"
#include "Graph.hpp"
#include "BuildTool.hpp"
#include "Environment.hpp"
#include "Scheduler.hpp"
#include "OsInterface.hpp"
#include <sweet/lua/filesystem.hpp>
#include <sweet/lua/vector.hpp>
#include <sweet/lua/ptr.hpp>
#include <sweet/lua/Lua.hpp>

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
  lua_(),
  rule_metatable_( lua_ ),
  rule_prototype_( lua_ ),
  scanner_metatable_( lua_ ),
  scanner_prototype_( lua_ ),
  target_metatable_( lua_ ),
  target_prototype_( lua_ ),
  rules_(),
  environments_(),
  root_directory_(),
  initial_directory_()
{
    SWEET_ASSERT( os_interface_ );
    SWEET_ASSERT( build_tool_ );

    rule_metatable_.members()
        ( "__call",  raw(&ScriptInterface::target_from_graph), this )
        ( "__index", rule_prototype_ )
    ;

    rule_prototype_.members()
        .type( SWEET_STATIC_TYPEID(Rule) )
        ( "id", &Rule::get_id )
    ;

    scanner_metatable_.members()
        ( "__index", scanner_prototype_ )
    ;

    scanner_prototype_.members()
        .type( SWEET_STATIC_TYPEID(Scanner) )
        ( "set_initial_lines", &Scanner::set_initial_lines )
        ( "get_initial_lines", &Scanner::get_initial_lines )
        ( "set_later_lines", &Scanner::set_later_lines )
        ( "get_later_lines", &Scanner::get_later_lines )
    ;

    target_metatable_.members()
        ( "__index", target_prototype_ )
    ;

    target_prototype_.members()
        .type( SWEET_STATIC_TYPEID(Target) )
        ( "id", &Target::get_id )
        ( "path", &Target::get_path )
        ( "directory", &Target::get_directory )
        ( "parent", &Target::get_parent )
        ( "rule", &Target::get_rule )
        ( "set_bind_type", &Target::set_bind_type )
        ( "get_bind_type", &Target::get_bind_type )
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
        ( "get_working_directory", &Target::get_working_directory )
        ( "get_targets", &Target::get_targets )
        ( "add_dependency", &Target::add_dependency )
        ( "get_dependencies", &Target::get_dependencies )
    ;

    lua_.globals()
        ( "set_maximum_parallel_jobs", &BuildTool::set_maximum_parallel_jobs, build_tool_ )
        ( "get_maximum_parallel_jobs", &BuildTool::get_maximum_parallel_jobs, build_tool_ )
        ( "set_stack_trace_enabled", &BuildTool::set_stack_trace_enabled, build_tool_ )
        ( "is_stack_trace_enabled", &BuildTool::is_stack_trace_enabled, build_tool_ )
    ;

    lua_.globals()
        ( "BIND_PHONY", int(BIND_PHONY) )
        ( "BIND_DIRECTORY", int(BIND_DIRECTORY) )
        ( "BIND_SOURCE_FILE", int(BIND_SOURCE_FILE) )
        ( "BIND_INTERMEDIATE_FILE", int(BIND_INTERMEDIATE_FILE) )
        ( "BIND_GENERATED_FILE", int(BIND_GENERATED_FILE) )        
        ( "Rule", &ScriptInterface::rule, this )
        ( "Scanner", raw(&ScriptInterface::scanner), this )
        ( "find_target", &ScriptInterface::find_target, this )
        ( "absolute", raw(&ScriptInterface::absolute_), this )
        ( "relative", raw(&ScriptInterface::relative_), this )
        ( "root", raw(&ScriptInterface::root_), this )
        ( "initial", raw(&ScriptInterface::initial_), this )
        ( "home", raw(&ScriptInterface::home_), this )
        ( "is_absolute", &ScriptInterface::is_absolute, this )
        ( "is_relative", &ScriptInterface::is_relative, this )
        ( "cd", &ScriptInterface::cd, this )
        ( "pushd", &ScriptInterface::pushd, this )
        ( "popd", &ScriptInterface::popd, this )
        ( "pwd", &ScriptInterface::pwd, this )
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
        ( "hostname", &ScriptInterface::hostname, this )
        ( "whoami", &ScriptInterface::whoami, this )
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
        ( "print_dependencies", &ScriptInterface::print_dependencies, this )
        ( "print_namespace", &ScriptInterface::print_namespace, this )
        ( "wait", &ScriptInterface::wait, this )
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

ptr<Rule> ScriptInterface::find_rule_by_id( const std::string& id ) const
{
    vector<ptr<Rule> >::const_iterator i = rules_.begin();
    while ( i != rules_.end() && (*i)->get_id() != id )
    {        
        ++i;
    }
    
    return i != rules_.end() ? *i : ptr<Rule>();
}

const std::vector<ptr<Rule> >& ScriptInterface::get_rules() const
{   
    return rules_;
}

void ScriptInterface::push_environment( ptr<Environment> environment )
{
    SWEET_ASSERT( environment );
    environments_.push_back( environment );
}

void ScriptInterface::pop_environment()
{
    SWEET_ASSERT( !environments_.empty() );
    if ( !environments_.empty() )
    {
        environments_.pop_back();
    }
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

void ScriptInterface::create_rule( ptr<Rule> rule )
{
    SWEET_ASSERT( rule );
    
    lua_.create( rule );
    lua_.members( rule )
        .type( SWEET_STATIC_TYPEID(Rule) )
        .metatable( rule_metatable_ )
        .this_pointer( rule.get() )
        ( lua::PTR_KEYWORD, value(rule) )
        ( "id", &Target::get_id )
        ( "path", &Target::get_path )
        ( "directory", &Target::get_directory )
        ( "parent", &Target::get_parent )
        ( "rule", &Target::get_rule )
        ( "set_bind_type", &Target::set_bind_type )
        ( "get_bind_type", &Target::get_bind_type )
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
        ( "get_working_directory", &Target::get_working_directory )
        ( "get_targets", &Target::get_targets )
        ( "add_dependency", &Target::add_dependency )
        ( "get_dependencies", &Target::get_dependencies )
    ;
    
    lua_.members( rule )
        ( "__index", rule )
    ;
}

void ScriptInterface::destroy_rule( Rule* rule )
{
    SWEET_ASSERT( rule );
    lua_.destroy( rule );
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
    lua_.create( target );
    recover_target( target );
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

void ScriptInterface::update_target( ptr<Target> target, ptr<Rule> rule )
{
    SWEET_ASSERT( target );
    if ( rule )
    {
        lua_.members( target )
            .metatable( rule )
        ;

        const char* INIT_FUNCTION = "init";
        if ( lua_.is_function(target, INIT_FUNCTION) )
        {
            Environment* environment = get_environment().get();
            SWEET_ASSERT( environment );
            environment->get_environment_thread().call( "init", target )
                ( target )
            .end();
        }
    }
    else
    {
        lua_.members( target )
            .metatable( target_metatable_ )
        ;
    }
    
    if ( target->get_filename().empty() )
    {
        target->set_filename( target->get_path() );
    }
}

void ScriptInterface::destroy_target( Target* target )
{
    SWEET_ASSERT( target );
    lua_.destroy( target );
}

ptr<Rule> ScriptInterface::rule( const std::string& id, int bind_type )
{
    if ( bind_type < BIND_PHONY || bind_type >= BIND_TYPE_COUNT )
    {
        SWEET_ERROR( InvalidBindTypeError("Invalid bind type (%d)", bind_type) );
    }

    ptr<Rule> rule;    

    vector<ptr<Rule> >::const_iterator i = rules_.begin(); 
    while ( i != rules_.end() && (*i)->get_id() != id )
    {
        ++i;
    }

    if ( i == rules_.end() )
    {
        rule.reset( new Rule(id, BindType(bind_type), build_tool_), Deleter<Rule>(&ScriptInterface::destroy_rule, this) );
        rules_.push_back( rule );
        create_rule( rule );
    }
    else
    {
        rule = *i;
    }

    return rule;
}

ptr<Target> ScriptInterface::target( const std::string& id, Graph* graph )
{
    ptr<Target> target( new Target(id, graph), Deleter<Target>(&ScriptInterface::destroy_target, this) );
    create_target( target );
    return target;
}

ptr<Target> ScriptInterface::target( const std::string& id, ptr<Rule> rule, Graph* graph )
{
    ptr<Target> target( new Target(id, graph), Deleter<Target>(&ScriptInterface::destroy_target, this) );
    create_target( target );
    return target;
}

ptr<Target> ScriptInterface::find_target( const std::string& id )
{
    Environment* environment = get_environment().get();
    SWEET_ASSERT( environment );
    return build_tool_->get_graph()->find_target( id, environment->get_working_directory() );
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
        path::Path absolute_path( environment->get_directory() );
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
        return environment->get_directory().relative( path::Path(path) ).string();
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
#if defined BUILD_PLATFORM_MSVC || defined BUILD_PLATFORM_MINGW
    const char* USERPROFILE = "USERPROFILE";
    const char* home = ::getenv( USERPROFILE );
    if ( !home )
    {
        SWEET_ERROR( EnvironmentVariableNotFoundError("The environment varaible '%s' could not be found", USERPROFILE) );
    }        
    
    if ( path::Path(path).is_absolute() )
    {
        return path;
    }

    path::Path absolute_path( home );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path.string();
#else
#error "ScriptInterface::home() is not implemented for this platform"
#endif
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
    return environment->get_directory().string();
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
    return native_path.file_string();
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
    return os_interface_->cpdir( absolute(from), absolute(to), environment->get_directory().branch() );
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

std::string ScriptInterface::hostname()
{
    return os_interface_->hostname();
}

std::string ScriptInterface::whoami()
{
    return os_interface_->whoami();
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

void ScriptInterface::print_dependencies( ptr<Target> target )
{
    SWEET_ASSERT( build_tool_ );
    return build_tool_->get_graph()->print_dependencies( target );
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

bool ScriptInterface::load_xml( const std::string& filename, const std::string& initial )
{
    SWEET_ASSERT( build_tool_ );
    SWEET_ASSERT( build_tool_->get_graph() );

    bool load = true;    
    if ( exists(filename) )
    {
        Graph* graph = build_tool_->get_graph();
        Target* cache_target = graph->load_xml( filename );
        if ( cache_target && !cache_target->is_outdated() )
        {
            ptr<Target> initial_target = graph->find_target( initial, ptr<Target>() );
            if ( !initial_target )
            {
                SWEET_ERROR( InitialTargetNotFoundError("No initial target found at '%s'", initial.c_str()) );
            }
            graph->bind( initial_target );
            load = false;
        }
    }

    if ( load )
    {
        Environment* environment = environments_.back().get();
        ptr<Target> working_directory = environment->get_working_directory();
        Graph* graph = build_tool_->get_graph();
        graph->clear( filename );
        working_directory = graph->target( working_directory->get_path() );
        environment->set_working_directory( working_directory );
    }

    return load;
}

void ScriptInterface::save_xml( const std::string& filename )
{
    SWEET_ASSERT( build_tool_ );
    build_tool_->get_graph()->save_xml( filename );
}

bool ScriptInterface::load_binary( const std::string& filename, const std::string& initial )
{
    SWEET_ASSERT( build_tool_ );
    SWEET_ASSERT( build_tool_->get_graph() );
    
    bool load = true;    
    if ( exists(filename) )
    {
        Graph* graph = build_tool_->get_graph();
        Target* cache_target = graph->load_binary( filename );
        if ( cache_target && !cache_target->is_outdated() )
        {
            ptr<Target> initial_target = graph->find_target( initial, ptr<Target>() );
            if ( !initial_target )
            {
                SWEET_ERROR( InitialTargetNotFoundError("No initial target found at '%s'", initial.c_str()) );
            }
            graph->bind( initial_target );
            load = false;
        }
    }

    if ( load )
    {
        Environment* environment = environments_.back().get();
        ptr<Target> working_directory = environment->get_working_directory();
        Graph* graph = build_tool_->get_graph();
        graph->clear( filename );
        working_directory = graph->target( working_directory->get_path() );
        environment->set_working_directory( working_directory );
    }

    return load;
}

void ScriptInterface::save_binary( const std::string& filename )
{
    SWEET_ASSERT( build_tool_ );
    build_tool_->get_graph()->save_binary( filename );
}

int ScriptInterface::absolute_( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const int PATH = 1;
    if ( lua_type(lua_state, PATH) != LUA_TSTRING )
    {
        SWEET_ERROR( lua::RuntimeError("The first parameter of 'absolute()' is not a string as expected") );
    }
    std::string path = lua_tostring( lua_state, PATH );
    
    const int WORKING_DIRECTORY = 2;
    path::Path working_directory;    
    if ( !lua_isnoneornil(lua_state, WORKING_DIRECTORY) )
    {
        if ( lua_type(lua_state, WORKING_DIRECTORY) != LUA_TSTRING )
        {
            SWEET_ERROR( lua::RuntimeError("The second parameter of 'absolute()' is not a string as expected") );
        }
        working_directory = path::Path( lua_tostring(lua_state, WORKING_DIRECTORY) );
    }
    
    ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( script_interface );
    std::string absolute_path = script_interface->absolute( path, working_directory );
    lua_pushlstring( lua_state, absolute_path.c_str(), absolute_path.length() );
    return 1;
}

int ScriptInterface::relative_( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const int PATH = 1;
    const int WORKING_DIRECTORY = 2;

    if ( lua_type(lua_state, PATH) != LUA_TSTRING )
    {
        SWEET_ERROR( lua::RuntimeError("The first parameter of 'relative()' is not a string as expected") );
    }
    std::string path = lua_tostring( lua_state, PATH );
    
    path::Path working_directory;    
    if ( !lua_isnoneornil(lua_state, WORKING_DIRECTORY) )
    {
        if ( lua_type(lua_state, WORKING_DIRECTORY) != LUA_TSTRING )
        {
            SWEET_ERROR( lua::RuntimeError("The second parameter of 'relative()' is not a string as expected") );
        }
        working_directory = path::Path( lua_tostring(lua_state, WORKING_DIRECTORY) );
    }
    
    ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( script_interface );
    std::string relative_path = script_interface->relative( path, working_directory );
    lua_pushlstring( lua_state, relative_path.c_str(), relative_path.length() );
    return 1;
}

int ScriptInterface::root_( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    std::string path;
    if ( lua_gettop(lua_state) > 0 && !lua_isnoneornil(lua_state, -1) )
    {
        path = lua_tostring( lua_state, -1 );
        lua_pop( lua_state, 1 );
    }
    
    ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( script_interface );
    std::string absolute_path = script_interface->root( path );
    lua_pushlstring( lua_state, absolute_path.c_str(), absolute_path.length() );
    return 1;
}

int ScriptInterface::initial_( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    std::string path;
    if ( lua_gettop(lua_state) > 0 && !lua_isnoneornil(lua_state, -1) )
    {
        path = lua_tostring( lua_state, -1 );
        lua_pop( lua_state, 1 );
    }
    
    ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( script_interface != NULL );
    std::string absolute_path = script_interface->initial( path );
    lua_pushlstring( lua_state, absolute_path.c_str(), absolute_path.length() );
    return 1;
}

int ScriptInterface::home_( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    std::string path;
    if ( lua_gettop(lua_state) > 0 && !lua_isnoneornil(lua_state, -1) )
    {
        path = lua_tostring( lua_state, -1 );
        lua_pop( lua_state, 1 );
    }
    
    ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( script_interface != NULL );
    std::string absolute_path = script_interface->home( path );
    lua_pushlstring( lua_state, absolute_path.c_str(), absolute_path.length() );
    return 1;
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

int ScriptInterface::target_from_graph( lua_State* lua_state )
{   
    SWEET_ASSERT( lua_state );
    
    try 
    {
        ScriptInterface* script_interface = reinterpret_cast<ScriptInterface*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( script_interface );

        Environment* environment = script_interface->get_environment().get();
        SWEET_ASSERT( environment );

        Graph* graph = script_interface->build_tool_->get_graph();
        SWEET_ASSERT( graph );

        ptr<Target> working_directory = environment->get_working_directory();
        SWEET_ASSERT( working_directory );

        const int RULE_PARAMETER = 1;
        ptr<Rule> rule = LuaConverter<ptr<Rule> >::to( lua_state, 1 );
        if ( !rule )
        {
            SWEET_ERROR( lua::RuntimeError("Rule is null when constructing a target") );
        }

        const int TARGET_OR_ID_PARAMETER = 2;
        if ( lua_istable(lua_state, TARGET_OR_ID_PARAMETER) )
        {        
            //
            // Target constructors that are passed a table argument are 
            // ignored if the graph has been loaded from cache.  This is 
            // because these targets are assumed to be creating targets 
            // during a buildfile load that will have already been loaded 
            // from the cache.
            //
            if ( !script_interface->build_tool_->get_graph()->is_loaded_from_cache() )
            {
                string id;
                lua_getfield( lua_state, TARGET_OR_ID_PARAMETER, "id" );
                if ( !lua_isnil(lua_state, -1) )
                {
                    if ( !lua_isstring(lua_state, -1) )
                    {
                        SWEET_ERROR( lua::RuntimeError("Target id is not a string when constructing a target") );
                    }
                    id = lua_tostring( lua_state, -1 );
                }
                lua_pop( lua_state, 1 );

                lua_pushstring( lua_state, "id" );
                lua_pushnil( lua_state );
                lua_rawset( lua_state, TARGET_OR_ID_PARAMETER );

                ptr<Target> target( new Target(id, graph) );
                graph->insert_target( target, environment->get_working_directory() );
                lua_pushvalue( lua_state, TARGET_OR_ID_PARAMETER );
                lua_create_object_with_existing_table( lua_state, target.get() );
                script_interface->recover_target( target );
                target->set_rule( rule );
                target->set_working_directory( working_directory );

                if ( !id.empty() )
                {
                    working_directory->add_dependency( target );
                }
            }
            else            
            {
                lua_pushnil( lua_state );
            }
        }
        else if ( lua_isstring(lua_state, TARGET_OR_ID_PARAMETER) )
        {
            string id = lua_tostring( lua_state, -1 );
            lua_pop( lua_state, 1 );
            ptr<Target> target = graph->target( id, rule, working_directory );
            LuaConverter<ptr<Target> >::push( lua_state, target );
        }
        else if ( lua_isnoneornil(lua_state, TARGET_OR_ID_PARAMETER) )
        {
            string id;
            ptr<Target> target = graph->target( id, rule, working_directory );
            LuaConverter<ptr<Target> >::push( lua_state, target );
        }

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
            SWEET_ERROR( lua::RuntimeError("Table not passed when creating a Scanner") );
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

            const int ARGUMENTS_PARAMETER = 3;
            ptr<Arguments> arguments;
            if ( lua_gettop(lua_state) >= ARGUMENTS_PARAMETER )
            {
                arguments.reset( new Arguments(script_interface->lua_, lua_state, ARGUMENTS_PARAMETER, lua_gettop(lua_state) + 1) );
            }

            target->bind();
            if ( target->get_last_write_time() == 0 )
            {
                SWEET_ERROR( ScannedFileNotFoundError("The scanned file '%s' was not found", target->get_filename().c_str()) );
            }
            
            if ( target->get_last_write_time() != target->get_last_scan_time() )
            {
                target->clear_dependencies();    
                target->set_last_scan_time( target->get_last_write_time() );
                ptr<Environment> environment = script_interface->get_environment();
                SWEET_ASSERT( environment );
                script_interface->build_tool_->get_scheduler()->scan( target, scanner, arguments, environment->get_working_directory(), environment );
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
