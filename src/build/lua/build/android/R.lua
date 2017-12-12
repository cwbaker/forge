
local R = build.TargetPrototype( "android.R" );

function R.create( settings, definition )
    local working_directory = build.working_directory();
    local gen_directory = ("%s/%s"):format( settings.gen, build.relative(working_directory:path(), build.root()) );
    local resource = build.Target( build.anonymous(), R );
    resource.settings = settings;
    resource.gen_directory = gen_directory;
    resource.packages = definition.packages;
    resource:call( definition, settings );
    return resource;
end

function R.call( resource, definition, settings )
    local settings = settings or build.current_settings();
    local working_directory = build.working_directory();
    local gen_directory = ("%s/%s"):format( settings.gen, build.relative(working_directory:path(), build.root()) );
    for _, package in ipairs(definition.packages) do
        local r_java = build.File( ("%s/%s/R.java"):format(gen_directory, string.gsub(package, "%.", "/")) );
        r_java.settings = settings;
        r_java:add_ordering_dependency( Directory(r_java:directory()) );
        r_java:set_built( true );
        resource:add_dependency( r_java );
    end
    local android_manifest = definition.android_manifest;
    if android_manifest then 
        resource:add_dependency( android_manifest );
        resource.android_manifest = android_manifest;
    end
    for _, directory in ipairs(definition) do 
        resource:add_dependency( build.SourceDirectory(directory) );
    end
end

function R.generate( resource )
    local android_manifest = resource.android_manifest;
    assertf( android_manifest, "Android R '%s' does not specify 'android_manifest'", resource:path() );

    local flags = {
        "package",
        "--auto-add-overlay",
        "-f",
        "-m",
        ([[-I "%s/platforms/%s/android.jar"]]):format( resource.settings.android.sdk_directory, resource.settings.android.sdk_platform ),
        ([[-J "%s"]]):format( resource.gen_directory ),
        ('-M "%s"'):format( android_manifest:filename() )
    };

    if resource.packages then 
        table.insert( flags, ("--extra-packages %s"):format(table.concat(resource.packages, ":")) );
    end

    local i = 1;
    local dependency = resource:dependency( i );
    while dependency do 
        if build.extension(dependency:filename()) == "" then
            table.insert( flags, ('-S "%s"'):format(build.relative(dependency:filename())) );
        end
        i = i + 1;
        dependency = resource:dependency( i );
    end

    print( ("%s/*/R.java"):format(resource.gen_directory) );
    local aapt = ("%s/aapt"):format( resource.settings.android.build_tools_directory );
    build.system( aapt, ([[aapt %s]]):format(table.concat(flags, " ")) );
end

function R.build( resource )
    if resource:outdated() then
        resource:generate();
    end
end

function R.clobber( resource )
    build.rmdir( resource:filename() );
end

android.R = R;
