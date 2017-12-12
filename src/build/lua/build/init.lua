
-- Provide printf().
function printf( format, ... ) 
    print( string.format(format, ...) );
end

-- Provide formatted assert().
function assertf( condition, format, ... )
    if not condition then 
        assert( condition, string.format(format, ...) );
    end
end

-- Provide formatted error().
function errorf( format, ... )
    error( string.format(format, ...) );
end

-- Split a '.' delimited string into a table hierarchy returning the last 
-- level table and identifier.
function split_modules( module, qualified_id )
    local start = qualified_id:find( ".", 1, true );
    if start then
        local id = qualified_id:sub( 1, start - 1 );
        local submodule = module[id];
        if submodule == nil then 
            submodule = {};
            module[id] = submodule;
        end
        local remaining = qualified_id:sub( start + 1 );
        return split_modules( submodule, remaining );
    end
    return module, qualified_id;
end

-- Provide buildfile() that restores the settings stack position.
local original_buildfile = buildfile;
function buildfile( ... )
    local position = build.store_settings();
    pcall( original_buildfile, ... );
    build.restore_settings( position );
end

build = {};

function build.matches( value, ... )
    for i = 1, select("#", ...) do 
        if string.match(value, select(i, ...)) then
            return true;
        end
    end
    return false;
end

function build.platform_matches( ... )
    return platform == "" or build.matches( platform, ... );
end

function build.variant_matches( ... )
    return variant == "" or build.matches( variant, ... );
end

function build.default_create_function( target_prototype, ... )
    local create = target_prototype.create;
    if create then 
        local settings = build.current_settings();
        return create( settings, ... );
    else
        local id = select( 1, ... );
        local definition = select( 2, ... );
        return target( id, target_prototype, definition );
    end
end

function build.default_call_function( target, ... )
    local call = target.call;
    if call then 
        call( target, ... );
    else
        local definition = select( 1, ... );
        for _, value in ipairs(definition) do 
            if type(value) == "string" then
                target:add_dependency( file(value) );
            elseif type(value) == "table" then
                target:add_dependency( value );
            end
        end
    end
    return target;
end

function build.TargetPrototype( id, create_function )
    local target_prototype = TargetPrototype { id };
    getmetatable( target_prototype ).__call = create_function or build.default_create_function;
    local module, id = split_modules( build, id );
    module[id] = target_prototype;
    return target_prototype;
end

function build.Target( id, target_prototype, definition )
    local target_ = target( id, target_prototype, definition );
    getmetatable( target_ ).__call = build.default_call_function;
    return target_;
end

function build.File( filename, target_prototype, definition )
    local target_ = file( filename, target_prototype, definition );
    target_:set_cleanable( true );
    getmetatable( target_ ).__call = build.default_call_function;
    return target_;
end

function build.SourceFile( value, settings )
    local target = value;
    if type(target) == "string" then 
        settings = settings or build.current_settings();
        target = file( build.interpolate(value, settings) );
        target:set_required_to_exist( true );
        target:set_cleanable( false );
    end
    return target;
end

function build.SourceDirectory( value, settings )
    local target = build.SourceFile( value, settings );
    target:set_always_bind( true );
    return target;
end

