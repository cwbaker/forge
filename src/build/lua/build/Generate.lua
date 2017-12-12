
Generate = build.TargetPrototype( "Generate" );

function Generate.create( settings, filename, template )
    local template_file = file( template );
    template_file:set_required_to_exist( true );

    filename = build.interpolate( filename, settings );
    local generated_file = file( filename, Generate );
    generated_file.template = template;
    generated_file.settings = settings;
    generated_file:add_dependency( template_file );
    generated_file:add_dependency( Directory(generated_file:branch()) );

    return generated_file;
end

function Generate.generate( generated_file )
    print( leaf(generated_file:filename()) );
    local template = assert( loadfile(native(absolute(generated_file.template))) );
    local success, output = pcall( template, generated_file );
    assertf( success, "Executing '%s' failed - %s", generated_file.template, output );

    local output_file = io.open( generated_file:filename(), "wb" );
    assertf( output_file, "Opening '%s' to write generated text failed", generated_file:filename() );
    output_file:write( output );
    output_file:close();
    output_file = nil;
end

function Generate.static_depend( generated_file )
    local filename = generated_file:filename();
    if not exists(filename) and platform ~= "" then
        local directory = branch( filename );
        if not exists(directory) then 
            mkdir( directory );
        end
        generated_file:generate();
    end
end

function Generate.build( generated_file )
    if generated_file:outdated() then
        generated_file:generate();
    end
end

function Generate.clean( generated_file )
    rm( generated_file:filename() );
end
