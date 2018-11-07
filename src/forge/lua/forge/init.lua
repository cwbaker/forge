
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

-- Provide global buildfile().
function buildfile( ... )
    return forge:buildfile( ... );
end

-- Provide global default command that calls through to `build()`.
function default()
    return build();
end

-- Provide global build command.
function build()
    local failures = forge:postorder( forge:find_initial_target(goal), forge:build_visit() );
    forge:save();
    printf( "forge: default (build)=%dms", math.ceil(forge:ticks()) );
    return failures;
end

-- Provide global clean command.
function clean()
    local failures = forge:postorder( forge:find_initial_target(goal), forge:clean_visit() );
    printf( "forge: clean=%sms", tostring(math.ceil(forge:ticks())) );
    return failures;
end

-- Provide global reconfigure command.
function reconfigure()
    rm( forge.settings.local_settings_filename );
    forge:find_initial_target();
    return 0;
end

-- Provide global dependencies command.
function dependencies()
    forge:print_dependencies( forge:find_initial_target(goal) );
    return 0;
end

-- Provide global namespace command.
function namespace()
    forge:print_namespace( forge:find_initial_target(goal) );
    return 0;
end

-- Provide global help command.
function help()
    printf [[
Variables:
  goal               Target to build (relative to current working directory).
  variant            Variant built (debug, release, or shipping).
  version            Version used through build scripts and code.
Commands:
  build              Build outdated targets.
  clean              Clean all targets.
  reconfigure        Regenerate per-machine configuration settings.
  dependencies       Print targets by dependency hierarchy.
  namespace          Print targets by namespace hierarchy.
    ]];
end

forge = _G.forge or {};
forge.settings = {};
forge.modules_ = {};
forge.default_builds_ = {};

function forge:add_default_build( identifier, build )
    table.insert( self.default_builds_, {identifier, build} );
end

function forge:default_build( pattern )
    if pattern then
        pattern = self:interpolate( pattern );
    end
    for _, default_build in ipairs(self.default_builds_) do 
        local identifier = default_build[1];
        if pattern == nil or pattern == "" or identifier:find(pattern) then 
            return default_build[2];
        end
    end        
end

function forge:default_builds( pattern )
    if pattern then
        pattern = self:interpolate( pattern );
    end
    return coroutine.wrap( function() 
        local index = 1;
        for _, default_build in ipairs(self.default_builds_) do 
            local identifier = default_build[1];
            if pattern == nil or pattern == "" or identifier:find(pattern) then 
                coroutine.yield( index, default_build[2] );
                index = index + 1;
            end
        end        
    end );
end

function forge:platform_matches( ... )
    local platform = self.settings.platform;
    if platform == "" then 
        return true;
    end
    for i = 1, select("#", ...) do
        if platform:match(select(i, ...)) then
            return true;
        end
    end
    return false;
end

function forge:TargetPrototype( identifier )
    local target_prototype = self:target_prototype( identifier );
    self[identifier] = target_prototype;
    return target_prototype;
end

function forge:FilePrototype( identifier, filename_modifier )
    local filename_modifier = filename_modifier or forge.interpolate;
    local file_prototype = self:TargetPrototype( identifier );
    file_prototype.create = function( forge, identifier, target_prototype )
        local identifier, filename = filename_modifier( forge, identifier );
        local target = forge:Target( identifier, target_prototype );
        target:set_filename( filename or target:path() );
        target:set_cleanable( true );
        target:add_ordering_dependency( forge:Directory(forge:branch(target)) );
        return target;
    end;
    return file_prototype;
end

function forge:PatternPrototype( identifier, replacement_modifier )
    local replacement_modifier = replacement_modifier or forge.interpolate;
    local pattern_prototype = self:TargetPrototype( identifier );
    pattern_prototype.create = function( forge, identifier )
        local target = forge:Target( forge:anonymous() );
        target.replacement = replacement_modifier( forge, identifier );
        target.depend = function( forge, target, dependencies )
            local replacement = target.replacement;
            local pattern = '(.-([^\\/]-))(%.?[^%.\\/]*)$';
            local attributes = forge:merge( {}, dependencies );
            for _, filename in ipairs(dependencies) do
                local source_file = forge:SourceFile( filename );
                local identifier = forge:root_relative( source_file ):gsub( pattern, replacement );
                local file = forge:File( identifier, pattern_prototype );
                forge:merge( file, attributes );
                file:add_dependency( source_file );
                target:add_dependency( file );
            end
        end     
        return target;
    end
    return pattern_prototype;
