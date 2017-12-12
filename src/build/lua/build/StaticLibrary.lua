
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
        local static_libraries = {};
        local settings = build.current_settings();
        for _, architecture in ipairs(settings.architectures) do 
            local static_library = target( "%s_%s" % {id, architecture}, StaticLibraryPrototype, build.copy(dependencies) );
            build.add_module_dependencies( static_library, "%s/%s" % {settings.lib, lib_name(id, architecture)}, build.current_settings(), architecture );
            table.insert( static_libraries, static_library );
        end
        return static_libraries;
    end
end
