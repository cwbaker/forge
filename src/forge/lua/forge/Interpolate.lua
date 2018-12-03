
local Interpolate = forge:FilePrototype( 'Interpolate' );

function Interpolate.depend( forge, target, dependencies )
    assert( type(dependencies) == 'table', 'Target.depend() parameter not a table as expected' );
    forge:merge( target, dependencies );
    for index, template in ipairs(dependencies) do 
        assertf( type(template) == 'string', 'Interpolate.depend() template %d not a string as expected', index );
        table.insert( target, template );
    end
end

function Interpolate.build( forge, target )
    local outputs = {};
    for _, template in ipairs(target) do 
        local output = forge:interpolate( template, target );
        table.insert( outputs, output );
    end
    local output_file, error_message = io.open( target:filename(), 'wb' );
    assert( output_file, error_message );
    output_file:write( table.concat(outputs, '') );
    output_file:close();
    output_file = nil;
end