-- Perform per run initialization of the build system.
function build.initialize( project_settings )
    -- Set default values for variables that can be passed on the command line.
    platform = platform or operating_system();
    variant = variant or "debug";
    version = version or ("%s %s %s"):format( os.date("%Y.%m.%d.%H%M"), platform, variant );
    goal = goal or "";
    jobs = jobs or 4;
    toolset = toolset or build.switch { 
        platform; 
        android = "gcc"; 
        ios = "clang"; 
        macosx = "clang";
        windows = "msvc"; 
    };

    set_maximum_parallel_jobs( jobs );
    if operating_system() == "windows" then 
        set_build_hooks_library( executable("build_hooks.dll") );
    elseif operating_system() == "macosx" then
        set_build_hooks_library( executable("build_hooks.dylib") );
    end    

    -- Set default settings (all other settings inherit from this table).
    local default_settings = dofile( build.script("build/default_settings") );
    build.merge_settings( default_settings, project_settings );

    local local_settings = {};
    setmetatable( local_settings, {__index = default_settings} );

    local user_settings_filename = default_settings.user_settings_filename;
    if exists(user_settings_filename) then
        build.merge_settings( local_settings, dofile(user_settings_filename) );
    end

    local local_settings_filename = default_settings.local_settings_filename;
    if exists(local_settings_filename) then
        build.merge_settings( local_settings, dofile(local_settings_filename) );
    end

    local settings = {};
    setmetatable( settings, {__index = local_settings} );

    local platform_settings = settings.settings_by_platform[platform];
    assertf( platform_settings, "The platform '%s' is not supported", platform );
    build.merge_settings( settings, platform_settings );

    local variant_settings = settings.settings_by_variant[variant];
    assertf( variant_settings, "The variant '%s' is not supported", variant );
    build.merge_settings( settings, variant_settings );

    if settings.library_type == "static" then
        build.Library = build.StaticLibrary;
    elseif settings.library_type == "dynamic" then
        build.Library = build.DynamicLibrary;
    else
        error( string.format("The library type '%s' is not 'static' or 'dynamic'", tostring(settings.library_type)) );
    end

    default_settings.cache = root( ("%s/%s_%s.cache"):format(settings.obj, platform, variant) );
    _G.settings = settings;
    build.default_buildfiles_ = {};
    build.default_targets_ = {};
    build.default_settings = default_settings;
    build.local_settings = local_settings;
    build.settings = settings;
    build.configure_modules( settings );
    build.initialize_modules( settings );
    return settings;
end

build.modules = {};

-- Register *module* to be configured and initialized when the build sysetm 
-- is initialized.
function build.register_module( module )
    table.insert( build.modules, module ); 
end

-- Call `configure` for each registered module that provides it.
function build.configure_modules( settings )
    local modules = build.modules;
    for _, module in ipairs(modules) do 
        local configure = module.configure;
        if configure and type(configure) == "function" then 
            configure( settings );
        end
    end
end

-- Call `initialize` for each registered module that provides it.
function build.initialize_modules( settings )
    local modules = build.modules;
    for _, module in ipairs(modules) do 
        local initialize = module.initialize;
        if initialize and type(initialize) == "function" then 
            initialize( settings );
        end
    end
end

-- Convert a version string into a date table (assuming that the version 
-- string is of the form '%Y.%m.%d.%H%M').
function build.version_date( version )
    version = version or _G.version;
    local _, _, year, month, day, hour, minute = string.find( version, "(%d%d%d%d)%.(%d%d)%.(%d%d)%.(%d%d)(%d%d)" );
    return {
        year = year;
        month = month;
        day = day;
        hour = hour;
        min = min;
        sec = 0;
    };
end

-- Convert a version string into a time (assuming that the version string is
-- of the form '%Y.%m.%d.%H%M').
function build.version_time( version )
    return os.time( build.version_date(version) );
end

-- Convert a version string into the number of half days passed since 
-- *reference_time* or since 2012/01/01 00:00 if *reference_time* is not 
-- provided (assuming that the version string is of the form '%Y.%m.%d.%H%M').
function build.version_code( version, reference_time )
    reference_time = reference_time or os.time( {year = 2012; month = 1; day = 1; hour = 0; min = 0; sec = 0} );
    local SECONDS_PER_HALF_DAY = 12 * 60 * 60;
    local version_time = build.version_time( version );
    return math.ceil( os.difftime(version_time, reference_time) / SECONDS_PER_HALF_DAY );
end

-- Add a target to the current directory's target so that it will be built 
-- when a build is invoked from that directory.
function build.default_target( target )
    local directory = working_directory();
    directory:add_dependency( target );
