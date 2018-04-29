
local StaticLibrary = build:TargetPrototype( "StaticLibrary" );

local function default_identifier_filename( identifier, platform, architecture, settings )
    local settings = settings or build:current_settings();
    local identifier = build:absolute( build:interpolate(identifier, settings) );
    local basename = build:basename( identifier );
    local branch = build:branch( identifier );
    local filename = ("%s/%s"):format( branch, settings.lib_name(basename, architecture) );
    return identifier, filename;
end

function StaticLibrary.create( build, settings, identifier, architecture )
    local identifier, filename = default_identifier_filename( identifier, platform, architecture, settings );
    local static_library = build:Target( identifier, StaticLibrary );
    static_library:set_filename( filename );
    static_library:set_cleanable( true );
    static_library:add_ordering_dependency( build:Directory(build:branch(static_library)) );
    static_library.settings = settings;
    static_library.architecture = settings.architecture or settings.default_architecture;
    return static_library;
end

function StaticLibrary.build( build, target )
    local settings = target.settings;
    settings.build_library( target );
end
