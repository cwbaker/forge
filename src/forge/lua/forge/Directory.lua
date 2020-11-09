
local Directory = TargetPrototype( 'Directory' );

function Directory.create( toolset, identifier )
    local target = toolset:Target( toolset:interpolate(identifier), Directory );
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
