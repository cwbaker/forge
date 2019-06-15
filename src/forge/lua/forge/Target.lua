
local Target = forge.Target;

function Target.create( toolset, identifier, target_prototype )
    local target = add_target( toolset, identifier, target_prototype, hash(toolset.settings) );
    target.settings = toolset.settings;
    return target;
end

function Target.depend( toolset, target, dependencies )
    assert( type(dependencies) == 'table', 'Target.depend() parameter not a table as expected' );
    toolset:merge( target, dependencies );
    for _, value in forge:walk_tables(dependencies) do 
        local source_file = toolset:SourceFile( value );
        target:add_dependency( source_file );
    end
end

return Target;