end

function forge:GroupPrototype( identifier, replacement_modifier )
    local replacement_modifier = replacement_modifier or forge.interpolate;
    local group_prototype = self:TargetPrototype( identifier );
    group_prototype.create = function( forge, identifier, target_prototype )
        local target = forge:Target( forge:anonymous(), target_prototype );
        target.replacement = replacement_modifier( forge, identifier );
        return target;
    end;
    group_prototype.depend = function( forge, target, dependencies )
        forge:merge( target, dependencies );
        local replacement = target.replacement;
        local pattern = '(.-([^\\/]-))(%.?[^%.\\/]*)$';
        for _, filename in ipairs(dependencies) do
            local source_file = forge:SourceFile( filename );
            local identifier = forge:root_relative( source_file ):gsub( pattern, replacement );
            local file = forge:File( identifier );
            file:add_dependency( source_file );
            target:add_dependency( file );
        end
    end     
    return group_prototype;
end

function forge:File( identifier, target_prototype )
    local target = self:Target( self:interpolate(identifier), target_prototype );
    target:set_filename( target:path() );
    target:set_cleanable( true );
    target:add_ordering_dependency( self:Directory(self:branch(target)) );
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

function forge:map( target_prototype, replacement, pattern, filenames )
    local targets = {};
    local settings = settings or self.settings;
    for _, source_filename in ipairs(filenames) do 
        local source = self:relative( source_filename );
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
    for source_filename in self:ls('') do
        local source = self:relative( source_filename );
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
    local settings = settings or self.settings;
    for source_filename in self:find("") do
        if self:is_file(source_filename) then
            local source = self:relative( source_filename );
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

-- Perform per run initialization of the build system.
function forge:initialize( project_settings )
    -- Merge settings from /source_settings/ into /settings/.
    local function merge_settings( settings, source_settings )
        settings = settings or {};
        if source_settings then 
            for _, v in ipairs(source_settings) do
                table.insert( settings, v );
            end
            for k, v in pairs(source_settings) do
                if type(k) == "string" then
                    if type(v) == "table" then
                        settings[k] = merge_settings( settings[k], v );
                    else
                        settings[k] = v;
                    end
                end
            end
        end
        return settings;
    end

    -- Set default values for variables that can be passed on the command line.
    platform = platform or self:operating_system();
    variant = variant or "debug";
    version = version or ("%s %s %s"):format( os.date("%Y.%m.%d.%H%M"), platform, variant );
    goal = goal or "";
    jobs = jobs or 8;

    self:set_maximum_parallel_jobs( jobs );
    if self:operating_system() == "linux" then
        self:set_forge_hooks_library( self:executable("libforge_hooks.so") );
    elseif self:operating_system() == "macos" then
        self:set_forge_hooks_library( self:executable("forge_hooks.dylib") );
    elseif self:operating_system() == "windows" then 
        self:set_forge_hooks_library( self:executable("forge_hooks.dll") );
    end    

    -- Set default settings (all other settings inherit from this table).
    local default_settings = dofile( self:script('forge/default_settings') );

    local local_settings = {};
    setmetatable( local_settings, {__index = default_settings}  );

    local user_settings_filename = default_settings.user_settings_filename;
    if self:exists(user_settings_filename) then
        merge_settings( local_settings, dofile(user_settings_filename) );
    end

    local local_settings_filename = default_settings.local_settings_filename;
    if self:exists(local_settings_filename) then
        merge_settings( local_settings, dofile(local_settings_filename) );
    end

    local variant_settings = default_settings.settings_by_variant[variant];
    assertf( variant_settings, "The variant '%s' is not supported", variant );

    local platform_settings = default_settings.settings_by_platform[platform];
    assertf( platform_settings, "The platform '%s' is not supported", platform );

    local settings = {};
    setmetatable( settings, {__index = local_settings} );
    merge_settings( settings, variant_settings );
    merge_settings( settings, platform_settings );
    merge_settings( settings, project_settings );

    if settings.library_type == "static" then
        self.Library = self.StaticLibrary;
    elseif settings.library_type == "dynamic" then
        self.Library = self.DynamicLibrary;
    else
        error( string.format("The library type '%s' is not 'static' or 'dynamic'", tostring(settings.library_type)) );
    end

    if settings.variants and #settings.variants > 0 then 
        default_settings.cache = self:root( ('%s/.forge'):format(variant) );
    end

    _G.settings = settings;
    self.default_settings = default_settings;
    self.local_settings = local_settings;
    self.settings = settings;
    self:configure_modules( settings );
    self:initialize_modules( settings );
    self:load_binary( settings.cache );
    self:clear();
    return settings;
