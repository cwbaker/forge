
ExecutablePrototype = TargetPrototype { "Executable" };

function ExecutablePrototype.static_depend( executable )
    build.add_library_dependencies( executable );
end

function ExecutablePrototype.build( executable )
    if executable:is_outdated() then
        build_executable( executable );
    end
end

function ExecutablePrototype.clean( executable )
    clean_executable( executable );
end

function Executable( id )
    build.begin_target();
    return function( dependencies )
        return build.end_target( function()
            local executables = {};
            local settings = build.push_settings( dependencies.settings );
            if build.built_for_platform_and_variant(settings) then
                for _, architecture in ipairs(settings.architectures) do 
                    local executable = target( module_name(id, architecture), ExecutablePrototype, build.copy(dependencies) );
                    build.add_module_dependencies( executable, "%s/%s" % {settings.bin, exe_name(id, architecture)}, architecture );
                    table.insert( executables, executable );
                end
            end
            build.pop_settings();
            return executables;
        end);
    end
end
