
local Directory = forge:TargetPrototype( 'Directory' );

function Directory.create( forge, identifier )
    local target = forge:Target( forge:interpolate(identifier), Directory );
    target:set_filename( target:path() );
    target:set_cleanable( true );
    return target;
end

function Directory.build( forge, target )
    mkdir( target );
end

function Directory.clean( forge, target )
end

return Directory;
