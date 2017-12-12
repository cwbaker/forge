
local R = build.TargetPrototype( "android.R" );

function R.create( settings, definition )
    local gen_directory = ("%s/%s"):format( settings.gen, relative(working_directory():path(), root()) );
    local resource = build.Target( anonymous(), R );
    resource.settings = settings;
    resource.gen_directory = gen_directory;
    resource.packages = definition.packages;
    resource:call( definition, settings );
    return resource;
end

function R.call( resource, definition, settings )
    local settings = settings or build.current_settings();
    local gen_directory = ("%s/%s"):format( settings.gen, relative(working_directory():path(), root()) );
    for _, package in ipairs(definition.packages) do
        r_java = build.File( ("%s/%s/R.java"):format(gen_directory, string.gsub(package, "%.", "/")) );
        r_java.settings = settings;
        r_java:add_dependency( Directory(r_java:branch()) );
        resource:add_dependency( r_java );
    end
    for _, directory in ipairs(definition) do 
        table.insert( resource, directory );
    end
end

function R.generate( resource )
    local flags = {
        "package",
        "--auto-add-overlay",
        "-f",
        "-m",
        ([[-I "%s/platforms/%s/android.jar"]]):format( resource.settings.android.sdk_directory, resource.settings.android.sdk_platform ),
        ([[-J "%s"]]):format( resource.gen_directory ),
        "-M AndroidManifest.xml",
    };

    if resource.packages then 
        table.insert( flags, ("--extra-packages %s"):format(table.concat(resource.packages, ":")) );
    end

    for _, directory in ipairs(resource) do 
        table.insert( flags, ([[-S "%s"]]):format(directory) );
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

function R.clean( resource )
    for dependency in resource:dependencies() do 
        rm( dependency:filename() );
    end
end

function R.clobber( resource )
    rmdir( resource:filename() );
end

android.R = R;
