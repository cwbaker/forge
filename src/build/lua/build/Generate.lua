
local Generate = build.TargetPrototype( "Generate" );

function Generate.create( settings, filename, template )
    local template_file = build.file( template );
    template_file:set_required_to_exist( true );

    filename = build.interpolate( filename, settings );
    local generated_file = build.file( filename, Generate );
    generated_file.template = template;
    generated_file.settings = settings;
    generated_file:add_dependency( template_file );
    generated_file:add_dependency( build.Directory(generated_file:branch()) );
    generated_file:build();
    return generated_file;
end

function Generate.build( generated_file )
    if not build.exists(generated_file:filename()) or generated_file:outdated() then
        print( build.leaf(generated_file:filename()) );
        local template = assert( loadfile(build.native(build.absolute(generated_file.template))) );
        local success, output = pcall( template, generated_file );
        assertf( success, "Executing '%s' failed - %s", generated_file.template, output );

        local directory = generated_file:branch();
        if not build.exists(directory) then 
            build.mkdir( directory );
        end

        local output_file = io.open( generated_file:filename(), "wb" );
        assertf( output_file, "Opening '%s' to write generated text failed", generated_file:filename() );
        output_file:write( output );
        output_file:close();
        output_file = nil;
    end
end

function Generate.clean( generated_file )
    build.rm( generated_file:filename() );
end
