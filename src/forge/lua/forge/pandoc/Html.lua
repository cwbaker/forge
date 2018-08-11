
local Html = forge:TargetPrototype( "pandoc.Html" );

function Html.build( forge, target )
    local append_arguments = function( arguments, prefix, other_arguments )
        if other_arguments then 
            for _, argument in ipairs(other_arguments) do 
                table.insert( arguments, ('%s"%s"'):format(prefix, argument) );
            end
        end
    end

    local settings = target.settings;
    local pandoc = target.settings.pandoc.executable;
    local arguments = {
        "pandoc",
        "--standalone",
        "--toc",
        "--to=html5",
        "--from=markdown+pipe_tables"
    };
    if target.katex then
        table.insert( arguments, ('--katex="%s"'):format(target.katex) );
    end
    if target.katex_stylesheet then
        table.insert( arguments, ('--katex-stylesheet="%s"'):format(target.katex_stylesheet) );
    end
    if target.template then 
        table.insert( arguments, ('--template "%s"'):format(target.template) );
    end
    append_arguments( arguments, "-B ", target.include_before_body );
    append_arguments( arguments, "-B ", settings.include_before_body );
    append_arguments( arguments, "-c ", target.stylesheets );
    append_arguments( arguments, "-c ", settings.stylesheets );

    local output = forge:relative( target );
    table.insert( arguments, ('-o "%s"'):format(output) );

    local input = forge:relative( target:dependency() );
    table.insert( arguments, ('"%s"'):format(input) );

    local environment = {
        PATH = os.getenv( "PATH" );
    };

    forge:system( pandoc, arguments, environment, forge:dependencies_filter(target) );
end
