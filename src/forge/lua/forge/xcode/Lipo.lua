
local Lipo = forge:FilePrototype( 'Lipo' );

function Lipo.build( forge, target )
    local settings = forge.settings;
    local xcrun = settings.xcode.xcrun;
    local executables = {};
    for _, executable in target:dependencies() do 
        table.insert( executables, executable:filename() );
    end
    printf( leaf(target) );
    system( xcrun, {
        'xcrun'; 
        ('--sdk %s'):format( settings.sdkroot );
        'lipo';
        '-create'; 
        ('-output "%s"'):format( target ); 
        ('"%s"'):format( table.concat(executables, '" "') ); 
    } );
end

return Lipo;
