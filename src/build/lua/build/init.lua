
-- Provide python like syntax for string interpolation.
getmetatable("").__mod = function( format, args )
    if args then
        if type(args) == "table" then
            return string.format( format, unpack(args) );
        else
            return string.format( format, args );
        end
    else
        return format;
    end
end

build = {};

require "build/default_settings";
require "build/Generate";
require "build/PrecompiledHeader";
require "build/Directory";
require "build/Copy";
require "build/Cc";
require "build/StaticLibrary";
require "build/DynamicLibrary";
require "build/Executable";
require "build/CcScanner";
require "build/commands";

-- Perform per run initialization of the build system.
function build.initialize( project_settings )
    platform = platform or build.switch { operating_system(); windows = "msvc"; macosx = "llvmgcc" };
    variant = variant or "debug";
    version = version or "%s %s %s" % { os.date("%Y.%m.%d.%H%M"), platform, variant };
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
    assert( platform_settings, "The platform '%s' is not supported" % platform );
    build.merge_settings( settings, platform_settings );

    local variant_settings = settings.settings_by_variant[variant];
    assert( variant_settings, "The variant '%s' is not supported" % variant );
    build.merge_settings( settings, variant_settings );

    if settings.library_type == "static" then
        Library = StaticLibrary;
    elseif settings.library_type == "dynamic" then
        Library = DynamicLibrary;
    else
        error( string.format("The library type '%s' is not 'static' or 'dynamic'", tostring(settings.library_type)) );
    end

    build.default_settings.cache = root( "%s/%s_%s.cache" % {settings.obj, platform, variant} );
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
-- of a C/C++ source or header file, then scan it with the CcScanner.
function build.depend_visit( target )
    local fn = target.depend;
    if fn then
        fn( target );
    elseif target:prototype() == nil and string.find(target:id(), ".+%.[chi]p?p?") then
        scan( target, CcScanner );
    end
end

-- Determine whether or not to load or build based on whether or not a 
-- settings table has platforms and variants fields that match the current
-- platform(s) and/or variant(s).
--
-- @return
--  True if \e target should be built otherwise false.
function build.built_for_platform_and_variant( settings )
    function contains( value, values )
        for _, v in ipairs(values) do
            if v == value then
                return true;
            end
        end        
        return false;
    end
    return 
        (platform == "" or settings.platforms == nil or contains(platform, settings.platforms)) and 
        (variant == "" or settings.variants == nil or contains(variant, settings.variants))
    ;
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
        local result = execute( cmd, [[cmd /c "%s"]] % arguments, filter );
        if result ~= 0 then
            error( "[[%s]] failed (result=%d)" % {arguments, result} );
        end
    else
        local sh = "/bin/sh";
        local result = execute( sh, [[sh -c "%s"]] % arguments, filter );
        if result ~= 0 then
            error( "[[%s]] failed (result=%d)" % {arguments, tonumber(result)} );
        end
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
                file:write( "%s = " % k );
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
    assert( file, "Opening %s to write settings failed" % filename );
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

-- Load a target.
function build.load_target( target )
    local inherited = build.inherit_settings( target, build.settings );
    for _, value in ipairs(target) do
        if type(value) == "table" then
            build.inherit_settings( value, target.settings );
        end
    end
    return inherited;
end

-- Load the dependency graph from the file specified by /settings.cache/.
function build.load()
    assert( initialize and type(initialize) == "function", "The 'initialize' function is not defined" );
    assert( buildfiles and type(buildfiles) == "function", "The 'buildfiles' function is not defined" );

    if not build.initialized then
        initialize();
        build.initialized = true;
    end

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
        assert( cache_target, "No cache target found at '%s' after loading buildfiles" % settings.cache );
        local script = build.script;
        cache_target:add_dependency( file(root("build.lua")) );
        cache_target:add_dependency( file(root("local_settings.lua")) );
        cache_target:add_dependency( file(script("build/default_settings")) );
        cache_target:add_dependency( file(script("build/Generate")) );
        cache_target:add_dependency( file(script("build/PrecompiledHeader")) );
        cache_target:add_dependency( file(script("build/Directory")) );
        cache_target:add_dependency( file(script("build/Copy")) );
        cache_target:add_dependency( file(script("build/StaticLibrary")) );
        cache_target:add_dependency( file(script("build/DynamicLibrary")) );
        cache_target:add_dependency( file(script("build/Executable")) );
        cache_target:add_dependency( file(script("build/CcScanner")) );
        cache_target:add_dependency( file(script("build/commands")) );

        mark_implicit_dependencies();
    end

    local all = find_target( initial(goal) );
    assert( all, "No target found at '"..tostring(initial(goal)).."'" );
    preorder( build.depend_visit, all );
    bind( all );
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
    return nil;
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

build.level = 0;

function build.begin_target()
    local level = build.level + 1;
    build.level = level;
    return level;
end

function build.end_target( create )
    local level = build.level - 1;
    build.level = level;    

    if level == 0 then
        return build.expand_target( create );
    else
        return create;
    end
end

function build.expand_target( create, ... )
    return create( ... );
end

