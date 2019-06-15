
local Plist = forge:FilePrototype( 'Plist' );

function Plist.build( forge, plist )
	local settings = forge.settings;
    local plutil = settings.xcode.plutil;
    system( plutil, {
        'plutil';
        '-convert binary1';
        ('-o "%s"'):format( plist );
        ('"%s"'):format( plist:dependency() );
    } );
end

return Plist;
