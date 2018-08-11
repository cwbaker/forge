
local Css = forge:TargetPrototype( "sass.Css" );

function Css.build( forge, target )
    local settings = target.settings;

    local load_paths = {};
    if settings.sass.load_paths then
        for _, directory in ipairs(settings.load_paths) do
            table.insert( load_paths, ('-I "%s"'):format(directory) );
        end
    end
    if target.load_paths then
        for _, directory in ipairs(target.load_paths) do
            table.insert( load_paths, ('-I "%s"'):format(directory) );
        end
    end

    local ruby = settings.ruby.executable;
    local arguments = {
        ruby,
        settings.sass.executable,
        table.concat( load_paths, " " ),
        ('--cache-location "%s"'):format( forge:interpolate("${obj}/.sass-cache", settings) ),
        ('"%s"'):format( target:dependency():filename() ),
        ('"%s"'):format( target:filename() ),
    };
    local environment = {
        PATH = os.getenv( "PATH" );
    };
    forge:system( ruby, arguments, environment, forge:dependencies_filter(target) );
end
