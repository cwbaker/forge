
local Copy = build:TargetPrototype( "Copy" );

function Copy.create( build, settings, identifier )
    local target = build:File( identifier, Copy );
    target:add_ordering_dependency( build:Directory(target:branch()) );
    target.settings = settings;
    return target;
end

function Copy.build( build, target )
    build:rm( target );
    build:cp( target, target:dependency() );
end
