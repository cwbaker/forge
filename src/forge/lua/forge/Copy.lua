
local Copy = build:TargetPrototype( 'Copy' );

function Copy.build( build, target )
    build:rm( target );
    build:cp( target, target:dependency() );
end
