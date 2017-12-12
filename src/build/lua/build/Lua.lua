
LuaPrototype = TargetPrototype { "Lua" };

function LuaPrototype.static_depend( text_file )
    text_file:build();
end

function LuaPrototype.build( text_file )
    if not exists(text_file:get_filename()) or text_file:is_outdated() then
        print( leaf(text_file.template) );
        local success, output = pcall( dofile, native(absolute(text_file.template)) );
        assert( success, "Executing '%s' failed - %s" % {text_file.template, output} );

        local output_file = io.open( text_file:get_filename(), "wb" );
        assert( output_file, "Opening '%s' to write generated text failed" % text_file:get_filename() );
        output_file:write( output );
        output_file:close();
        output_file = nil;
    end
end

function LuaPrototype.clean( text_file )
    rm( text_file:get_filename() );
end

function Lua( filename, template )
    build.begin_target();
    return function( template )
        return build.end_target( function()
            local template_file = file( template );
            template_file:set_required_to_exist( true );

            local text_file = file( filename, LuaPrototype );
            text_file.template = template;
            text_file:add_dependency( template_file );

            return { text_file };
        end );
    end
end
