
local Copy = forge:FilePrototype( 'Copy' );

function Copy.build( forge, target )
    rm( target );
    cp( target, target:dependency() );
end

return Copy;
