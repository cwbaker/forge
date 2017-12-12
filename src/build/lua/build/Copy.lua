
local Copy = build.TargetPrototype( "Copy" );

function Copy.create( settings, destination, source )
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
            rm( copy:filename() );
            cp( copy.source, copy:filename() );
        else
            rmdir( copy:filename() );
            cpdir( copy.source, copy:filename() );
        end
    end
end

function Copy.clean( copy )
    if is_file(copy.source) then 
        rm( copy:filename() );
    else
        rmdir( copy:filename() );
    end
end

_G.Copy = Copy;
