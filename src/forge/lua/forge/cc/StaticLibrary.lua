
local StaticLibrary = forge:TargetPrototype( "StaticLibrary" );

local function default_identifier_filename( settings, identifier, architecture )
    local identifier = forge:absolute( forge:interpolate(identifier, settings) );
    local basename = forge:basename( identifier );
    local branch = forge:branch( identifier );
    local filename = ("%s/%s"):format( branch, settings.lib_name(basename, architecture) );
    return identifier, filename;
end

function StaticLibrary.create( forge, settings, identifier, architecture )
    local settings = settings or forge:current_settings();
    local architecture = architecture or settings.architecture or settings.default_architecture;
    local identifier, filename = default_identifier_filename( settings, identifier, architecture );
    local static_library = forge:Target( identifier, StaticLibrary );
    static_library:set_filename( filename );
    static_library:set_cleanable( true );
    static_library:add_ordering_dependency( forge:Directory(forge:branch(static_library)) );
    static_library.settings = settings;
    static_library.architecture = architecture;
    return static_library;
end

function StaticLibrary.build( forge, target )
    local settings = target.settings;
    settings.build_library( target );
end
