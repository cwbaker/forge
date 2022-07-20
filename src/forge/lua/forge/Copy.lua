
local Copy = PatternRule( 'Copy' );

function Copy.build( toolset, target )
    rm( target );
    cp( target, target:dependency() );
end

return Copy;
