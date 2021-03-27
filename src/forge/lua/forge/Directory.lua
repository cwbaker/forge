
local Directory = TargetPrototype( 'Directory' );

function Directory.create( toolset, identifier )
    local identifier = toolset:interpolate( identifier );
    local target = toolset:Target( identifier, Directory );
    target:set_filename( target:path() );
    target:set_cleanable( true );
    return target;
end

function Directory.build( toolset, target )
    mkdir( target );
end

function Directory.clean( toolset, target )
end

return Directory;
