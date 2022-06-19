
local Target = _G.Target;

function Target.depend( toolset, target, dependencies )
    assert( type(dependencies) == 'table', 'Target.depend() parameter not a table as expected' );
    merge( target, dependencies );
    for _, value in walk_tables(dependencies) do 
        local source_file = toolset:SourceFile( value );
        target:add_dependency( source_file );
    end
end

return Target;
