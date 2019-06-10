
local android = require 'forge.android';

local R = forge:JavaStylePrototype( 'R' );

function R.build( forge, target )
    local android_manifest = target:dependency( 1 );
    assertf( android_manifest and leaf(android_manifest) == "AndroidManifest.xml", "Android R '%s' does not specify 'AndroidManifest.xml' as its first dependency", target:path() );

    local command_line = {
        'aapt',
        'package',
        '--auto-add-overlay',
        '-f',
        '-m',
        ('-I "%s"'):format( android.android_jar(forge) ),
        ('-J "%s"'):format( target:ordering_dependency() ),
        ('-M "%s"'):format( android_manifest:filename() ),
        ('--extra-packages %s'):format(table.concat(target.packages, ":"))
    };

    for _, dependency in target:dependencies( 2 ) do
        if dependency:prototype() == forge.Ivy then 
            for _, archive in dependency:implicit_dependencies() do
                if extension(archive) ~= '.jar' and exists(('%s/res'):format(archive)) then 
                    table.insert( command_line, ('-S "%s/res"'):format(archive) );
                end
            end
        else
            table.insert( command_line, ('-S "%s"'):format(relative(dependency)) );
        end
    end

    local aapt = ('%s/aapt'):format( forge.settings.android.build_tools_directory );
    forge:system( aapt, command_line, nil, forge:filenames_filter(target) );
end

return R;
