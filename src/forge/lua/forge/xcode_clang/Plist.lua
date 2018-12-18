
local Plist = forge:FilePrototype( 'Plist' );

function Plist.build( forge, plist )
    local command_line = {
        'plutil';
        '-convert binary1';
        ('-o "%s"'):format( plist );
        ('"%s"'):format( plist:dependency() );
    };
    local plutil = plist.settings.ios.plutil;
    forge:system( plutil, table.concat(command_line, " ") );
end

return Plist;
