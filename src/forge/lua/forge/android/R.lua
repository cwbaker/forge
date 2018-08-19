
local R = forge:TargetPrototype( 'R' );

function R.create( forge, packages )
    local r = forge:Target( forge:anonymous(), R );
    r.packages = packages;
    for index, package in ipairs(packages) do 
        local filename = forge:generated( ("%s/R.java"):format(package:gsub("%.", "/")), nil, forge.settings );
        r:set_filename( filename, index );
        r:add_ordering_dependency( forge:Directory(forge:branch(filename)) );
    end
    r:add_implicit_dependency( forge:current_buildfile() );
    return r;
end

function R.build( forge, target )
    local android_manifest = target:dependency( 1 );
    assertf( android_manifest and forge:leaf(android_manifest) == "AndroidManifest.xml", "Android R '%s' does not specify 'AndroidManifest.xml' as its first dependency", target:path() );

    local settings = target.settings;
    local working_directory = target:working_directory();
    local gen_directory = ("%s/%s"):format( settings.gen, forge:relative(working_directory:path(), forge:root()) );

    local command_line = {
        'aapt',
        'package',
        '--auto-add-overlay',
        '-f',
        '-m',
        ('-I "%s"'):format( android.android_jar(settings) ),
        ('-J "%s"'):format( gen_directory ),
        ('-M "%s"'):format( android_manifest:filename() ),
        ('--extra-packages %s'):format(table.concat(target.packages, ":"))
    };

    for _, dependency in target:dependencies( 2 ) do
        if dependency:prototype() == forge.Ivy then 
            for _, archive in dependency:implicit_dependencies() do
                if forge:extension(archive) ~= '.jar' and forge:exists(('%s/res'):format(archive)) then 
                    table.insert( command_line, ('-S "%s/res"'):format(archive) );
                end
            end
        else
            table.insert( command_line, ('-S "%s"'):format(forge:relative(dependency)) );
        end
    end

    local aapt = ('%s/aapt'):format( settings.android.build_tools_directory );
    forge:system( aapt, command_line );
end

android.R = R;
