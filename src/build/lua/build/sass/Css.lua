
local Css = build.TargetPrototype( "sass.Css" );

function Css.create( settings, filename, scss_filename, definition )
    local css = build.File( build.interpolate(filename, settings), Css, definition );
    css.settings = settings;

    local scss = build.SourceFile( build.interpolate(scss_filename, settings) );
    scss:set_required_to_exist( true );
    css:add_dependency( scss );

    local directory = build.Directory( css:branch() );
    css:add_dependency( directory );

    return css;
end

function Css.build( css )
    if css:outdated() then
        print( leaf(css:dependency():filename()) );
        local settings = css.settings;

        local load_paths = {};
        if settings.load_paths then
            for _, directory in ipairs(settings.load_paths) do
                table.insert( load_paths, ('-I "%s"'):format(directory) );
            end
        end
        if css.load_paths then
            for _, directory in ipairs(css.load_paths) do
                table.insert( load_paths, ('-I "%s"'):format(directory) );
            end
        end

        local ruby = settings.ruby.executable;
        local arguments = {
            ruby,
            settings.sass.executable,
            table.concat( load_paths, " " ),
            ('--cache-location "%s"'):format( build.interpolate("${obj}/.sass-cache", settings) ),
            ('"%s"'):format( css:dependency():filename() ),
            ('"%s"'):format( css:filename() ),
        };
        local environment = {
            PATH = getenv( "PATH" );
        };
        build.system( ruby, arguments, environment, build.dependencies_filter(css) );
    end
end
