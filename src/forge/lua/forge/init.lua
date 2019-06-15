
forge = _G.forge or {};

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

-- Provide global default command that calls through to `build()`.
function default()
    return build();
end

-- Visit a target by calling a member function "clean" if it exists or if
-- there is no "clean" function and the target is not marked as a source file
-- that must exist then its associated file is deleted.
local function clean_visit( target )
    local clean_function = target.clean;
    if clean_function then 
        clean_function( target.forge, target );
    elseif target:cleanable() then 
        for _, filename in target:filenames() do 
            if filename ~= '' then
                rm( filename );
            end
        end
        target:clear_filenames();
        target:set_built( false );
    end
end

-- Visit a target by calling a member function "build" if it exists and 
-- setting that Target's built flag to true if the function returns with
-- no errors.
local function build_visit( target )
    if target:outdated() then
        local build_function = target.build;
        if build_function then 
            local success, error_message = pcall( build_function, target.forge, target );
            target:set_built( success );
            if not success then 
                clean_visit( target );
                assert( success, error_message );
            end
        else
            target:set_built( true );
        end
    end
end

-- Provide global build command.
function build()
    local failures = postorder( find_initial_target(goal), build_visit );
    forge:save();
    printf( "forge: default (build)=%dms", math.ceil(ticks()) );
    return failures;
end

-- Provide global clean command.
function clean()
    local failures = postorder( find_initial_target(goal), clean_visit );
    forge:save();
    printf( "forge: clean=%sms", tostring(math.ceil(ticks())) );
    return failures;
end

-- Provide global reconfigure command.
function reconfigure()
    rm( root('local_settings.lua') );
    return 0;
end

-- Provide global dependencies command.
function dependencies()
    print_dependencies( find_initial_target(goal) );
    return 0;
end

-- Provide global namespace command.
function namespace()
    print_namespace( find_initial_target(goal) );
    return 0;
end

-- Provide global help command.
function help()
    printf [[
Variables:
  goal               Target to build (relative to current working directory).
  variant            Variant built (debug, release, or shipping).
Commands:
  build              Build outdated targets.
  clean              Clean all targets.
  reconfigure        Regenerate per-machine configuration settings.
  dependencies       Print targets by dependency hierarchy.
  namespace          Print targets by namespace hierarchy.
    ]];
end

local toolsets_ = {};

function add_toolset( identifier, toolset )
    table.insert( toolsets_, {toolset:interpolate(identifier), toolset} );
    return toolset;
end

function toolsets( pattern )
    return coroutine.wrap( function() 
        local index = 1;
        for _, identifier_toolset in ipairs(toolsets_) do 
            local identifier = identifier_toolset[1];
            if pattern == nil or pattern == '' or identifier:find(pattern) then 
                coroutine.yield( index, identifier_toolset[2] );
                index = index + 1;
            end
        end        
    end );
end

-- Execute command with arguments and optional filter and raise an error if 
-- it doesn't return 0.
function system( command, arguments, environment, dependencies_filter, stdout_filter, stderr_filter, ... )
    if type(arguments) == 'table' then
        arguments = table.concat( arguments, ' ' );
    end
    if execute(command, arguments, environment, dependencies_filter, stdout_filter, stderr_filter, ...) ~= 0 then       
        error( ('%s failed'):format(arguments), 0 );
    end
end

-- Execute a command through the host system's native shell - either 
-- "C:/windows/system32/cmd.exe" on Windows system or "/bin/sh" anywhere else.
function shell( arguments, dependencies_filter, stdout_filter, stderr_filter, ... )
    if type(arguments) == 'table' then 
        arguments = table.concat( arguments, ' ' );
    end
    if operating_system() == 'windows' then
        local cmd = 'C:/windows/system32/cmd.exe';
        local result = execute( cmd, ('cmd /c "%s"'):format(arguments), dependencies_filter, stdout_filter, stderr_filter, ... );
        assertf( result == 0, '[[%s]] failed (result=%d)', arguments, result );
    else
        local sh = '/bin/sh';
        local result = execute( sh, ('sh -c "%s"'):format(arguments), dependencies_filter, stdout_filter, stderr_filter, ... );
        assertf( result == 0, '[[%s]] failed (result=%d)', arguments, tonumber(result) );
    end
end

-- Return a value from a table using the first key as a lookup.
function switch( values )
    assert( values[1] ~= nil, "No value passed to `switch()`" );
    return values[values[1]];
end

