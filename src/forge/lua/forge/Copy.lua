
local Copy = forge:TargetPrototype( 'Copy' );

function Copy.build( forge, target )
    forge:rm( target );
    forge:cp( target, target:dependency() );
end
