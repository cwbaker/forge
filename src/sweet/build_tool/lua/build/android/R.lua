
local R = build:TargetPrototype( "android.R" );

function R.create( build, settings, packages )
    local r = build:Target( build:anonymous(), R );
    r.settings = settings;
    r.packages = packages;
    for index, package in ipairs(packages) do 
        local filename = build:generated( ("%s/R.java"):format(package:gsub("%.", "/")), nil, settings );
        r:set_filename( filename, index );
        r:add_ordering_dependency( build:Directory(build:branch(filename)) );
    end
    r:add_implicit_dependency( build:current_buildfile() );
    return r;
end

function R.build( build, target )
    local android_manifest = target:dependency( 1 );
    assertf( android_manifest and build:leaf(android_manifest) == "AndroidManifest.xml", "Android R '%s' does not specify 'AndroidManifest.xml' as its first dependency", target:path() );

    local settings = target.settings;
    local working_directory = target:working_directory();
    local gen_directory = ("%s/%s"):format( settings.gen, build:relative(working_directory:path(), build:root()) );

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
        if dependency:prototype() == build.Ivy then 
            for _, archive in dependency:implicit_dependencies() do
                if build:extension(archive) == '' then 
                    table.insert( command_line, ('-S "%s/res"'):format(archive:filename()) );
                end
            end
        else
            table.insert( command_line, ('-S "%s"'):format(build:relative(dependency)) );
        end
    end

    local aapt = ('%s/aapt'):format( settings.android.build_tools_directory );
    build:system( aapt, command_line );
end

android.R = R;
