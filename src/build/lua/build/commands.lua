
function default()
    build.load();
    local all = all or build.find_target( build.initial(goal) );
    assert( all, ("No target found at '%s'"):format(tostring(build.initial(goal))) );
    local failures = build.postorder( build.visit("build"), all );
    build.save();
    printf( "build: default (build)=%dms", math.ceil(build.ticks()) );
    return failures;
end

function clean()
    build.load();
    local all = all or build.find_target( build.initial(goal) );
    assert( all, ("No target found at '%s'"):format(tostring(build.initial(goal))) );
    local failures = build.postorder( build.clean_visit, all );
    printf( "build: clean=%sms", tostring(math.ceil(build.ticks())) );
    return failures;
end

function clobber()
    build.load();
    local all = all or build.find_target( build.initial(goal) );
    assert( all, ("No target found at '%s'"):format(tostring(build.initial(goal))) );
    local failures = build.postorder( build.visit("clean"), all ) + build.postorder( build.visit("clobber"), all );
    rm( settings.cache );
    printf( "build: clobber=%sms", tostring(math.ceil(build.ticks())) );
    return failures;
end

function generate()
    build.load();
    local all = all or build.find_target( build.initial(goal) );
    assert( all, ("No target found at '%s'"):format(tostring(build.initial(goal))) );
    local failures = build.postorder( build.visit("generate"), all );
    printf( "build: generate=%sms", tostring(math.ceil(build.ticks())) );
    return failures;
end

function reconfigure()
    rm( build.settings.local_settings_filename );
    build.load();
    build.save();
end

function dependencies()
    build.load();
    local all = all or build.find_target( build.initial(goal) );
    assert( all, ("No target found at '%s'"):format(tostring(build.initial(goal))) );
    build.print_dependencies( all );
end

function namespace()
    build.load();
    local all = all or build.find_target( build.initial(goal) );
    assert( all, ("No target found at '%s'"):format(tostring(build.initial(goal))) );
    build.print_namespace( all );
end
