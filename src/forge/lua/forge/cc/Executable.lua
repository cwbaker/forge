
local Executable = forge:TargetPrototype( "Executable" );

function Executable.create( forge, identifier )
    local executable = forge:File( settings.exe_name(identifier), Executable );
    executable.architecture = settings.architecture or settings.default_architecture;
    return executable;
end

function Executable.build( forge, target )
    local settings = target.settings;
    settings.build_executable( target );
end