function build.add_unit_dependencies( definition, prototype )
    return function( architecture )
        local unit;
        local settings = build.push_settings( definition.settings );
        if build.built_for_platform_and_variant(settings) then
            unit = target( "", prototype, build.copy(definition) );
            unit.settings = settings;
            unit.architecture = architecture;

            for _, value in ipairs(unit) do
                local source_file = file( value );
                source_file:set_required_to_exist( true );
                source_file.unit = unit;
                source_file.settings = settings;

                local object = file( "%s/%s/%s" % {obj_directory(source_file), architecture, obj_name(value)} );
                object.source = value;
                object:add_dependency( source_file );
                object:add_dependency( Directory("%s/%s" % {obj_directory(source_file), architecture}) );
                unit:add_dependency( object );
            end
        end
        build.pop_settings();        
        return unit;
    end
end

function build.add_library_dependencies( target )
    local libraries = {};
    if platform ~= "" and target.libraries then
        for _, value in ipairs(target.libraries) do
            local library = find_target( root(module_name(value, target.architecture)) );
            assert( library, "Failed to find library '%s' for '%s'" % {value, relative(target:path(), root())} );
            if build.built_for_platform_and_variant(library.settings) then
                table.insert( libraries, library );
                target:add_dependency( library );
            end
        end
    end
    target.libraries = libraries;
end

function build.add_jar_dependencies( target )
    local jars = {};
    if platform ~= "" and target.jars then
        for _, value in ipairs(target.jars) do
            local jar = find_target( root("%s.jar" % value) );
            assert( jar, "Failed to find JAR '%s' for '%s'" % {value, relative(target:path(), root())} );
            if build.built_for_platform_and_variant(jar.settings) then
                table.insert( jars, jar );
                target:add_dependency( jar );
            end
        end
        target.jars = jars;
    end
end

function build.add_module_dependencies( target, filename, architecture, settings )
    if build.built_for_platform_and_variant(settings) then
        local working_directory = working_directory();
        working_directory:add_dependency( target );

        local root_target = find_target( root() );
        assert( root_target , "No root target found at '%s'" % tostring(root()) );
        root_target:add_dependency( target );

        target.settings = settings;
        target.architecture = architecture;
        target:set_filename( filename );
        target:add_dependency( Directory(branch(filename)) );
        if target:prototype() == DynamicLibraryPrototype and operating_system() == "windows" then 
            target:add_dependency( Directory(target.settings.lib) );
        end

        for _, dependency in ipairs(target) do 
            if type(dependency) == "function" then
                dependency = build.expand_target( dependency, architecture );
            end
            if dependency then
                target:add_dependency( dependency );
                dependency.module = target;
            end
        end
    end
end

function build.add_package_dependencies( target, filename, dependencies )
    if build.built_for_platform_and_variant(target) then
        local working_directory = working_directory();
        working_directory:add_dependency( target );

        local root_target = find_target( root() );
        assert( root_target , "No root target found at '%s'" % tostring(root()) );
        root_target:add_dependency( target );

        target.settings = build.current_settings();
        target:set_filename( filename );
        target:add_dependency( Directory(branch(filename)) );

        for _, dependency in ipairs(dependencies) do 
            if type(dependency) == "function" then
                dependency = build.expand_target( dependency );
            end
            if dependency then
                for _, dependency_target in ipairs(dependency) do 
                    working_directory:remove_dependency( dependency_target );
                    target:add_dependency( dependency_target );
                    dependency_target.module = target;
                end
            end
        end
    end
end

function build.expand( values )
    if type(values) == "string" then
        local result = {};
        for value in string.gmatch(values, "[_/%.%w]+") do 
            table.insert( result, value );
        end
        return result;
    else
        return values;
    end
end

function build.append( values, value )
    if type(value) == "table" then 
        for _, other_value in ipairs(value) do 
            table.insert( values, other_value );
        end
    else
        table.insert( values, value );
    end
end

function build.copy( value )
    local copied_table = {};
    for k, v in pairs(value) do 
        copied_table[k] = v;
    end
    return copied_table;
end

function build.Pushd( directory )
    pushd( directory );
    return function()
        pushd( directory );
        return nil;
    end
end

function build.Popd()
    popd();
    return function()
        popd();
        return nil;
    end
end

function build.obj_directory( target )
    return "%s/%s_%s/%s" % { target.settings.obj, platform, variant, relative(target:get_working_directory():path(), root()) };
end;

function build.cc_name( name )
    return "%s.c" % basename( name );
end;

function build.cxx_name( name )
    return "%s.cpp" % basename( name );
end;

function build.obj_name( name, architecture )
    return "%s.o" % basename( name );
end;

function build.lib_name( name, architecture )
    return "lib%s_%s_%s.a" % { name, architecture, variant };
end;

function build.dll_name( name )
    return "%s.dylib" % { name };
end;

function build.exe_name( name, architecture )
    return "%s_%s" % { name, architecture };
end;

function build.module_name( name, architecture )
    return "%s_%s" % { name, architecture };
end

obj_directory = build.obj_directory;
cc_name = build.cc_name;
cxx_name = build.cxx_name;
obj_name = build.obj_name;
lib_name = build.lib_name;
dll_name = build.dll_name;
exe_name = build.exe_name;
module_name = build.module_name;
