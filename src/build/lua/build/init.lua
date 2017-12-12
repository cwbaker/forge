
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

build = {};

function build.default_create_function( target_prototype, ... )
    local create = target_prototype.create;
    if create then 
        return create( target_prototype, ... );
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
    getmetatable( target_prototype ).__call = build.default_create_function;
    return target_prototype;
end

function build.Target( id, target_prototype, definition )
    local target_ = target( id, target_prototype, definition );
    getmetatable( target_ ).__call = build.default_call_function;
    return target_;
end

function build.File( filename, target_prototype, definition )
    local target_ = file( filename, target_prototype, definition );
    getmetatable( target_ ).__call = build.default_call_function;
    return target_;
end

function build.SourceFile( value, settings )
    local target = value;
    if type(target) == "string" then 
        target = file( build.interpolate(value, settings) );
        target:set_required_to_exist( true );
    end
    return target;
end

-- Perform per run initialization of the build system.
function build.initialize( project_settings )
    platform = platform or build.switch { operating_system(); windows = "msvc"; macosx = "clang" };
    variant = variant or "debug";
    version = version or ("%s %s %s"):format( os.date("%Y.%m.%d.%H%M"), platform, variant );
    environment = environment or "dev";
    goal = goal or "";
    jobs = jobs or 4;

    set_maximum_parallel_jobs( jobs );

    -- Set default settings (all other settings inherit from this table).
    local default_settings = build.default_settings;
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
        Library = StaticLibrary;
    elseif settings.library_type == "dynamic" then
        Library = DynamicLibrary;
    else
        error( string.format("The library type '%s' is not 'static' or 'dynamic'", tostring(settings.library_type)) );
    end

    build.default_settings.cache = root( ("%s/%s_%s.cache"):format(settings.obj, platform, variant) );
    _G.settings = settings;
    build.default_settings = default_settings;
    build.local_settings = local_settings;
    build.settings = settings;
    return settings;
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

-- Convert a version string into the number of half days passed since the 
-- reference time of 2013/01/01 00:00 (assuming that the version string is of
-- the form '%Y.%m.%d.%H%M').
function build.version_code( version )
    local SECONDS_PER_HALF_DAY = 12 * 60 * 60;
    local reference_time = os.time( {year = 2013; month = 1; day = 1; hour = 0; min = 0; sec = 0} );
    local version_time = build.version_time( version );
    return math.ceil( os.difftime(version_time, reference_time) / SECONDS_PER_HALF_DAY );
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

-- Visit a target by calling a member function "depend" if it has one or,
-- if there is no "depend" function and the id of the target matches that
-- of a C, C++, Objective C, or Objective C++ source or header file, then 
-- scan it with the CcScanner.
function build.depend_visit( target )
    local fn = target.depend;
    if fn then
        fn( target );
    elseif target:prototype() == nil then
        local id = target:id();
        if id:find(".+%.[chi]p?p?") or id:find(".+%.mm?") then
            scan( target, CcScanner );
        end
    end
end

-- Execute command with arguments and optional filter and raise an error if 
-- it doesn't return 0.
function build.system( command, arguments, filter )
    if execute(command, arguments, filter) ~= 0 then       
        error( arguments.." failed", 0 );
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
            print( "  prototype="..tostring(getmetatable(t).__index) );
        end
        for k, v in pairs(t) do
            print( "  "..tostring(k).." -> "..tostring(v) );
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
function build.load( goal )
    assert( initialize and type(initialize) == "function", "The 'initialize' function is not defined" );
    assert( buildfiles and type(buildfiles) == "function", "The 'buildfiles' function is not defined" );

    local initialize_start = ticks();
    if not build.initialized then
        initialize();
        build.initialized = true;
    end
    local initialize_finish = ticks();

    local load_start = initialize_finish;
    local cache_target = load_binary( settings.cache, initial(goal) );
    if cache_target == nil or cache_target:is_outdated() or build.local_settings.updated then
        clear();
        build.push_settings( build.settings );
        buildfiles();
        build.pop_settings();

        local root_target = find_target( root() );
        assert( root_target , "No root target found at '"..tostring(root()).."'" );
        preorder( build.visit("static_depend"), root_target );

        cache_target = find_target( settings.cache );
        assertf( cache_target, "No cache target found at '%s' after loading buildfiles", settings.cache );
        local script = build.script;
        cache_target:add_dependency( file(root("build.lua")) );
        cache_target:add_dependency( file(root("local_settings.lua")) );
        cache_target:add_dependency( file(script("build/default_settings")) );
        cache_target:add_dependency( file(script("build/commands")) );
        cache_target:add_dependency( file(script("build/Generate")) );
        cache_target:add_dependency( file(script("build/Directory")) );
        cache_target:add_dependency( file(script("build/Copy")) );

        mark_implicit_dependencies();
    end
    local load_finish = ticks();

    local depend_start = load_finish;
    local all = find_target( initial(goal) );
    assert( all, ("No target found at '%s'"):format(tostring(initial(goal))) );
    preorder( build.depend_visit, all );
    local depend_finish = ticks();

    local bind_start = depend_finish;
    bind( all );
    local bind_finish = ticks();

    return 
        initialize_finish - initialize_start, 
        load_finish - load_start, 
        depend_finish - depend_start, 
        bind_finish - bind_start
    ;
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
        return string.format( "%s/%s", branch, basename(path) );
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
    if settings then
        if #settings_stack > 0 then
            setmetatable( settings, {__index = settings_stack[#settings_stack]} );
        end
    else
        assert( #settings_stack > 0 );
        settings = settings_stack[#settings_stack];
    end
    table.insert( settings_stack, settings );
    return settings;
end

function build.pop_settings( settings )
    table.remove( build.settings_stack );
end

function build.current_settings( settings )
    local settings_stack = build.settings_stack;
    if #settings_stack > 0 then
        if settings then 
            setmetatable( settings, {__index = settings_stack[#settings_stack]} );
        else
            settings = settings_stack[#settings_stack];
        end
    end
    return settings;
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
    return string.format( "%s/%s", target.settings.gen, relative(target:get_working_directory():path(), root()) );
end

function build.classes_directory( target )
    return string.format( "%s/%s", target.settings.classes, relative(target:get_working_directory():path(), root()) );
end

function build.obj_directory( target )
    return ("%s/%s_%s/%s"):format( target.settings.obj, platform, variant, relative(target:get_working_directory():path(), root()) );
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

require "build.default_settings";
require "build.commands";
require "build.Generate";
require "build.Directory";
require "build.Copy";
