
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

-- Provide global build command.
function build()
    local failures = postorder( forge:find_initial_target(goal), forge:build_visit() );
    forge:save();
    printf( "forge: default (build)=%dms", math.ceil(ticks()) );
    return failures;
end

-- Provide global clean command.
function clean()
    local failures = postorder( forge:find_initial_target(goal), forge:clean_visit() );
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
    print_dependencies( forge:find_initial_target(goal) );
    return 0;
end

-- Provide global namespace command.
function namespace()
    print_namespace( forge:find_initial_target(goal) );
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

function forge:add_build( identifier, build )
    table.insert( self.builds_, {build:interpolate(identifier), build} );
    return build;
end

function forge:build( pattern )
    for _, build in ipairs(self.builds_) do 
        local identifier = build[1];
        if pattern == nil or pattern == '' or identifier:find(pattern) then 
            return build[2];
        end
    end        
end

function forge:builds( pattern )
    return coroutine.wrap( function() 
        local index = 1;
        for _, build in ipairs(self.builds_) do 
            local identifier = build[1];
            if pattern == nil or pattern == '' or identifier:find(pattern) then 
                coroutine.yield( index, build[2] );
                index = index + 1;
            end
        end        
    end );
end

function forge:call( settings_to_apply )
    self:copy_settings( self.settings, settings_to_apply );
end

function forge:create( settings )
    local forge = {
        __this = self.__this;
        settings = settings or {};
    };
    setmetatable( forge, {
        __index = self;
    } );
    return forge;
end

function forge:clone( settings_to_apply )
    local settings = {};
    local copy_settings = self.copy_settings;
    copy_settings( self, settings, self.settings );
    copy_settings( self, settings, settings_to_apply );
    return self:create( settings );
end

function forge:inherit( settings )
    local settings = settings or {};
    setmetatable( settings, {
        __index = self.settings;
    } );
    return self:create( settings );
end

function forge:PatternElement( target_prototype, replacement_modifier, pattern )
    local target_prototype = target_prototype or self.File;
    local replacement_modifier = replacement_modifier or self.interpolate;
    local pattern = pattern or '(.-([^\\/]-))%.?([^%.\\/]*)$';

    return function( forge, replacement )
        local targets = {};
        local replacement = replacement_modifier( forge, replacement );
        return function( dependencies )
            local attributes = forge:merge( {}, dependencies );
            for _, filename in forge:walk_tables(dependencies) do
                local source_file = forge:SourceFile( filename );
                local identifier = forge:root_relative( source_file ):gsub( pattern, replacement );
                local target = target_prototype( forge, identifier );
                target:add_dependency( source_file );
                forge:merge( target, attributes );
                table.insert( targets, target );
            end
            return targets;
        end
    end    
end

function forge:GroupElement( target_prototype, replacement_modifier, pattern )
    local target_prototype = target_prototype or self.File;
    local replacement_modifier = replacement_modifier or self.interpolate;
    local pattern = pattern or '(.-([^\\/]-))%.?([^%.\\/]*)$';
    
    return function( forge, replacement )
        local target = forge:Target( anonymous(), target_prototype );
        local replacement = replacement_modifier( forge, replacement );
        return function( dependencies )
            forge:merge( target, dependencies );
            for _, filename in forge:walk_tables(dependencies) do
                local source_file = forge:SourceFile( filename );
                local identifier = forge:root_relative( source_file ):gsub( pattern, replacement );
                local file = forge:File( identifier );
                file:add_dependency( source_file );
                target:add_dependency( file );
            end
            return target;
        end
    end
end

function forge:TargetPrototype( identifier )
    return add_target_prototype( identifier );
end

function forge:FilePrototype( identifier, filename_modifier )
    local filename_modifier = filename_modifier or forge.interpolate;
    local file_prototype = self:TargetPrototype( identifier );
    file_prototype.create = function( forge, identifier, target_prototype )
        local identifier, filename = filename_modifier( forge, identifier );
        local target = forge:Target( identifier, target_prototype );
        target:set_filename( filename or target:path() );
        target:set_cleanable( true );
        target:add_ordering_dependency( forge:Directory(branch(target)) );
        return target;
    end;
    return file_prototype;
end

