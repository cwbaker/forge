
local Generate = forge:TargetPrototype( "Generate" );

function Generate.build( forge, target )
    local outputs = {};
    for _, dependency in target:dependencies() do 
        local template = assert( loadfile(forge:native(forge:absolute(dependency))) );
        local success, output_or_error_message = pcall( template, target );
        assert( success, output_or_error_message );
        table.insert( outputs, output_or_error_message );
    end

    local output_file, error_message = io.open( target:filename(), "wb" );
    assert( output_file, error_message );
    output_file:write( table.concat(outputs, "") );
    output_file:close();
    output_file = nil;
end
