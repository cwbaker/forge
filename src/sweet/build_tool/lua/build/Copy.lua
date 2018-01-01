
local Copy = build:TargetPrototype( "Copy" );

function Copy.create( build, settings, identifier )
    local copy = build:File( identifier, Copy );
    copy:add_ordering_dependency( build:Directory(copy:branch()) );
    copy.settings = settings;
    return copy;
end

function Copy.build( build, target )
    build:rm( target );
    build:cp( target, target:dependency() );
end
