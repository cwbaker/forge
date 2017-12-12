
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
    return function( dependencies )
        local executables = {};
        local settings = build.current_settings();
        for _, architecture in ipairs(settings.architectures) do 
            local executable = target( "%s_%s" % {id, architecture}, ExecutablePrototype, build.copy(dependencies) );
            build.add_module_dependencies( executable, "%s/%s" % {settings.bin, exe_name(id, architecture)}, build.current_settings(), architecture );
            table.insert( executables, executable );
        end
        return executables;
    end
end
