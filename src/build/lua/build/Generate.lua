
GeneratePrototype = TargetPrototype { "Generate" };

function GeneratePrototype.generate( text_file )
    print( leaf(text_file:get_filename()) );
    local template = assert( loadfile(native(absolute(text_file.template))) );
    local success, output = pcall( template, text_file );
    assert( success, "Executing '%s' failed - %s" % {text_file.template, output} );

    local output_file = io.open( text_file:get_filename(), "wb" );
    assert( output_file, "Opening '%s' to write generated text failed" % text_file:get_filename() );
    output_file:write( output );
    output_file:close();
    output_file = nil;
end

function GeneratePrototype.static_depend( text_file )
    local filename = text_file:get_filename();
    if not exists(filename) and platform ~= "" then
        local directory = branch( filename );
        if not exists(directory) then 
            mkdir( directory );
        end
        text_file:generate();
    end
end

function GeneratePrototype.build( text_file )
    if text_file:is_outdated() then
        text_file:generate();
    end
end

function GeneratePrototype.clean( text_file )
    rm( text_file:get_filename() );
end

function Generate( filename )
    build.begin_target();
    return function( definition )
        return build.end_target( function()
            local template;
            if type(definition) == "table" then
                assert( type(definition[1]) == "string", "No source file specified for Generate template definition" );
                template = definition[1];
            else
                assert( type(definition) == "string", "No source file or definition specified for Generate template" );
                template = definition;
                definition = {};
            end
            local template_file = file( template );
            template_file:set_required_to_exist( true );

            local settings = build.current_settings();
            filename = build.interpolate( filename, settings );
            local text_file = file( filename, GeneratePrototype, build.copy(definition) );
            text_file.template = template;
            text_file.settings = settings;
            text_file:add_dependency( template_file );
            text_file:add_dependency( Directory(branch(filename)) );

            return { text_file };
        end );
    end
end
