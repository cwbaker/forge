
local Directory = forge:TargetPrototype( "Directory" );

function Directory.create( forge, settings, identifier )
    local target = forge:Target( forge:interpolate(identifier, settings), Directory );
    target:set_filename( target:path() );
    target:set_cleanable( true );
    return target;
end

function Directory.build( forge, target )
    forge:mkdir( target );
end

function Directory.clean( forge, target )
end
