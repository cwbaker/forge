
local Aidl = forge:FilePrototype( 'Aidl' );

function Aidl.build( forge, target )
    local settings = forge.settings;
    local aidl = ('%s/aidl'):format( settings.android.build_tools_directory );
    local platform = ('%s/platforms/%s/framework.aidl'):format( settings.android.sdk_directory, settings.android.sdk_platform );
    local output = forge:generated(target:working_directory(), nil, settings);
    local command_line = {
        'aidl',
        ('-p%s'):format( platform ),
        ('-o"%s"'):format( output ), 
        ('"%s"'):format( target:dependency() )
    };
    forge:system( 
        aidl, 
        command_line
    );
end
