
function default()
    local failures = build.postorder( build.build_visit, build.load() );
    build.save();
    printf( "build: default (build)=%dms", math.ceil(build.ticks()) );
    return failures;
end

function clean()
    local failures = build.postorder( build.clean_visit, build.load() );
    printf( "build: clean=%sms", tostring(math.ceil(build.ticks())) );
    return failures;
end

function generate()
    local failures = build.postorder( build.visit("generate"), build.load() );
    printf( "build: generate=%sms", tostring(math.ceil(build.ticks())) );
    return failures;
end

function reconfigure()
    rm( build.settings.local_settings_filename );
    build.load();
    return 0;
end

function dependencies()
    build.print_dependencies( build.load() );
    return 0;
end

function namespace()
    build.print_namespace( build.load() );
    return 0;
end
