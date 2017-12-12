
local CopyDirectory = build.TargetPrototype( "CopyDirectory" );

function CopyDirectory.create( settings, destination )
    destination = build.interpolate( destination, settings );
    local copy_directory = build.Target( build.anonymous(), CopyDirectory );
    copy_directory.directory = build.absolute( destination );
    return copy_directory;
end

function CopyDirectory.call( copy_directory, source_directory )
    local settings = build.current_settings();
    source_directory = build.interpolate( source_directory, settings );

    local cache = build.find_target( settings.cache );
    cache:add_dependency( build.SourceDirectory(source_directory) );

    build.pushd( source_directory );
    for source_filename in build.find("") do 
        if build.is_file(source_filename) then
            local filename = build.absolute( build.relative(source_filename), copy_directory.directory );
            local copy = build.Copy (filename) (source_filename);
            copy_directory:add_dependency( copy );
        elseif build.is_directory(source_filename) then 
            local directory = build.SourceDirectory( source_filename );
            cache:add_dependency( directory );
        end
    end
    build.popd();
end
