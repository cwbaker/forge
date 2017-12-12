
function default()
    build.load();
    local all = all or find_target( initial(goal) );
    assert( all, ("No target found at '%s'"):format(tostring(initial(goal))) );
    local failures = postorder( build.visit("build"), all );
    build.save();
    printf( "build: default (build)=%dms", math.ceil(ticks()) );
    return failures;
end

function clean()
    build.load();
    local all = all or find_target( initial(goal) );
    assert( all, ("No target found at '%s'"):format(tostring(initial(goal))) );
    local failures = postorder( build.clean_visit, all );
    rm( settings.cache );
    printf( "build: clean=%sms", tostring(math.ceil(ticks())) );
    return failures;
end

function clobber()
    build.load();
    local all = all or find_target( initial(goal) );
    assert( all, ("No target found at '%s'"):format(tostring(initial(goal))) );
    local failures = postorder( build.visit("clean"), all ) + postorder( build.visit("clobber"), all );
    rm( settings.cache );
    printf( "build: clobber=%sms", tostring(math.ceil(ticks())) );
    return failures;
end

function generate()
    build.load();
    local all = all or find_target( initial(goal) );
    assert( all, ("No target found at '%s'"):format(tostring(initial(goal))) );
    local failures = postorder( build.visit("generate"), all );
    printf( "build: generate=%sms", tostring(math.ceil(ticks())) );
    return failures;
end

function reconfigure()
    rm( build.settings.local_settings_filename );
    build.load();
    build.save();
end

function dependencies()
    build.load();
    local all = all or find_target( initial(goal) );
    assert( all, ("No target found at '%s'"):format(tostring(initial(goal))) );
    print_dependencies( all );
end

function namespace()
    build.load();
    local all = all or find_target( initial(goal) );
    assert( all, ("No target found at '%s'"):format(tostring(initial(goal))) );
    print_namespace( all );
end
