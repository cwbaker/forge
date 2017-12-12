
local Pdf = build:TargetPrototype( "asciidoc.Pdf" );

function Pdf.build( build, target )
    local settings = target.settings;
    local python = settings.python.executable;
    local arguments = {
        "python",
        settings.a2x.executable,
        "-f target",
        ('-o "%s"'):format( target ),
        ('"%s"'):format( target:dependency() )
    };
    build:system( python, arguments, nil, build:dependencies_filter(target) );
end;
