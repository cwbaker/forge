
local Target = forge.Target;

function Target.depend( forge, target, dependencies )
    assert( type(dependencies) == 'table', 'Table.depend() parameter not a table as expected' );
    forge:merge( target, dependencies );
    local settings = target.settings;
    for _, value in ipairs(dependencies) do 
        local source_file = forge:SourceFile( value, settings );
        target:add_dependency( source_file );
    end
end