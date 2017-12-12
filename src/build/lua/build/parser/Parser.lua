
local Parser = build.TargetPrototype( "Parser" );

function Parser.create( settings, value )
    local grammar = build.SourceFile( value );    
    local parser = build.File( ("%s.hpp"):format(basename(value)), Parser );
    parser.grammar = value;
    parser.settings = settings;
    parser:add_dependency( grammar );
    parser:build();
    return parser;
end

function Parser.build( parser )
    local parser_ = parser.settings.parser.executable;
    if not exists(parser:filename()) or parser:outdated() then
        print( parser.grammar );
        local arguments = ("parser -r parser/cxx %s"):format( parser.grammar );
        local result = execute( parser_, arguments );
        local environment = {
            LUA_PATH = settings.parser.lua_path;
        };
        assertf( result == 0, ("[[%s]] failed"):format(arguments), environment );
    end
end

function Parser.clean( parser )
end

function Parser.clobber( parser )
    if exists(parser.settings.parser.executable) then    
        rm( parser:filename() );
    end
end
