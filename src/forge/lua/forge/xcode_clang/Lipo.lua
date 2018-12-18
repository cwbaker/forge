
local Lipo = forge:FilePrototype( 'Lipo' );

function Lipo.build( forge, lipo )
    local settings = forge.settings;
    settings.lipo_executable( forge, lipo );
end

return Lipo;
