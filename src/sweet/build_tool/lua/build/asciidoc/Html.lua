
local Html = build:TargetPrototype( "asciidoc.Html" );

function Html.build( build, target )
    local settings = target.settings;
    local python = settings.python.executable;
    local arguments = {
        "python",
        settings.asciidoc.executable,
        ('-f "%s"'):format( settings.asciidoc.conf_file ),
        ('-o "%s"'):format( target ),
        ('"%s"'):format( target:dependency() )
    };
    local environment = {
        PATH = os.getenv( "PATH" );
    };
    build:system( python, arguments, environment, build:dependencies_filter(target) );
end
