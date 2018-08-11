
local DynamicLibrary = forge:TargetPrototype( "DynamicLibrary" );

local function default_identifier_filename( settings, identifier, architecture )
    local identifier = forge:absolute( forge:interpolate(identifier, settings) );
    local basename = forge:basename( identifier );
    local branch = forge:branch( identifier );
    local filename = ("%s/%s"):format( branch, settings.dll_name(basename, architecture) );
    return identifier, filename;
end

function DynamicLibrary.create( forge, settings, identifier, architecture )
    local settings = settings or forge:current_settings();
    local architecture = architecture or settings.architecture or settings.default_architecture;
    local identifier, filename = default_identifier_filename( settings, identifier, architecture );
    local dynamic_library = forge:Target( identifier, DynamicLibrary );
    dynamic_library:set_filename( filename );
    dynamic_library:set_cleanable( true );
    dynamic_library:add_ordering_dependency( forge:Directory(forge:branch(dynamic_library)) );
    dynamic_library.settings = settings;
    dynamic_library.architecture = architecture;
    return dynamic_library;
end

function DynamicLibrary.build( forge, target )
    local settings = target.settings;
    settings.build_executable( target );
end
