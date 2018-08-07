
local DynamicLibrary = build:TargetPrototype( "DynamicLibrary" );

local function default_identifier_filename( settings, identifier, architecture )
    local identifier = build:absolute( build:interpolate(identifier, settings) );
    local basename = build:basename( identifier );
    local branch = build:branch( identifier );
    local filename = ("%s/%s"):format( branch, settings.dll_name(basename, architecture) );
    return identifier, filename;
end

function DynamicLibrary.create( build, settings, identifier, architecture )
    local settings = settings or build:current_settings();
    local architecture = architecture or settings.architecture or settings.default_architecture;
    local identifier, filename = default_identifier_filename( settings, identifier, architecture );
    local dynamic_library = build:Target( identifier, DynamicLibrary );
    dynamic_library:set_filename( filename );
    dynamic_library:set_cleanable( true );
    dynamic_library:add_ordering_dependency( build:Directory(build:branch(dynamic_library)) );
    dynamic_library.settings = settings;
    dynamic_library.architecture = architecture;
    return dynamic_library;
end

function DynamicLibrary.build( build, target )
    local settings = target.settings;
    settings.build_executable( target );
end
