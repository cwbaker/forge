
local CopyDirectory = build.TargetPrototype( "CopyDirectory" );

function CopyDirectory.create( settings, destination, source, include_filter, exclude_filter )
    source = build.interpolate( source, settings );
    destination = build.interpolate( destination, settings );

    local cache = build.find_target( build.current_settings().cache );
    assertf( cache, "The target for the cache file at '%s' was not found", settings.cache );

    local directory = build.SourceDirectory( source );
    cache:add_dependency( directory );

    local copy_directory = build.Target( build.anonymous(), CopyDirectory );
    build.pushd( source );
    for filename in build.find("") do 
        if build.is_file(filename) then
            local destination_filename = build.absolute( build.relative(filename), destination );
            local copy = build.Copy( destination_filename, filename );
            copy_directory:add_dependency( copy );
        elseif build.is_directory(filename) then 
            local directory = build.SourceDirectory( filename );
            cache:add_dependency( directory );
        end
    end
    build.popd();
    return copy_directory;
end
