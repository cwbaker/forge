
local Generate = FilePrototype( 'Generate' );

function Generate.build( toolset, target )
    local dependency = target:dependency();
    local template = assert( loadfile(native(absolute(dependency))) );
    local success, output_or_error_message = pcall( template, toolset, target );
    assert( success, output_or_error_message );
    local output = output_or_error_message;

    local output_file, error_message = io.open( target:filename(), 'wb' );
    assert( output_file, error_message );
    output_file:write( output );
    output_file:close();
    output_file = nil;
end

return Generate;
