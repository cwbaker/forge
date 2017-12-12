
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
    build.begin_target();
    return function( dependencies )
        return build.end_target( function()
            local dynamic_libraries = {};
            local settings = build.push_settings( dependencies.settings );
            if build.built_for_platform_and_variant(settings) then
                for _, architecture in ipairs(settings.architectures) do 
                    local dynamic_library = target( module_name(id, architecture), DynamicLibraryPrototype, build.copy(dependencies) );
                    build.add_module_dependencies( dynamic_library, "%s/%s" % {settings.bin, dll_name(id, architecture)}, architecture );
                    table.insert( dynamic_libraries, dynamic_library );
                end
            end
            build.pop_settings();
            return dynamic_libraries;
        end);
    end
end
