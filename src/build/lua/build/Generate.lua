
local Generate = build.TargetPrototype( "Generate" );

function Generate.build( generate )
    local outputs = {};
    for _, dependency in generate:dependencies() do 
        local template = assert( loadfile(build.native(build.absolute(dependency))) );
        local success, output_or_error_message = pcall( template, generate );
        assert( success, output_or_error_message );
        table.insert( outputs, output_or_error_message );
    end

    local output_file, error_message = io.open( generate:filename(), "wb" );
    assert( output_file, error_message );
    output_file:write( table.concat(outputs, "") );
    output_file:close();
    output_file = nil;
end
