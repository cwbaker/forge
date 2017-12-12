
StaticLibraryPrototype = TargetPrototype { "StaticLibrary" };

function StaticLibraryPrototype.build( static_library )
    if static_library:is_outdated() then
        build_library( static_library );
    end    
end

function StaticLibraryPrototype.clean( static_library )
    clean_library( static_library );
end

function StaticLibrary( id )
    return function( dependencies )
        local static_library = target( id, StaticLibraryPrototype, dependencies );
        build.add_module_dependencies( static_library, lib_name("%s/%s" % {settings.lib, id}), build.current_settings() );
        return static_library;                
    end
end
