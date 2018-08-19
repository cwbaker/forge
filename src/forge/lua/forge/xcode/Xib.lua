
local Xib = forge:TargetPrototype( 'Xib' );

function Xib.build( forge, xib )
    local command_line = {
        'xcrun';
        ('--sdk %s'):format( ios.sdkroot_by_target_and_platform(xib, platform) );
        'ibtool';
        '--output-format binary1';
        ('--compile "%s"'):format( xib );
        ('"%s"'):format( xib:dependency() );
    };
    forge:system( 
        xib.settings.ios.xcrun, 
        command_line
    );
end

xcode.Xib = Xib;
