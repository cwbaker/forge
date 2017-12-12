
local Executable = build:TargetPrototype( "Executable" );

local function default_identifier_filename( identifier, architecture, settings )
    local settings = settings or build:current_settings();
    local identifier = build:interpolate( identifier, settings );
    local basename = build:basename( identifier );
    local branch = settings.bin;
    if build:is_absolute(identifier) then 
        branch = build:branch( identifier );
    end
    local filename = ("%s/%s"):format( branch, settings.exe_name(basename, architecture) );
    local identifier = ("%s/%s"):format( branch, basename );
    return identifier, filename;
end

function Executable.create( build, settings, identifier, architecture )
    local architecture = architecture or settings.architecture or settings.default_architecture;
    local identifier, filename = default_identifier_filename( identifier, architecture, settings );
    local executable = build:Target( identifier, Executable );
    executable:set_filename( filename );
    executable:set_cleanable( true );
    executable:add_ordering_dependency( build:Directory(build:branch(executable)) );
    executable.settings = settings;
    executable.architecture = architecture or settings.default_architecture;
    return executable;
end

function Executable.depend( build, target, dependencies )
    local libraries = dependencies.libraries;
    if libraries and platform ~= "" then
        local platform = target.settings.platform;
        local architecture = target.architecture;
        for _, value in ipairs(libraries) do
            local library = ("%s_%s_%s"):format( value, platform, architecture );
            target:add_dependency( build:target(build:root(library)) );
        end
    end
    return build.Target.depend( build, target, dependencies );
end

function Executable.build( build, target )
    local settings = target.settings;
    settings.build_executable( target );
end
