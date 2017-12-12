
CopyPrototype = TargetPrototype { "Copy" };

function CopyPrototype.build( copy )
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

function Copy( destination )
    build.begin_target();
    return function( source )
        return build.end_target( function()
            local settings = build.current_settings();
            destination = build.interpolate( destination, settings );
            source = build.interpolate( source, settings );
            local copy = target( destination, CopyPrototype );
            copy:set_filename( destination );
            copy:add_dependency( file(source) );
            copy:add_dependency( Directory(branch(destination)) );
            copy.source = source;
            return { copy };
        end );
    end
end
