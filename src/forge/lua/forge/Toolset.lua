
local Toolset = forge.Toolset or {};

function Toolset:create( settings )
    local toolset = {
        settings = settings or {};
    };
    setmetatable( toolset, {
        __index = self;
    } );
    return toolset;
end

function Toolset:clone( settings_to_apply )
    local settings = {};
    self:copy_settings( settings, self.settings );
    self:copy_settings( settings, settings_to_apply );
    return self:create( settings );
end

function Toolset:inherit( settings )
    local settings = settings or {};
    setmetatable( settings, {
        __index = self.settings;
    } );
    return self:create( settings );
end

-- Append values from /value/ to /values/.
function Toolset:append( values, value )
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
function Toolset:merge( destination, source )
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
function Toolset:defaults( destination, source )
    local destination = destination or {};
    for key, value in pairs(source) do 
        if type(key) == 'string' and destination[key] == nil then 
            destination[key] = value;
        end
    end
    return destination;
end

function Toolset:copy_settings( destination, source )
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

-- Provide GNU Make like string substitution.
function Toolset:interpolate( template, variables )
    local function split( input )
        local output = {};
        for value in input:gmatch('%S+') do 
            table.insert( output, value );
        end
        return output;
    end

    local settings = self.settings;
    local variables = variables or settings;
    local interpolate = self.interpolate;
    return (template:gsub('%$(%b{})', function(word) 
        local parameters = split( interpolate(self, word:sub(2, -2), variables) );
        local identifier = parameters[1];
        local index = 1;
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

-- Add dependencies detected by the injected build hooks library to the 
-- target /target/.
function Toolset:dependencies_filter( target )
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
function Toolset:filenames_filter( target )
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

function Toolset:platform_matches( ... )
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

-- Convert a target into the object directory that it should build to.
function Toolset:obj_directory( target )
    local relative_path = relative( target:working_directory():path(), root() );
    return absolute( relative_path, self.settings.obj );
end

-- Recursively copy files from *source* to *destination*.
function Toolset:cpdir( destination, source, settings )
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

function Toolset:configure( module, identifier )
    local local_settings = forge.local_settings;
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

-- Get the *all* target for the current working directory adding any 
-- targets that are passed in as dependencies.
function Toolset:all( dependencies )
    local all = add_target( self, 'all' );
    if dependencies then 
        for _, dependency in forge:walk_tables(dependencies) do
            if type(dependency) == 'string' then 
                dependency = add_target( self, self:interpolate(dependency) );
            end
            all:add_dependency( dependency );
        end
    end
    return all;
end

function Toolset:File( identifier, target_prototype )
    local target = forge.Target( self, self:interpolate(identifier), target_prototype );
    target:set_filename( target:path() );
    target:set_cleanable( true );
    target:add_ordering_dependency( self:Directory(branch(target)) );
    return target;
end

function Toolset:SourceFile( identifier )
    local target = identifier;
    if type(target) == 'string' then 
        target = forge.Target( self, self:interpolate(identifier) );
        if target:filename() == '' then 
            target:set_filename( target:path() );
        end
        target:set_cleanable( false );
    end
    return target;
end

function Toolset:SourceDirectory( identifier, settings )
    return self:SourceFile( identifier, settings );
end

Toolset.Target = require 'forge.Target';

Toolset.Directory = require 'forge.Directory';

Toolset.Copy = forge:PatternElement( require('forge.Copy') );

Toolset.CopyDirectory = require 'forge.CopyDirectory';

return Toolset;
