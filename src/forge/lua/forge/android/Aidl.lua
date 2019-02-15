
local Aidl = forge:JavaStylePrototype( 'Aidl' );

function Aidl.build( forge, target )
    local settings = forge.settings;
    local command_line = {
        'aidl',
        ('-p%s'):format( ('%s/platforms/%s/framework.aidl'):format(settings.android.sdk_directory, settings.android.sdk_platform) ),
        ('-o"%s"'):format( target:ordering_dependency() ),
        ('"%s"'):format( target:dependency() )
    };
    local aidl = ('%s/aidl'):format( settings.android.build_tools_directory );
    forge:system( 
        aidl, 
        command_line,
        nil,
        forge:filenames_filter(target)
    );
end

return Aidl;