end

-- Add targets to the current directory's target so that they will be built 
-- when a build is invoked from that directory.
function build.default_targets( targets )
    for _, target in ipairs(targets) do 
        local directory = working_directory();
        table.insert( build.default_targets_, {directory:path(), absolute(target)} );
    end
end

-- Set the root buildfile to be load to populate the dependency graph.
function build.default_buildfiles( buildfiles )
    build.default_buildfiles_ = buildfiles;
end

-- Visit a target by calling a member function /pass/ if it has one.
function build.visit( pass, ... )
    local args = {...};
    return function( target )
        local fn = target[pass];
        if fn then
            fn( target, unpack(args) );
        end
    end
end

-- Visit a target by calling a member function "clean" if it exists or if
-- there is no "clean" function and the target is not marked as a source file
-- that must exist then its associated file is deleted.
function build.clean_visit( target )
    local fn = target.clean;
    if fn then 
        fn( target );
    elseif target:cleanable() and target:filename() ~= "" then 
        rm( target:filename() );
    end
end

-- Execute command with arguments and optional filter and raise an error if 
-- it doesn't return 0.
function build.system( command, arguments, environment, dependencies_filter, stdout_filter, stderr_filter, ... )
    if type(arguments) == "table" then
        arguments = table.concat( arguments, " " );
    end
    if execute(command, arguments, environment, dependencies_filter, stdout_filter, stderr_filter, ...) ~= 0 then       
        error( ("%s failed"):format(arguments), 0 );
    end
end

-- Execute a command through the host system's native shell - either 
-- "C:/windows/system32/cmd.exe" on Windows system or "/bin/sh" anywhere else.
function build.shell( arguments, filter )
    if operating_system() == "windows" then
        local cmd = "C:/windows/system32/cmd.exe";
        local result = execute( cmd, ('cmd /c "%s"'):format(arguments), filter );
        assertf( result == 0, "[[%s]] failed (result=%d)", arguments, result );
    else
        local sh = "/bin/sh";
        local result = execute( sh, ('sh -c "%s"'):format(arguments), filter );
        assertf( result == 0, "[[%s]] failed (result=%d)", arguments, tonumber(result) );
    end
end

-- Return a value from a table using the first key as a lookup.
function build.switch( values )
    assert( values[1] ~= nil, "No value passed to `build.switch()`" );
    return values[values[1]];
end

-- Provide shell like string interpolation.
function build.interpolate( template, variables )
    return (template:gsub('($%b{})', function(word) return variables[word:sub(3, -2)] or word end));
end

-- Dump the keys, values, and prototype of a table for debugging.
function build.dump( t )
    print( tostring(t) );
    if t ~= nil then
        if getmetatable(t) ~= nil then
            printf( "  prototype=%s", tostring(getmetatable(t).__index) );
        end
        for k, v in pairs(t) do
            printf( "  %s -> %s", tostring(k), tostring(v) );
        end
    end
end

-- Serialize values to to a Lua file (typically the local settings table).
function build.serialize( file, value, level )
    local function indent( level )
        for i = 1, level do
            file:write( "  " );
        end
    end

    if type(value) == "boolean" then
        file:write( tostring(value) );
    elseif type(value) == "number" then
        file:write( value );
    elseif type(value) == "string" then
        file:write( string.format("%q", value) );
    elseif type(value) == "table" then
        file:write( "{\n" );
        for _, v in ipairs(value) do
            indent( level + 1 );
            build.serialize( file, v, level + 1 );
            file:write( ",\n" );
        end
        for k, v in pairs(value) do
            if type(k) == "string" then
                indent( level + 1 );
                file:write( ("%s = "):format(k) );
                build.serialize( file, v, level + 1 );
                file:write( ";\n" );
            end
        end
        indent( level );
        file:write( "}" );
    end