function forge:JavaStylePrototype( identifier, pattern )
    local interpolate = forge.interpolate;
    local pattern = pattern or '(.-([^\\/]-))%.?([^%.\\/]*)$';
    local java_style_prototype = self:TargetPrototype( identifier );
    function java_style_prototype.create( forge, output_directory, target_prototype )
        local output_directory = forge:root_relative():gsub( pattern, interpolate(forge, output_directory) );
        local target = forge:Target( anonymous(), target_prototype );
        target:add_ordering_dependency( forge:Directory(output_directory) );
        target:set_cleanable( true );
        return target;
    end
    return java_style_prototype;
end

function forge:File( identifier, target_prototype )
    local target = self:Target( self:interpolate(identifier), target_prototype );
    target:set_filename( target:path() );
    target:set_cleanable( true );
    target:add_ordering_dependency( self:Directory(branch(target)) );
    return target;
end

function forge:SourceFile( identifier )
    local target = identifier;
    if type(target) == 'string' then 
        target = self:Target( self:interpolate(identifier) );
        if target:filename() == '' then 
            target:set_filename( target:path() );
        end
        target:set_cleanable( false );
    end
    return target;
end

function forge:SourceDirectory( identifier, settings )
    return self:SourceFile( identifier, settings );
end

-- Execute command with arguments and optional filter and raise an error if 
-- it doesn't return 0.
function forge:system( command, arguments, environment, dependencies_filter, stdout_filter, stderr_filter, ... )
    if type(arguments) == 'table' then
        arguments = table.concat( arguments, ' ' );
    end
    if execute(command, arguments, environment, dependencies_filter, stdout_filter, stderr_filter, ...) ~= 0 then       
        error( ('%s failed'):format(arguments), 0 );
    end
end

-- Execute a command through the host system's native shell - either 
-- "C:/windows/system32/cmd.exe" on Windows system or "/bin/sh" anywhere else.
function forge:shell( arguments, dependencies_filter, stdout_filter, stderr_filter, ... )
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
function forge:switch( values )
    assert( values[1] ~= nil, "No value passed to `forge:switch()`" );
    return values[values[1]];
end

-- Provide GNU Make like string interpolation.
function forge:interpolate( template, variables )
    local function split( input )
        local output = {};
        for value in input:gmatch('%S+') do 
            table.insert( output, value );
        end
        return output;
    end

    local interpolate = forge.interpolate;
    local settings = self.settings;
    local variables = variables or settings;
    return (template:gsub('%$(%b{})', function(word) 
        local parameters = split( interpolate(forge, word:sub(2, -2), variables) );
        local identifier = parameters[1];
        local substitute = variables[identifier];
        if not substitute then 
            substitute = settings[identifier];
        end
        if not substitute then 
            substitute = self[identifier];
        end
        if not substitute then 
            substitute = _G[identifier]
        end
        if not substitute then 
            substitute = os.getenv( identifier );
        end
        if type(substitute) == 'function' then 
            substitute = substitute( self, table.unpack(parameters, 2) );
        elseif type(substitute) == 'table' then
            substitute = substitute[parameters[2]];
        end
        assertf( substitute, 'Missing substitute for "%s" in "%s"', identifier, template );
        return substitute or word;
    end));
end

-- Save a settings table to a file.
function forge:save_settings( settings, filename )
    -- Serialize values to to a Lua file (typically the local settings table).
    local function serialize( file, value, level )
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
                serialize( file, v, level + 1 );
                file:write( ",\n" );
            end
            for k, v in pairs(value) do
                if type(k) == "string" then
                    indent( level + 1 );
                    file:write( ("%s = "):format(k) );
                    serialize( file, v, level + 1 );
                    file:write( ";\n" );
                end
            end
            indent( level );
            file:write( "}" );
        end
    end

    local file = io.open( filename, "wb" );
    assertf( file, "Opening %s to write settings failed", filename );
    file:write( "\nreturn " );
    serialize( file, settings, 0 );
    file:write( "\n" );
    file:close();
end

-- Append values from /value/ to /values/.
function forge:append( values, value )
    local values = values or {};
    if type(value) == "table" then 
        for _, other_value in ipairs(value) do 
            table.insert( values, other_value );
        end
    else
        table.insert( values, value );
    end
    return values;
end

-- Merge fields with string keys from /source/ to /destination/.
function forge:merge( destination, source )
    local destination = destination or {};
    for key, value in pairs(source) do
        if type(key) == 'string' then
            if type(value) == 'table' then
                destination[key] = self:append( destination[key], value );
            else
                destination[key] = value;
            end
        end
    end
    return destination;
end

