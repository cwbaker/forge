
local Html = build.TargetPrototype( "asciidoc.Html" );

function Html.create( settings, filename )
    local html = build.File( ("%s/%s.html"):format(settings.doc, build.basename(filename)), Html )
    html.settings = settings;

    local source = build.SourceFile( filename );
    source:set_required_to_exist( true );
    html:add_dependency( source );

    local directory = build.Directory( html:directory() );
    html:add_ordering_dependency( directory );

    local working_directory = build.working_directory();
    working_directory:add_dependency( html );
    return html;
end

function Html.build( html )
    if html:outdated() then
        print( build.leaf(html:dependency():filename()) );
        local settings = html.settings;
        local python = settings.python.executable;
        local arguments = {
            "python",
            settings.asciidoc.executable,
            ('-f "%s"'):format( settings.asciidoc.conf_file ),
            ('-o "%s"'):format( html:filename() ),
            ('"%s"'):format( html:dependency():filename() )
        };
        local environment = {
            PATH = os.getenv( "PATH" );
        };
        build.system( python, arguments, environment, build.dependencies_filter(html) );
    end
end
