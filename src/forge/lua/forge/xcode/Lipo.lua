
local Lipo = forge:TargetPrototype( "xcode.Lipo" );

function Lipo.build( forge, lipo )
    local settings = lipo.settings;
    settings.lipo_executable( lipo );
end

xcode.Lipo = Lipo;
