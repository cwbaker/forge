
local Pdf = build:TargetPrototype( "pandoc.Pdf" );

function Pdf.build( build, target )
    local settings = target.settings;
    local pandoc = target.settings.pandoc.executable;
    local arguments = {
        "pandoc",
        "--to=pdf",
        ('-o "%s"'):format( target ),
        ('"%s"'):format( target:dependency() )
    };
    local environment = {
        PATH = os.getenv( "PATH" );
    };
    build:system( pandoc, arguments, environment, build:dependencies_filter(target) );
end
