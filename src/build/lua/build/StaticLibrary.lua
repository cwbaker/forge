
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
    build.begin_target();
    return function( dependencies )
        return build.end_target( function()
            local static_libraries = {};
            local settings = build.push_settings( dependencies.settings );
            for _, architecture in ipairs(settings.architectures) do 
                local static_library = target( module_name(id, architecture), StaticLibraryPrototype, build.copy(dependencies) );
                build.add_module_dependencies( static_library, "%s/%s" % {settings.lib, lib_name(id, architecture)}, architecture, settings );
                table.insert( static_libraries, static_library );
            end
            build.pop_settings();            
            return static_libraries;
        end );
    end
end