-- Set fields in that aren't set in *destination* to values in *source*.
function forge:defaults( destination, source )
    local destination = destination or {};
    for key, value in pairs(source) do 
        if type(key) == 'string' and destination[key] == nil then 
            destination[key] = value;
        end
    end
    return destination;
end

function forge:copy_settings( destination, source )
    if source then
        local copy_settings = self.copy_settings;
        for key, value in pairs(source) do 
            if type(value) ~= 'table' then 
                destination[key] = value;
            else
                destination[key] = copy_settings( self, destination[key] or {}, value );
            end
        end
    end
    return destination;
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
function forge:find_initial_target( goal )
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

-- Save the dependency graph to the file specified by /settings.cache/.
function forge:save()
    local settings = self.settings;
    if self.local_settings and self.local_settings.updated then
        self.local_settings.updated = nil;
        self:save_settings( self.local_settings, root('local_settings.lua') );
    end
    mkdir( branch(forge.cache) );
    save_binary();
end

-- Express *path* relative to the root directory.
function forge:root_relative( path )
    return relative( absolute(path), root() );
end

-- Visit a target by calling a member function "clean" if it exists or if
-- there is no "clean" function and the target is not marked as a source file
-- that must exist then its associated file is deleted.
function forge:clean_visit( ... )
    local args = {...};
    return function ( target )
        local clean_function = target.clean;
        if clean_function then 
            clean_function( target.forge, target, table.unpack(args) );
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
end

-- Visit a target by calling a member function "build" if it exists and 
-- setting that Target's built flag to true if the function returns with
-- no errors.
function forge:build_visit( ... )
    local args = {...};
    local clean_visit = self:clean_visit( table.unpack(args) );
    return function ( target )
        if target:outdated() then
            local build_function = target.build;
            if build_function then 
                local success, error_message = pcall( build_function, target.forge, target, table.unpack(args) );
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
end

-- Add dependencies detected by the injected build hooks library to the 
-- target /target/.
function forge:dependencies_filter( target )
    return function( line )
        if line:match('^==') then 
            local READ_PATTERN = "^== read '([^']*)'";
            local filename = line:match( READ_PATTERN );
            if filename then
                local within_source_tree = relative( absolute(filename), root() ):find( '..', 1, true ) == nil;
                if within_source_tree then 
                    local header = self:SourceFile( filename );
                    target:add_implicit_dependency( header );
                end
            end
        else
            print( line );
        end
    end
end

-- Add dependencies detected by the injected build hooks library to the 
-- target /target/.
function forge:filenames_filter( target )
    target:clear_filenames();
    local output_directory = target:ordering_dependency():filename();
    return function( line )
        if line:match('^==') then 
            local READ_WRITE_PATTERN = "^== (%a+) '([^']*)'";
            local read_write, filename = line:match( READ_WRITE_PATTERN );
            if read_write and filename then
                local within_source_tree = relative( absolute(filename), output_directory ):find( '..', 1, true ) == nil;
                if within_source_tree then 
                    if read_write == 'write' then
                        target:add_filename( filename );
                    else
                        local source_file = self:SourceFile( filename );
                        target:add_implicit_dependency( source_file );
                    end
                end
            end
        else
            print( line );
        end
    end
end

