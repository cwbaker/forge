
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
        local dynamic_libraries = {};
        local settings = build.current_settings();
        for _, architecture in ipairs(settings.architectures) do 
            local dynamic_library = target( "%s_%s" % {id, architecture}, DynamicLibraryPrototype, build.copy(dependencies) );
            build.add_module_dependencies( dynamic_library, "%s/%s" % {settings.bin, dll_name(id, architecture)}, build.current_settings(), architecture );
            table.insert( dynamic_libraries, dynamic_library );
        end
        return dynamic_libraries;
    end
end