end

-- Register *module* to be configured and initialized when the build sysetm 
-- is initialized.
function forge:register_module( module )
    table.insert( self.modules_, module ); 
end

-- Call `configure` for each registered module that provides it.
function forge:configure_modules( settings )
    local modules = self.modules_;
    for _, module in ipairs(modules) do 
        local configure = module.configure;
        if configure and type(configure) == "function" then 
            configure( settings );
        end
    end
end

-- Call `initialize` for each registered module that provides it.
function forge:initialize_modules( settings )
    local modules = self.modules_;
    for _, module in ipairs(modules) do 
        local initialize = module.initialize;
        if initialize and type(initialize) == "function" then 
            initialize( settings );
        end
    end
end

-- Convert a version string into a date table (assuming that the version 
-- string is of the form '%Y.%m.%d.%H%M').
function forge:version_date( version )
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
function forge:version_time( version )
    return os.time( self:version_date(version) );
end

-- Convert a version string into the number of half days passed since 
-- *reference_time* or since 2012/01/01 00:00 if *reference_time* is not 
-- provided (assuming that the version string is of the form '%Y.%m.%d.%H%M').
function forge:version_code( version, reference_time )
    reference_time = reference_time or os.time( {year = 2012; month = 1; day = 1; hour = 0; min = 0; sec = 0} );
    local SECONDS_PER_HALF_DAY = 12 * 60 * 60;
    local version_time = self:version_time( version );
    return math.ceil( os.difftime(version_time, reference_time) / SECONDS_PER_HALF_DAY );
end

-- Add targets to the current directory's target so that they will be built 
-- when a build is invoked from that directory.
function forge:default_targets( targets )
    local all = self:all();
    for _, default_target in ipairs(targets) do
        local target = self:add_target( ("%s/all"):format(default_target) );
        all:add_dependency( target );
    end
end

-- Execute command with arguments and optional filter and raise an error if 
-- it doesn't return 0.
function forge:system( command, arguments, environment, dependencies_filter, stdout_filter, stderr_filter, ... )
    if type(arguments) == 'table' then
        arguments = table.concat( arguments, ' ' );
    end
    if self:execute(command, arguments, environment, dependencies_filter, stdout_filter, stderr_filter, ...) ~= 0 then       
        error( ('%s failed'):format(arguments), 0 );
    end
end

