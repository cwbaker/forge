
local Executable = build:TargetPrototype( "Executable" );

function Executable.create( build, settings, identifier )
    local executable = build:File( settings.exe_name(identifier), Executable, settings );
    executable:add_ordering_dependency( build:Directory(build:branch(executable)) );
    executable.settings = settings;
    executable.architecture = settings.architecture or settings.default_architecture;
    return executable;
end

function Executable.build( build, target )
    local settings = target.settings;
    settings.build_executable( target );
end
