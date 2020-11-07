
local Toolset = forge.Toolset or {};

function Toolset.new( toolset_prototype, values )
    local settings = forge.Settings():apply( values );
    local identifier = Toolset.interpolate( Toolset, values.identifier or '', values );
    local toolset = add_toolset( identifier, toolset_prototype );
    toolset.settings = settings;
    toolset:install();
    return toolset;
end

function Toolset:clone( values )
    local toolset = {
        settings = forge.Settings()
            :apply( self.settings )
            :apply( values )
        ;
    };
    setmetatable( toolset, {__index = self} );
    return toolset;
end

function Toolset:inherit( values )
    local toolset = {
        settings = self.settings:inherit( values );
    };
    setmetatable( toolset, {__index = self} );
    return toolset;
end

function Toolset:apply( values )
    local settings = self.settings;
    settings:apply( values );
    return self;
end

function Toolset:defaults( values )
    local settings = self.settings;
    settings:defaults( values );
    return self;
end

function Toolset.configure( toolset, module_settings )
    return {};
end

function Toolset.validate( toolset, module_settings )
    return true;
end

function Toolset.initialize( toolset )
    return true;
end

function Toolset:install( toolset_prototype )
    local id = toolset_prototype and tostring( toolset_prototype ) or tostring( self:prototype() );
    local toolset_prototype = toolset_prototype or self;

    local module_settings;
    local configure = toolset_prototype.configure;
    if configure and self:prototype() then
        local local_settings = forge.local_settings;
        module_settings = local_settings[id];
        if not module_settings then
            local settings = self.settings;
            module_settings = configure( self, settings[id] or {} );
            settings[id] = module_settings;
            local_settings[id] = module_settings;
            local_settings.updated = true;
        end
    end

    local validate = toolset_prototype.validate;
    if validate and not validate( self, module_settings ) then
        return;
    end

    local initialize = toolset_prototype.initialize;
    if initialize and not initialize( self ) then
        return;
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

    local settings = self.settings;
    local variables = variables or settings;
    local interpolate = self.interpolate;
    return (template:gsub('%$(%b{})', function(word) 
        local parameters = split( interpolate(self, word:sub(2, -2), variables) );
        local identifier = parameters[1];
        local index = 1;
        local substitute = variables and variables[identifier];
        if not substitute then 
            substitute = settings and settings[identifier];
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

-- Return true if this toolset's platform matches any passed in pattern.
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

-- Get the *all* target for the current working directory adding any 
-- targets that are passed in as dependencies.
function Toolset:all( dependencies )
    local all = forge.Target( self, 'all' );
    if dependencies then 
        for _, dependency in forge:walk_tables(dependencies) do
            if type(dependency) == 'string' then 
                dependency = forge.Target( self, self:interpolate(dependency) );
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

function Toolset:SourceDirectory( identifier )
    return self:SourceFile( identifier );
end

Toolset.Target = require 'forge.Target';

Toolset.Directory = require 'forge.Directory';

Toolset.Copy = require 'forge.Copy';

Toolset.CopyDirectory = require 'forge.CopyDirectory';

return Toolset;