end

-- Save a settings table to a file.
function build.save_settings( settings, filename )
    local file = io.open( filename, "wb" );
    assertf( file, "Opening %s to write settings failed", filename );
    file:write( "\nreturn " );
    build.serialize( file, settings, 0 );
    file:write( "\n" );
    file:close();
end

-- Merge settings from /source_settings/ into /settings/.
function build.merge_settings( settings, source_settings )
    settings = settings or {};
    for _, v in ipairs(source_settings) do
        table.insert( settings, v );
    end
    for k, v in pairs(source_settings) do
        if type(k) == "string" then
            if type(v) == "table" then
                settings[k] = build.merge_settings( settings[k], v );
            else
                settings[k] = v;
            end
        end
    end
    return settings;
end

-- Inherit settings from /settings/ to /target/.
function build.inherit_settings( target, settings )
    local inherited = false;
    if target.settings then
        if not getmetatable(target.settings) then
            setmetatable( target.settings, {__index = settings} );
            inherited = true;
        end
    else
        target.settings = settings;
        inherited = true;
    end
    return inherited;
end

-- Merge fields from /source/ to /destination/.
function build.merge( destination, source )
    for k, v in pairs(source) do
        if type(k) == "string" then
            if type(v) == "table" then
                destination[k] = build.append( destination[k], v );
            else
                destination[k] = v;
            end
        end
    end
    return destination;
end

-- Load the dependency graph from the file specified by /settings.cache/ and
-- running depend and bind passes over the target specified by /goal/ and its
-- dependencies.
function build.load( force )
    local load_start = ticks();
    local cache_target = load_binary( settings.cache );
    if cache_target == nil or cache_target:outdated() or build.local_settings.updated or force then
        clear();
        build.push_settings( build.settings );
        for _, filename in ipairs(build.default_buildfiles_) do
            buildfile( filename );
        end
        build.pop_settings();

        cache_target = find_target( settings.cache );
        assertf( cache_target, "No cache target found at '%s' after loading buildfiles", settings.cache );
        local script = build.script;
        cache_target:add_dependency( file(root("build.lua")) );
        cache_target:add_dependency( file(script("build/default_settings")) );
        cache_target:add_dependency( file(script("build/commands")) );
        cache_target:add_dependency( file(script("build/Generate")) );
        cache_target:add_dependency( file(script("build/Directory")) );
        cache_target:add_dependency( file(script("build/Copy")) );

        -- Add default targets as dependencies of the working directory that
        -- was in effect when `build.default_target(s)` was called.
        for _, default_target in ipairs(build.default_targets_) do
            local directory = build.Target( default_target[1] );
            local target = build.Target( default_target[2] );
            directory:add_dependency( target );
        end
    end
    local load_finish = ticks();
    return load_finish - load_start;
end

-- Save the dependency graph to the file specified by /settings.cache/.
function build.save()
    if build.local_settings.updated then
        build.local_settings.updated = nil;
        build.save_settings( build.local_settings, build.settings.local_settings_filename );
    end
    mkdir( branch(settings.cache) );
    save_binary( settings.cache );
end

-- Convert /name/ into a path relative to the first pattern in package.paths
-- that expands to an existing file.
function build.script( name )
    for path in string.gmatch(package.path, "([^;]*);?") do
        local filename = string.gsub( path, "?", name );
        if exists(filename) then
            return filename;
        end
    end
    errorf( "Script '%s' not found", filename );
end