-- Execute a command through the host system's native shell - either 
-- "C:/windows/system32/cmd.exe" on Windows system or "/bin/sh" anywhere else.
function forge:shell( arguments, dependencies_filter, stdout_filter, stderr_filter, ... )
    if type(arguments) == "table" then 
        arguments = table.concat( arguments, " " );
    end
    if self:operating_system() == "windows" then
        local cmd = "C:/windows/system32/cmd.exe";
        local result = self:execute( cmd, ('cmd /c "%s"'):format(arguments), dependencies_filter, stdout_filter, stderr_filter, ... );
        assertf( result == 0, "[[%s]] failed (result=%d)", arguments, result );
    else
        local sh = "/bin/sh";
        local result = self:execute( sh, ('sh -c "%s"'):format(arguments), dependencies_filter, stdout_filter, stderr_filter, ... );
        assertf( result == 0, "[[%s]] failed (result=%d)", arguments, tonumber(result) );
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
    local variables = variables or self:current_settings();
    return (template:gsub('%$(%b{})', function(word) 
        local parameters = split( interpolate(forge, word:sub(2, -2), variables) );
        local identifier = parameters[1];
        local substitute = variables[identifier];
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

-- Dump the keys, values, and prototype of a table for debugging.
function forge:dump( t )
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

-- Get the *all* target for the current working directory adding any 
-- targets that are passed in as dependencies.
function forge:all( dependencies )
    local all = self:target( "all" );
    if dependencies then 
        all.depend( self, all, dependencies );
    end
    return all;
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
    if not goal or goal == "" then 
        local goal = self:initial();
        local all = self:find_target( ("%s/all"):format(goal) );
        while not all and goal ~= "" do 
            goal = self:branch( goal );
            all = self:find_target( ("%s/all"):format(goal) );
        end
        return all;
    end

    local goal = self:initial( goal );
    local all = self:find_target( goal );
    if all and all:dependency() then 
        return all;
    end

    local all = self:find_target( ("%s/all"):format(goal) );
    if all and all:dependency() then
        return all;
    end
    return nil;
end

-- Save the dependency graph to the file specified by /settings.cache/.
function forge:save()
    local settings = self:current_settings();
    if self.local_settings and self.local_settings.updated then
        self.local_settings.updated = nil;
        self:save_settings( self.local_settings, settings.local_settings_filename );
    end
    self:mkdir( self:branch(settings.cache) );
    self:save_binary( settings.cache );
end

-- Express *path* relative to the root directory.
function forge:root_relative( path )
    return self:relative( self:absolute(path), self:root() );
end

-- Convert /name/ into a path relative to the first pattern in package.paths
-- that expands to an existing file.
function forge:script( name )
    for path in string.gmatch(package.path, "([^;]*);?") do
        local filename = string.gsub( path, "?", name );
        if self:exists(filename) then
            return filename;
        end
    end
    errorf( "Script '%s' not found", filename );
end

-- Convert /filename/ into an object directory path by prepending the object 
-- directory to the portion of /filename/ that is relative to the root 
-- directory.
function forge:object( filename, extension, settings )
    local settings = settings or self:current_settings();
    local prefix = settings.obj or self:root();
    local filename = self:relative( self:absolute(filename), self:root() );
    return ('%s/%s'):format( prefix, filename );
end

-- Convert /path/ into a generated files directory path by prepending the 
-- generated directory to the portion of /path/ that is relative to the root
-- directory.
function forge:generated( filename, architecture, settings )
    settings = settings or self:current_settings();
    filename = self:relative( self:absolute(filename), self:root() );
    if architecture then 
        return ("%s/%s/%s"):format( settings.gen, architecture, filename );
    end
    return ("%s/%s"):format( settings.gen, filename );
end

function forge:configure( settings )
    local forge = { 
        __this = self.__this;
        settings = settings;
    };
    setmetatable( settings, {
        __index = self.settings;
    } );
    setmetatable( forge, {
        __index = self;
    } );
    return forge, settings;
end

function forge:current_settings()
    return self.settings;
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
        elseif target:cleanable() and target:filename() ~= "" then 
            forge:rm( target:filename() );
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
        local build_function = target.build;
        if build_function and target:outdated() then 
            local success, error_message = pcall( build_function, target.forge, target, table.unpack(args) );
            target:set_built( success );
            if not success then 
                clean_visit( target );
                assert( success, error_message );
            end
        end
    end
end

-- Add dependencies detected by the injected build hooks library to the 
-- target /target/.
function forge:dependencies_filter( target )
    return function( line )
        if line:match("^==") then 
            local READ_PATTERN = "^== read '([^']*)'";
            local filename = line:match( READ_PATTERN );
            if filename then
                local within_source_tree = self:relative( self:absolute(filename), self:root() ):find( "..", 1, true ) == nil;
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

-- Recursively walk the dependencies of *target* until a target with a 
-- filename or the maximum level limit is reached.
function forge:walk_dependencies( target, start, finish, maximum_level )
    local index = 1;
    local maximum_level = maximum_level or math.maxinteger;
    local function walk_dependencies_recursively( target, level )
        for _, dependency in target:dependencies() do 
            local phony = dependency:filename() == "";
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

-- Recursively copy files from *source* to *destination*.
function forge:cpdir( destination, source, settings )
    local settings = settings or self:current_settings();
    local destination = self:interpolate( destination, settings );
    local source = self:interpolate( source, settings );
    self:pushd( source );
    for source_filename in self:find('') do 
        if self:is_file(source_filename) then
            local filename = ('%s/%s'):format( destination, self:relative(source_filename) );
            self:mkdir( self:branch(filename) );
            self:rm( filename );
            self:cp( filename, source_filename );
        end
    end
    self:popd();
end

require "forge.Target";
require "forge.Directory";
