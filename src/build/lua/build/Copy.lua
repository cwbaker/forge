
Copy = build.TargetPrototype( "Copy" );

function Copy.create( _, destination, source, settings )
    local settings = settings or build.current_settings();
    destination = build.interpolate( destination, settings );
    source = build.interpolate( source, settings );
    local copy = build.File( destination, Copy );
    copy:add_dependency( file(source) );
    copy:add_dependency( Directory(branch(destination)) );
    copy.source = source;
    return copy;
end

function Copy.build( copy )
    if copy:is_outdated() then
        if is_file(copy.source) then 
            rm( copy:get_filename() );
            cp( copy.source, copy:get_filename() );
        else
            rmdir( copy:get_filename() );
            cpdir( copy.source, copy:get_filename() );
        end
    end
end
