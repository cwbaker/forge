
local Lipo = forge:TargetPrototype( 'Lipo' );

function Lipo.build( forge, lipo )
    local settings = forge.settings;
    settings.lipo_executable( lipo );
end

xcode.Lipo = Lipo;
