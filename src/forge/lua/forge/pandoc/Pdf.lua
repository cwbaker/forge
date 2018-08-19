
local Pdf = forge:TargetPrototype( 'Pdf' );

function Pdf.build( forge, target )
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
    forge:system( pandoc, arguments, environment, forge:dependencies_filter(target) );
end
