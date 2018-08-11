
local Executable = forge:TargetPrototype( "Executable" );

function Executable.create( forge, settings, identifier )
    local executable = forge:File( settings.exe_name(identifier), Executable, settings );
    executable:add_ordering_dependency( forge:Directory(forge:branch(executable)) );
    executable.settings = settings;
    executable.architecture = settings.architecture or settings.default_architecture;
    return executable;
end

function Executable.build( forge, target )
    local settings = target.settings;
    settings.build_executable( target );
end