-- Strip the extension from a path (e.g. "foo.txt" -> "foo" and "bar/foo.txt"
-- -> "bar/foo".
function build.strip( path )
    local branch = branch( path );
    if branch ~= "" then 
        return ("%s/%s"):format( branch, basename(path) );
    else
        return basename( path );
    end
end

-- Return a string for a specific indentation level.
function build.indent( level )
    return string.rep("  ", level );
end

build.settings_stack = {};

function build.push_settings( settings )
    local settings_stack = build.settings_stack;
    local back = #settings_stack;
    if settings then
        if back > 0 then
            setmetatable( settings, {__index = settings_stack[back]} );
        end
    else
        assert( back > 0 );
        settings = settings_stack[back];
    end
    table.insert( settings_stack, settings );
    return settings;
end

function build.pop_settings()
    local settings_stack = build.settings_stack;
    assert( #settings_stack > 0 );
    table.remove( settings_stack );
end

function build.current_settings()
    local settings_stack = build.settings_stack;
    local back = #settings_stack;
    if back > 0 then
        return settings_stack[back];
    else 
        return build.settings;
    end
end

function build.store_settings()
    return #build.settings_stack;
end

function build.restore_settings( position )
    local settings_stack = build.settings_stack;
    local top_position = #settings_stack;
    while top_position > position do
        table.remove( settings_stack, top_position );
        top_position = top_position - 1;
    end 
end

function build.add_library_dependencies( executable, libraries )
    if libraries and platform ~= "" then
        local architecture = executable.architecture;
        for _, value in ipairs(libraries) do
            local library = ("%s_%s"):format( value, architecture );
            executable:add_dependency( target(root(library)) );
        end
    end
end

function build.add_jar_dependencies( jar, jars )
    if jars and platform ~= "" then
        for _, value in ipairs(jars) do
            jar:add_dependency( target(root(value)) );
        end
    end
end

-- Add dependencies detected by the injected build hooks library to the 
-- target /target/.
function build.dependencies_filter( target )
    target:clear_implicit_dependencies();
    return function( line )
        if line:match("^==") then 
            local READ_PATTERN = "^== read '([^']*)'";
            local filename = line:match( READ_PATTERN );
            if filename then
                local within_source_tree = relative( absolute(filename), root() ):find( "..", 1, true ) == nil;
                if within_source_tree then 
                    local header = build.SourceFile( filename );
                    target:add_implicit_dependency( header );
                end
            end
        else
            print( line );
        end
    end
end

-- Append values from /value/ to /values/.
function build.append( values, value )
    values = values or {};
    if type(value) == "table" then 
        for _, other_value in ipairs(value) do 
            table.insert( values, other_value );
        end
    else
        table.insert( values, value );
    end
    return values;
end

function build.copy( value )
    local copied_table = {};
    for k, v in pairs(value) do 
        copied_table[k] = v;
    end
    return copied_table;
end

function build.gen_directory( target )
    return string.format( "%s/%s", target.settings.gen, relative(target:working_directory():path(), root()) );
end

function build.classes_directory( target )
    return string.format( "%s/%s", target.settings.classes, relative(target:working_directory():path(), root()) );
end

function build.obj_directory( target )
    return ("%s/%s_%s/%s"):format( target.settings.obj, platform, variant, relative(target:working_directory():path(), root()) );
end;

function build.cc_name( name )
    return ("%s.c"):format( basename(name) );
end;

function build.cxx_name( name )
    return ("%s.cpp"):format( basename(name) );
end;

function build.obj_name( name, architecture )
    return ("%s.o"):format( basename(name) );
end;

function build.lib_name( name, architecture )
    return ("lib%s_%s.a"):format( name, architecture );
end;

function build.dll_name( name )
    return ("%s.dylib"):format( name );
end;

function build.exe_name( name, architecture )
    return ("%s_%s"):format( name, architecture );
end;

function build.module_name( name, architecture )
    return ("%s_%s"):format( name, architecture );
end

obj_directory = build.obj_directory;
cc_name = build.cc_name;
cxx_name = build.cxx_name;
obj_name = build.obj_name;
lib_name = build.lib_name;
dll_name = build.dll_name;
exe_name = build.exe_name;
module_name = build.module_name;

require "build.commands";
require "build.Generate";
require "build.Directory";
require "build.Copy";
