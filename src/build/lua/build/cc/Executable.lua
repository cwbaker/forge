
local Executable = build.TargetPrototype( "Executable" );

function Executable.create( _, id, architecture, settings )
    local settings = settings or build.current_settings();
    local executable = build.Target( ("%s_%s"):format(id, architecture), Executable );
    executable.settings = settings;
    executable.architecture = architecture;
    executable:set_filename( ("%s/%s"):format(settings.bin, exe_name(id, architecture)) );
    executable:add_dependency( Directory(branch(executable:get_filename())) );
    working_directory():add_dependency( executable );
    return executable;
end

function Executable.call( executable, definition )
    build.merge( executable, definition );
    build.add_library_dependencies( executable, definition.libraries );
    for _, dependency in ipairs(definition) do 
        dependency.module = executable;
        executable:add_dependency( dependency );
    end
end

function Executable.build( executable )
    if executable:is_outdated() then
        build_executable( executable );
    end
end

function Executable.clean( executable )
    clean_executable( executable );
end

_G.Executable = Executable;
