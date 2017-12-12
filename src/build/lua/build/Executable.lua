
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
        local executable = target( id, ExecutablePrototype, dependencies );
        build.add_module_dependencies( executable, exe_name("%s/%s" % {settings.bin, id}), build.current_settings() );
        return executable;
    end
end
