
local Parser = build.TargetPrototype( "Parser" );

function Parser.create( _, value )
    local grammar = build.SourceFile( value );    
    local parser = build.File( ("%s.hpp"):format(basename(value)), Parser );
    parser.grammar = value;
    parser.settings = build.current_settings();
    parser:add_dependency( grammar );
    return parser;
end

function Parser.static_depend( parser )
    parser:build();
end

function Parser.build( parser )
    local parser_ = parser.settings.parser.executable;
    if not exists(parser:get_filename()) or parser:is_outdated() then
        print( parser.grammar );
        local arguments = ("parser -r parser/cxx %s"):format( parser.grammar );
        local result = execute( parser_, arguments );
        assertf( result == 0, ("[[%s]] failed"):format(arguments) );
    end
end

function Parser.clobber( parser )
    if exists(parser.settings.parser.executable) then    
        rm( parser:get_filename() );
    end
end

_G.Parser = Parser;