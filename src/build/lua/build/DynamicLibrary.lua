
DynamicLibraryPrototype = TargetPrototype { "DynamicLibrary" };

function DynamicLibraryPrototype.static_depend( dynamic_library )
    build.add_library_dependencies( dynamic_library );
end

function DynamicLibraryPrototype.build( dynamic_library )
    if dynamic_library:is_outdated() then
        build_executable( dynamic_library );
    end
end

function DynamicLibraryPrototype.clean( dynamic_library )
    clean_executable( dynamic_library );
end

function DynamicLibrary( id )
    return function( dependencies )
        local dynamic_library = target( id, DynamicLibraryPrototype, dependencies );
        build.add_module_dependencies( dynamic_library, dll_name("%s/%s" % {settings.bin, id}), build.current_settings() );
        return dynamic_library;        
    end
end
