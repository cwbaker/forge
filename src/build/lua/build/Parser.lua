
ParserPrototype = TargetPrototype { "Parser" };

function ParserPrototype.static_depend( parser )
    parser:build();
end

function ParserPrototype.build( parser )
    local parser_ = parser.settings.parser.executable;
    if not exists(parser:get_filename()) or parser:is_outdated() then
        print( parser.grammar );
        local arguments = "parser -r parser/cxx %s" % parser.grammar;
        if exec(parser_, arguments) ~= 0 then
            error( "%s failed" % arguments );
        end
    end
end

function ParserPrototype.clobber( parser )
    if exists(parser.settings.parser.executable) then    
        rm( parser:get_filename() );
    end
end

function Parser( definition )
    assert( type(definition) == "table" );
    assert( #definition == 1 );

    local value = definition[1];
    local grammar = file( value );    
    grammar:set_required_to_exist( true );

    local parser = target( "%s.hpp" % basename(value), ParserPrototype );
    parser.grammar = value;
    parser.settings = build.current_settings();
    parser:add_dependency( grammar );

    return parser;
end

parser = {};

function parser.configure( settings )
    local local_settings = build.local_settings;
    if not local_settings.parser then
        local_settings.updated = true;
        if operating_system() == "windows" then
            local_settings.parser = {
                executable = "d:/usr/local/bin/parser.exe";
                lua_path = "d:/usr/local/lua/?.lua";
            };
        else
            local_settings.parser = {
                executable = "/usr/local/bin/parser.exe";
                lua_path = "/usr/local/lua/?.lua";
            };
        end
    end
end

function parser.initialize( settings )
    parser.configure( settings );
    putenv( "LUA_PATH", settings.parser.lua_path );
end