-- Express *path* relative to the root directory.
function root_relative( path )
    return relative( absolute(path), root() );
end

-- Find first existing file named *filename* in *paths*.
--
-- Searching is not performed when *filename* is an absolute path.  In this 
-- case *filename* is returned immediately only if it names an existing file.
--
-- The *paths* variable can be a string containing a `:` or `;` delimited list
-- of paths or a table containing those paths.  If *paths* is nil then its 
-- default value is set to that returned by `os.getenv('PATH')`.
--
-- Returns the first file named *filename* that exists at a directory listed
-- in *paths* or nothing if no existing file is found.
function which( filename, paths )
    local paths = paths or os.getenv( 'PATH' );
    local separator_pattern = '[^:]+';
    if operating_system() == 'windows' then 
        separator_pattern = '[^;]+';
        if extension(filename) == '' then
            filename = ('%s.exe'):format( filename );
        end
    end
    if type(paths) == 'string' then
        if is_absolute(filename) then
            if exists(filename) then 
                return filename;
            end
        else
            for directory in paths:gmatch(separator_pattern) do 
                local path = ('%s/%s'):format( directory, filename );
                if exists(path) then 
                    return path;
                end
            end
        end
    elseif type(paths) == 'table' then
        for _, directory in ipairs(paths) do 
            local path = ('%s/%s'):format( directory, filename );
            if exists(path) then 
                return path;
            end
        end
    end
end

-- Find and return the initial target to forge.
-- 
-- If *goal* is nil or empty then the initial target is the first all target
-- that is found in a search up from the current working directory to the
-- root directory.
--
-- Otherwise if *goal* is specified then the target that matches *goal* 
-- exactly and has at least one dependency or the target that matches 
-- `${*goal*}/all` is returned.  If neither of those targets exists then nil 
-- is returned.
function find_initial_target( goal )
    if not goal or goal == '' then 
        local goal = initial();
        local all = find_target( ('%s/all'):format(goal) );
        while not all and goal ~= '' do 
            goal = branch( goal );
            all = find_target( ('%s/all'):format(goal) );
        end
        return all;
    end

    local goal = initial( goal );
    local all = find_target( goal );
    if all and all:dependency() then 
        return all;
    end

    local all = find_target( ('%s/all'):format(goal) );
    if all and all:dependency() then
        return all;
    end
    return nil;
end

function forge:TargetPrototype( identifier )
    return add_target_prototype( identifier );
end

function forge:FilePrototype( identifier, filename_modifier )
    local filename_modifier = filename_modifier or forge.Toolset.interpolate;
    local file_prototype = forge:TargetPrototype( identifier );
    file_prototype.create = function( toolset, identifier, target_prototype )
        local identifier, filename = filename_modifier( toolset, identifier );
        local target = forge.Target( toolset, identifier, target_prototype );
        target:set_filename( filename or target:path() );
        target:set_cleanable( true );
        target:add_ordering_dependency( toolset:Directory(branch(target)) );
        return target;
    end;
    return file_prototype;
end

function forge:JavaStylePrototype( identifier, pattern )
    local output_directory_modifier = forge.Toolset.interpolate;
    local pattern = pattern or '(.-([^\\/]-))%.?([^%.\\/]*)$';
    local java_style_prototype = forge:TargetPrototype( identifier );
    function java_style_prototype.create( toolset, output_directory, target_prototype )
        local output_directory = root_relative():gsub( pattern, output_directory_modifier(toolset, output_directory) );
        local target = forge.Target( toolset, anonymous(), target_prototype );
        target:add_ordering_dependency( toolset:Directory(output_directory) );
        target:set_cleanable( true );
        return target;
    end
    return java_style_prototype;
end

function forge:PatternElement( target_prototype, replacement_modifier, pattern )
    local target_prototype = target_prototype or forge.Toolset.File;
    local replacement_modifier = replacement_modifier or forge.Toolset.interpolate;
    local pattern = pattern or '(.-([^\\/]-))%.?([^%.\\/]*)$';
    return function( toolset, replacement )
        local targets = {};
        local replacement = replacement_modifier( toolset, replacement );
        return function( dependencies )
            local attributes = toolset:merge( {}, dependencies );
            for _, filename in forge:walk_tables(dependencies) do
                local source_file = toolset:SourceFile( filename );
                local identifier = root_relative( source_file ):gsub( pattern, replacement );
                local target = target_prototype( toolset, identifier );
                target:add_dependency( source_file );
                toolset:merge( target, attributes );
                table.insert( targets, target );
            end
            return targets;
        end
    end    
