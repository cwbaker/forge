
local Copy = build.TargetPrototype( "Copy" );

function Copy.create( settings, destination, source )
    destination = build.interpolate( destination, settings );
    source = build.interpolate( source, settings );
    local copy = build.File( destination, Copy );
    copy:add_dependency( build.file(source) );
    copy:add_dependency( Directory(build.branch(destination)) );
    copy.source = source;
    return copy;
end

function Copy.build( copy )
    if copy:outdated() then
        print( build.leaf(copy:filename()) );
        if build.is_file(copy.source) then 
            build.rm( copy:filename() );
            build.cp( copy.source, copy:filename() );
        end
    end
end

function Copy.clean( copy )
    if build.is_file(copy.source) then 
        build.rm( copy:filename() );
    end
end
