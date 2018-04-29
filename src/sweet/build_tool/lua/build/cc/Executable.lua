
local Executable = build:TargetPrototype( "Executable" );

local function default_identifier_filename( identifier, settings )
    local settings = settings or build:current_settings();
    local identifier = build:absolute( build:interpolate(identifier, settings) );
    local basename = build:basename( identifier );
    local branch = build:branch( identifier );
    local filename = ("%s/%s"):format( branch, settings.exe_name(basename) );
    return identifier, filename;
end

function Executable.create( build, settings, identifier )
    local identifier, filename = default_identifier_filename( identifier, settings );
    local executable = build:Target( identifier, Executable );
    executable:set_filename( filename );
    executable:set_cleanable( true );
    executable:add_ordering_dependency( build:Directory(build:branch(executable)) );
    executable.settings = settings;
    executable.architecture = settings.architecture or settings.default_architecture;
    return executable;
end

function Executable.build( build, target )
    local settings = target.settings;
    settings.build_executable( target );
end