end

function forge:GroupElement( target_prototype, replacement_modifier, pattern )
    local target_prototype = target_prototype or forge.Toolset.File;
    local replacement_modifier = replacement_modifier or forge.Toolset.interpolate;
    local pattern = pattern or '(.-([^\\/]-))%.?([^%.\\/]*)$';
    return function( toolset, replacement )
        local target = forge.Target( toolset, anonymous(), target_prototype );
        local replacement = replacement_modifier( toolset, replacement );
        return function( dependencies )
            toolset:merge( target, dependencies );
            for _, filename in forge:walk_tables(dependencies) do
                local source_file = toolset:SourceFile( filename );
                local identifier = root_relative( source_file ):gsub( pattern, replacement );
                local file = toolset:File( identifier );
                file:add_dependency( source_file );
                target:add_dependency( file );
            end
            return target;
        end
    end
end

-- Recursively walk the dependencies of *target* until a target with a 
-- filename is reached.
function forge:walk_dependencies( target )
    local index = 1;
    local function walk( target )
        for _, dependency in target:dependencies() do 
            local phony = dependency:filename() == '';
            if not phony then
                coroutine.yield( index, dependency );
                index = index + 1;
            end
            if phony then 
                walk( dependency );
            end
        end
    end
    return coroutine.wrap( function()
        walk( target );
    end );
end

-- Recursively walk the tables passed in *dependencies* until reaching targets
-- or non-table values (e.g. strings) that are yielded back to the caller.
function forge:walk_tables( dependencies )
    local function typename( value )
        if type(value) == 'table' then 
            local metatable = getmetatable( value );
            return metatable and metatable.__name;
        end
    end

    local index = 1;
    local function walk( values )
        for _, value in ipairs(values) do
            if type(value) ~= 'table' or typename(value) == 'forge.Target' then
                coroutine.yield( index, value );
                index = index + 1;
            else
                walk( value );
            end
        end
    end

    return coroutine.wrap( function()
        walk( dependencies );
    end );
end

-- Load cached dependencies and local settings.
--
-- Cached dependencies are loaded from the file named *.forge* in the root
-- directory of the project or the file named *${variant}/.forge* if the
-- variables `variant` or `forge.variant` are set.
--
-- Local settings are loaded from the file *local_settings.lua* in the root
-- directory of the project if it exists or set to an empty table otherwise.
--
-- Returns a new toolset initialized with the local settings.
function forge:load()
    self.local_settings = exists( root('local_settings.lua') ) and dofile( root('local_settings.lua') ) or {};
    self.cache = root( '.forge' );
    self.Toolset = require 'forge.Toolset';

    if variant or self.variant then 
        self.cache = root( ('%s/.forge'):format(variant or self.variant) );
    end

    load_binary( self.cache );
    return forge.Toolset( forge.local_settings );
end

-- Save the dependency graph and local settings.
function forge:save()
    -- Serialize values to to a Lua file (typically the local settings table).
    local function serialize( file, value, level )
        local function indent( level )
            for i = 1, level do
                file:write( '  ' );
            end
        end

        if level == 0 then
            file:write( '\nreturn ' );
        end

        if type(value) == 'boolean' then
            file:write( tostring(value) );
        elseif type(value) == 'number' then
            file:write( value );
        elseif type(value) == 'string' then
            file:write( string.format('%q', value) );
        elseif type(value) == 'table' then
            file:write( '{\n' );
            for _, v in ipairs(value) do
                indent( level + 1 );
                serialize( file, v, level + 1 );
                file:write( ',\n' );
            end
            for k, v in pairs(value) do
                if type(k) == 'string' and k ~= '__forge_hash' then
                    indent( level + 1 );
                    file:write( ('%s = '):format(k) );
                    serialize( file, v, level + 1 );
                    file:write( ';\n' );
                end
            end
            indent( level );
            file:write( '}' );
        end

        if level == 0 then 
            file:write( '\n' );
        end
    end

    local local_settings = self.local_settings;
    if local_settings and local_settings.updated then
        local_settings.updated = nil;
        local filename = root( 'local_settings.lua' );
        local file = io.open( filename, 'wb' );
        assertf( file, 'Opening "%s" to write settings failed', filename );
        serialize( file, local_settings, 0 );
        file:close();
    end
    mkdir( branch(forge.cache) );
    save_binary();
end

return forge:load();
