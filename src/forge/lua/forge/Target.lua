
local Target = forge.Target;

function Target.create( forge, identifier, target_prototype )
    local target = forge:add_target( identifier, target_prototype, forge.hash and forge:hash(forge.settings) );
    target.settings = forge.settings;
    return target;
end

function Target.depend( forge, target, dependencies )
    assert( type(dependencies) == 'table', 'Target.depend() parameter not a table as expected' );
    forge:merge( target, dependencies );
    for _, value in forge:walk_tables(dependencies) do 
        local source_file = forge:SourceFile( value );
        target:add_dependency( source_file );
    end
end

return Target;
