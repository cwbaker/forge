
function default()
    local load_time, depend_time, bind_time = build.load( initial(goal) );
    local build_start = ticks();
    local all = all or find_target( initial(goal) );
    assert( all, ("No target found at '%s'"):format(tostring(initial(goal))) );
    local failures = postorder( build.visit("build"), all );
    local build_finish = ticks();

    local save_start = build_finish;
    build.save();
    local save_finish = ticks();
    printf( "build: default (build)=%dms; load=%dms, depend=%dms, bind=%dms, build=%dms, save=%dms", 
        math.ceil(ticks()),
        math.ceil(load_time),
        math.ceil(depend_time),
        math.ceil(bind_time),
        math.ceil(build_finish - build_start),
        math.ceil(save_finish - save_start)
    );
    return failures;
end

function clean()
    build.load( initial(goal) );
    local all = all or find_target( initial(goal) );
    assert( all, ("No target found at '%s'"):format(tostring(initial(goal))) );
    local failures = postorder( build.clean_visit, all );
    rm( settings.cache );
    printf( "build: clean=%sms", tostring(math.ceil(ticks())) );
    return failures;
end

function clobber()
    build.load( initial(goal) );
    local all = all or find_target( initial(goal) );
    assert( all, ("No target found at '%s'"):format(tostring(initial(goal))) );
    local failures = postorder( build.visit("clean"), all ) + postorder( build.visit("clobber"), all );
    rm( settings.cache );
    printf( "build: clobber=%sms", tostring(math.ceil(ticks())) );
    return failures;
end

function generate()
    build.load( initial(goal) );
    local all = all or find_target( initial(goal) );
    assert( all, ("No target found at '%s'"):format(tostring(initial(goal))) );
    local failures = postorder( build.visit("generate"), all );
    printf( "build: generate=%sms", tostring(math.ceil(ticks())) );
    return failures;
end

function compile()
    assert( source, "The 'source' variable must be specified for the 'compile' command" );
    local source_file = find_target( initial(source) );
    assert( source_file, "No compilable source file found at '"..initial(source).."'" );
    local object_file = source_file.object;
    assert( object_file, "No object file found at '"..obj_directory(source_file.unit)..obj_name(source_file:id()).."'" );
    local unit = source_file.unit;
    for dependency in unit:get_dependencies() do
        if dependency:prototype() == nil and dependency ~= unit.precompiled_header then
            dependency:set_outdated( false );
        end
    end
    unit:set_outdated( true );
    object_file:set_outdated( true );
    local failures = postorder( build.visit("build"), unit );
    return failures;
end

function reconfigure()
    build.load( initial(goal) );
    rm( build.settings.local_settings_filename );
    build.load();
    build.save();
end

function dependencies()
    build.load( initial(goal) );
    local all = all or find_target( initial(goal) );
    assert( all, ("No target found at '%s'"):format(tostring(initial(goal))) );
    print_dependencies( all );
end

function namespace()
    build.load( initial(goal) );
    local all = all or find_target( initial(goal) );
    assert( all, ("No target found at '%s'"):format(tostring(initial(goal))) );
    print_namespace( all );
end
