
local Toolset = _G.Toolset or {};

function Toolset.new( _, template )
    local function generate_toolset( values )
        local local_settings = require( 'forge' ).local_settings;
        assertf( local_settings, 'missing local settings' );
        local identifier = Toolset.interpolate( Toolset, template or '', values );
        local toolset = add_toolset( identifier );
        apply( toolset, local_settings );
        apply( toolset, values );
        return toolset;
    end;
    return generate_toolset;
end

function Toolset:clone( values )
    local toolset = {};
    setmetatable( toolset, {__index = self} );
    apply( toolset, self );
    apply( toolset, values );
    return toolset;
end

function Toolset:inherit( values )
    local toolset = values or {};
    setmetatable( toolset, {__index = self} );
    return toolset;
end

function Toolset:apply( values )
    return apply( self, values );
end

function Toolset:defaults( values )
    for key, value in pairs(values) do 
        if type(key) == 'string' and self[key] == nil then
            self[key] = value;
        end
    end
    return self;
end

function Toolset:configure_once( id, configure )
    local local_settings = require( 'forge' ).local_settings;
    assertf( local_settings, 'missing local settings' );
    local settings = local_settings[id];
    if not settings then
        local module_settings = self[id] or {};
        settings = configure( self, module_settings );
        self[id] = settings;
        local_settings[id] = settings;
        local_settings.updated = true;
    end
    return settings;
end

function Toolset:install( module )
    assert( module );
    if type(module) == 'string' then
        module = require( module );
        assert( type(module) == 'table' );
    end
    local install = module.install;
    if install then
        install( self );
    end
end

-- Provide GNU Make like string substitution.
function Toolset:interpolate( template, variables )
    local function split( input )
        local output = {};
        for value in input:gmatch('%S+') do 
            table.insert( output, value );
        end
        return output;
    end

    local variables = variables or self;
    return (template:gsub('%$(%b{})', function(word) 
        local parameters = split( self:interpolate(word:sub(2, -2), variables) );
        local identifier = parameters[1];
        local index = 1;
        local substitute = variables and variables[identifier];
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

-- Add dependencies detected by the injected build hooks library to the 
-- target /target/.
function Toolset:dependencies_filter( target )
    target:clear_implicit_dependencies();
    return function( line )
        if line:match('^==') then 
            local READ_PATTERN = "^== read '([^']*)'";
            local path = line:match( READ_PATTERN );
            if path then
                local relative_path = relative( absolute(path), root() );
                local within_source_tree = is_relative(relative_path) and relative_path:find( '..', 1, true ) == nil;
                if within_source_tree then 
                    local file = self:SourceFile( path );
                    target:add_implicit_dependency( file );
                end
            end
        else
            print( line );
        end
    end
end

-- Add dependencies detected by the injected build hooks library to the 
-- target /target/.
function Toolset:filenames_filter( target )
    target:clear_filenames();
    target:clear_implicit_dependencies();
    local output_directory = target:ordering_dependency():filename();
    return function( line )
        if line:match('^==') then
            local READ_WRITE_PATTERN = "^== (%a+) '([^']*)'";
            local read_write, path = line:match( READ_WRITE_PATTERN );
            if read_write and path then
                local relative_path = relative( absolute(path), output_directory );
                local within_source_tree = is_relative(relative_path) and relative_path:find( '..', 1, true ) == nil;
                if within_source_tree then 
                    if read_write == 'write' then
                        target:add_filename( path );
                    else
                        local source_file = self:SourceFile( path );
                        target:add_implicit_dependency( source_file );
                    end
                end
            end
        else
            print( line );
        end
    end
end

-- Return true if this toolset's platform matches any passed in pattern.
function Toolset:platform_matches( ... )
    local platform = self.platform;
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

-- Convert a target into the object directory that it should build to.
function Toolset:obj_directory( target )
    local relative_path = relative( target:working_directory():path(), root() );
    return absolute( relative_path, self.obj );
end

-- Recursively copy files from *source* to *destination*.
function Toolset:cpdir( destination, source, variables )
    local variables = variables or self;
    local destination = self:interpolate( destination, variables );
    local source = self:interpolate( source, variables );
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
-- the `${}` syntax are expanded.  See `Toolset:interpolate()` for details.
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
function Toolset:which( filename, paths )
    return which( self:interpolate(filename), paths );
end

-- Get the *all* target for the current working directory adding any 
-- targets that are passed in as dependencies.
function Toolset:all( dependencies )
    local all = Target( self, 'all' );
    if dependencies then 
        for _, dependency in walk_tables(dependencies) do
            if type(dependency) == 'string' then 
                dependency = Target( self, self:interpolate(dependency) );
            end
            all:add_dependency( dependency );
        end
    end
    return all;
end

function Toolset:File( identifier, target_prototype )
    local target = Target( self, self:interpolate(identifier), target_prototype );
    target:set_filename( target:path() );
    target:set_cleanable( true );
    target:add_ordering_dependency( self:Directory(branch(target)) );
    return target;
end

function Toolset:SourceFile( identifier )
    local target = identifier;
    if type(target) == 'string' then 
        target = Target( self, self:interpolate(identifier) );
        if target:filename() == '' then 
            target:set_filename( target:path() );
        end
        target:set_cleanable( false );
    end
    return target;
end

function Toolset:SourceDirectory( identifier )
    return self:SourceFile( identifier );
end

Toolset.Target = require 'forge.Target';

Toolset.Directory = require 'forge.Directory';

Toolset.Copy = require 'forge.Copy';

Toolset.CopyDirectory = require 'forge.CopyDirectory';

return Toolset;
