
local Executable = build.TargetPrototype( "Executable" );

function Executable.create( settings, id, architecture )
    local executable = build.Target( id, Executable );
    executable.settings = settings;
    executable.architecture = architecture or settings.default_architecture;
    executable:set_filename( ("%s/%s"):format(settings.bin, exe_name(id, architecture)) );
    executable:add_dependency( Directory(build.branch(executable:filename())) );
    build.default_target( executable );
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
    if executable:outdated() then
        build_executable( executable );
    end
end

function Executable.clean( executable )
    clean_executable( executable );
end
