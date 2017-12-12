
local Html = build.TargetPrototype( "pandoc.Html" );

function Html.create( settings, filename, txt_filename, definition )
    local html = build.File( build.interpolate(filename, settings), Html, definition )
    html.settings = settings;

    local source = build.SourceFile( build.interpolate(txt_filename) );
    source:set_required_to_exist( true );
    html:add_dependency( source );

    local directory = build.Directory( html:branch() );
    html:add_dependency( directory );

    local working_directory = build.working_directory();
    working_directory:add_dependency( html );
    return html;
end

function Html.build( html )
    local append_arguments = function( arguments, prefix, other_arguments )
        if other_arguments then 
            for _, argument in ipairs(other_arguments) do 
                table.insert( arguments, ('%s"%s"'):format(prefix, argument) );
            end
        end
    end

    if html:outdated() then
        print( build.leaf(html:dependency():filename()) );
        local settings = html.settings;
        local pandoc = html.settings.pandoc.executable;
        local arguments = {
            "pandoc",
            "--standalone",
            "--toc",
            "--to=html5"
        };
        if html.katex then
            table.insert( arguments, ('--katex="%s"'):format(html.katex) );
        end
        if html.katex_stylesheet then
            table.insert( arguments, ('--katex-stylesheet="%s"'):format(html.katex_stylesheet) );
        end
        if html.template then 
            table.insert( arguments, ('--template "%s"'):format(html.template) );
        end
        append_arguments( arguments, "-B ", html.include_before_body );
        append_arguments( arguments, "-B ", settings.include_before_body );
        append_arguments( arguments, "-c ", html.stylesheets );
        append_arguments( arguments, "-c ", settings.stylesheets );

        local output = build.relative( html:filename() );
        table.insert( arguments, ('-o "%s"'):format(output) );

        local input = build.relative( html:dependency():filename() );
        table.insert( arguments, ('"%s"'):format(input) );

        local environment = {
            PATH = os.getenv( "PATH" );
        };

        build.system( pandoc, arguments, environment, build.dependencies_filter(html) );
    end
end
