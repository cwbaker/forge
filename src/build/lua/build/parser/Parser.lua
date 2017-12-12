
local Parser = build:TargetPrototype( "Parser" );

function Parser.build( build, target )
    -- Quietly do nothing if the parser executable is not available.
    local settings = target.settings;
    if build:exists(settings.parser.executable) then
        local command_line = {
            'parser',
            '-r parser/cxx',
            ('"%s"'):format( build:relative(target:dependency()) )
        };
        local environment = {
            LUA_PATH = settings.parser.lua_path;
        };
        local parser = settings.parser.executable;
        build:system( parser, command_line, environment );
    end
end

function Parser.clean( build, target )
end

function Parser.clobber( build, target )
    local settings = target.settings;
    if build:exists(settings.parser.executable) then    
        build:rm( target );
    end
end