-- Recursively walk the dependencies of *target* until a target with a 
-- filename or the maximum level limit is reached.
function forge:walk_dependencies( target, start, finish, maximum_level )
    local index = 1;
    local maximum_level = maximum_level or math.maxinteger;
    local function walk_dependencies_recursively( target, level )
        for _, dependency in target:dependencies() do 
            local phony = dependency:filename() == '';
            if not phony then
                coroutine.yield( index, dependency, level );
                index = index + 1;
            end
            if phony and level + 1 < maximum_level then 
                walk_dependencies_recursively( dependency, level + 1 );
            end
        end
    end
    return coroutine.wrap( function() 
        for _, dependency in target:dependencies(start, finish) do 
            local phony = dependency:filename() == '';
            if not phony then
                coroutine.yield( index, dependency, level );
                index = index + 1;
            end
            if phony then 
                walk_dependencies_recursively( dependency, 0 );
            end
        end
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

function forge:platform_matches( ... )
    local platform = self.settings.platform;
    if platform == nil or platform == '' then 
        return true;
    end
    for i = 1, select('#', ...) do
        if platform:match(select(i, ...)) then
            return true;
        end
    end
    return false;
end

-- Get the *all* target for the current working directory adding any 
-- targets that are passed in as dependencies.
function forge:all( dependencies )
    local all = add_target( self, 'all' );
    if dependencies then 
        for _, dependency in self:walk_tables(dependencies) do
            if type(dependency) == 'string' then 
                dependency = add_target( self, self:interpolate(dependency) );
            end
            all:add_dependency( dependency );
        end
    end
    return all;
end

function forge:map( target_prototype, replacement, pattern, filenames )
    local targets = {};
    local settings = self.settings;
    for _, source_filename in ipairs(filenames) do 
        local source = forge:relative( source_filename );
        local filename, substitutions = source:gsub( pattern, replacement );
        if substitutions > 0 then 
            local destination = self:interpolate( filename, settings );
            local target = target_prototype (self, destination) {
                source
            };
            table.insert( targets, target );
        end
    end
    return table.unpack( targets );
end

function forge:map_ls( target_prototype, replacement, pattern, settings )
    local targets = {};
    local settings = settings or self.settings;
    for source_filename in ls('') do
        local source = forge:relative( source_filename );
        local filename, substitutions = source:gsub( pattern, replacement );
        if substitutions > 0 then    
            local destination = self:interpolate( filename, settings );
            local target = target_prototype (self, destination) {
                source
            };
            table.insert( targets, target );
        end
    end
    return table.unpack( targets );
end

function forge:map_find( target_prototype, replacement, pattern, settings )
    local targets = {};
    local settings = self.settings;
    for source_filename in find("") do
        if is_file(source_filename) then
            local source = forge:relative( source_filename );
            local filename, substitutions = source:gsub( pattern, replacement );
            if substitutions > 0 then
                local destination = self:interpolate( filename, settings );
                local target = target_prototype (self, destination) {
                    source
                };
                table.insert( targets, target );
            end
        end
    end
    return table.unpack( targets );
end

-- Convert a target into the object directory that it should build to.
function forge:obj_directory( target )
    local relative_path = relative( target:working_directory():path(), root() );
    return absolute( relative_path, self.settings.obj );
end

-- Recursively copy files from *source* to *destination*.
function forge:cpdir( destination, source, settings )
    local settings = settings or self.settings;
    local destination = self:interpolate( destination, settings );
    local source = self:interpolate( source, settings );
    pushd( source );
    for source_filename in find('') do 
        if is_file(source_filename) then
            local filename = ('%s/%s'):format( destination, relative(source_filename) );
            mkdir( branch(filename) );
            rm( filename );
            cp( filename, source_filename );
        end
    end
    popd();
end

-- Find first existing file named *filename* in *paths*.
--
-- The *filename* variable is interpolated and any variables referenced with 
-- the `${}` syntax are expanded.  See `forge:interpolate()` for details.
--
-- Searching is not performed when *filename* is absolute path.  In this case
-- *filename* is returned immediately only if it names an existing file.
--
-- The *paths* variable can be a string containing a `:` or `;` delimited list
-- of paths or a table containing those paths.  If *paths* is nil then its 
-- default value is set to that returned by `os.getenv('PATH')`.
--
-- Returns the first file named *filename* that exists at a directory listed
-- in *paths* or nothing if no existing file is found.
function forge:which( filename, paths )
    local filename = self:interpolate( filename );
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

function forge:configure( module, identifier )
    local local_settings = self.local_settings;
    local module_settings = local_settings[identifier];
    if not module_settings then
        local settings = self.settings;
        module_settings = module.configure( self, settings[identifier] or {} );
        settings[identifier] = module_settings;
        local_settings[identifier] = module_settings;
        local_settings.updated = true;
    end
    local validate = module.validate;
    return validate == nil or validate( self, module_settings );
end

forge.Target = require 'forge.Target';
forge.Directory = require 'forge.Directory';
forge.Copy = forge:PatternElement( require('forge.Copy') );
forge.CopyDirectory = require 'forge.CopyDirectory';

forge.settings = {};
forge.builds_ = {};
forge.local_settings = exists( root('local_settings.lua') ) and dofile( root('local_settings.lua') ) or {};
forge.cache = root( '.forge' );

if variant or forge.variant then 
    forge.cache = root( ('%s/.forge'):format(variant or forge.variant) );
end

-- Load dependency graph from the cache file, if it exists, and set the name
-- of the cache file either way.
load_binary( forge.cache );

-- Copy local settings values through to settings
forge:copy_settings( forge.settings, forge.local_settings );

setmetatable( forge, {
    __call = function( forge, settings_to_apply )
        forge:copy_settings( forge.settings, settings_to_apply );
        return forge;
    end;
} );

return forge;
