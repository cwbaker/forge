
local Pdf = build.TargetPrototype( "asciidoc.Pdf" );

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
        local python = settings.python.executable;
        local arguments = {
            "python",
            settings.a2x.executable,
            "-f pdf",
            ("-o %s"):format( pdf:filename() ),
            pdf:dependency():filename()
        };
        build.system( python, arguments, nil, build.dependencies_filter(pdf) );
    end
end;
