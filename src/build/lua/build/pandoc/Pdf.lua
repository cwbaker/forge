
local Pdf = build.TargetPrototype( "pandoc.Pdf" );

function Pdf.create( settings, filename )
    local pdf = build.File( ("%s/%s.pdf"):format(settings.doc, basename(filename)), Pdf );
    pdf.setting = settings;

    local source = build.SourceFile( filename );
    source:set_required_to_exist( true );
    pdf:add_dependency( source );

    local directory = build.Directory( pdf:branch() );
    pdf:add_ordering_dependency( directory );

    local working_directory = working_directory();
    working_directory:add_dependency( pdf );
    return pdf;
end;

function Pdf.build( pdf )
    if pdf:outdated() then
        print( leaf(pdf:dependency():filename()) );
        local settings = pdf.settings;
        local pandoc = "/usr/local/bin/pandoc";
        local arguments = {
            "pandoc",
            "--to=pdf",
            ('-o "%s"'):format( pdf:filename() ),
            ('"%s"'):format( pdf:dependency():filename() )
        };
        local environment = {
            PATH = os.getenv( "PATH" );
        };
        build.system( pandoc, arguments, environment, build.dependencies_filter(html) );
    end
end;
