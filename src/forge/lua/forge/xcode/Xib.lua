
local Xib = forge:FilePrototype( 'Xib' );

function Xib.build( forge, target )
    local settings = forge.settings;
    printf( leaf(target) );
    forge:system( settings.xcode.xcrun, {
        'xcrun';
        ('--sdk %s'):format( settings.sdkroot );
        'ibtool';
        '--output-format binary1';
        ('--compile "%s"'):format( target );
        ('"%s"'):format( target:dependency() );
    } );
end

return Xib;
