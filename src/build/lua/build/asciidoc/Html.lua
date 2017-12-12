
local Html = build.TargetPrototype( "asciidoc.Html" );

function Html.create( settings, filename )
    local html = build.File( ("%s/%s.html"):format(settings.doc, basename(filename)), Html )
    html.settings = settings;

    local source = build.SourceFile( filename );
    source:set_required_to_exist( true );
    html:add_dependency( source );

    local directory = build.Directory( html:branch() );
    html:add_dependency( directory );

    local working_directory = working_directory();
    working_directory:add_dependency( html );
    return html;
end

function Html.build( html )
    if html:outdated() then
        print( leaf(html:dependency():filename()) );
        local settings = html.settings;
        local python = settings.python.executable;
        local arguments = {
            "python",
            settings.asciidoc.executable,
            ('-f "%s"'):format( settings.asciidoc.conf_file ),
            ('-o "%s"'):format( html:filename() ),
            ('"%s"'):format( html:dependency():filename() )
        };
        build.system( python, arguments, nil, build.dependencies_filter(html) );
    end
end
