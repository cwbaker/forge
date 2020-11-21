
local gcc = ToolsetPrototype( 'gcc' );

gcc.flags_by_architecture = {
    armv5 = '-march=armv5te -mtune=xscale -mthumb';
    armv7 = '-march=armv7 -mtune=xscale -mthumb';
    armv8 = '-march=armv8-a -mtune=xscale -mthumb';
    x86_64 = '-march=westmere';
};

function gcc.configure( toolset, gcc_settings )
    local paths = os.getenv( 'PATH' );
    return {
        gcc = toolset:which( gcc_settings.gcc or os.getenv('CC') or 'gcc', paths );
        gxx = toolset:which( gcc_settings.gxx or os.getenv('CXX') or 'g++', paths );
        ar = toolset:which( gcc_settings.ar or os.getenv('AR') or 'ar', paths );
        environment = gcc_settings.environment or {
            PATH = '/usr/bin';
        };
    };
end

function gcc.validate( toolset, gcc_settings )
    return 
        exists( gcc_settings.gcc ) and 
        exists( gcc_settings.gxx ) and 
        exists( gcc_settings.ar )
    ;
end

function gcc.initialize( toolset )
    local Cc = PatternPrototype( 'Cc', gcc.object_filename );
    Cc.language = 'c';
    Cc.build = gcc.compile;
    toolset.Cc = Cc;

    local Cxx = PatternPrototype( 'Cxx', gcc.object_filename );
    Cxx.language = 'c++';
    Cxx.build = gcc.compile;
    toolset.Cxx = Cxx;

    local StaticLibrary = FilePrototype( 'StaticLibrary', gcc.static_library_filename );
    StaticLibrary.build = gcc.archive;
    toolset.StaticLibrary = StaticLibrary;

    local DynamicLibrary = FilePrototype( 'DynamicLibrary', gcc.dynamic_library_filename );
    DynamicLibrary.build = gcc.link;
    toolset.DynamicLibrary = DynamicLibrary;

    local Executable = FilePrototype( 'Executable', gcc.executable_filename );
    Executable.build = gcc.link;
    toolset.Executable = Executable;

    toolset:defaults {
        architecture = 'x86_64';
        assertions = true;
        debug = true;
        debuggable = true;
        exceptions = true;
        fast_floating_point = false;
        framework_directories = {};
        generate_dsym_bundle = false;
        generate_map_file = true;
        incremental_linking = true;
        link_time_code_generation = false;
        minimal_rebuild = true;
        objc_arc = true;
        objc_modules = true;
        optimization = false;
        pre_compiled_headers = true;
        preprocess = false;
        profiling = false;
        run_time_checks = true;
        runtime_library = 'static_debug';
        run_time_type_info = true;
        stack_size = 1048576;
        standard = 'c++17';
        string_pooling = false;
        strip = false;
        subsystem = 'CONSOLE';
        verbose_linking = false;
        warning_level = 3;
        warnings_as_errors = true;
    };

    return true;
end

function gcc.object_filename( toolset, identifier )
    return ('%s.o'):format( identifier );
end

function gcc.static_library_filename( toolset, identifier )
    local identifier = absolute( toolset:interpolate(identifier) );
    local filename = ('%s/lib%s.a'):format( branch(identifier), leaf(identifier) );
    return identifier, filename;
end

function gcc.dynamic_library_filename( toolset, identifier )
    local identifier = absolute( toolset:interpolate(identifier) );
    local filename = ('%s/lib%s.so'):format( branch(identifier), leaf(identifier) );
    return identifier, filename;
end

function gcc.executable_filename( toolset, identifier )
    local identifier = toolset:interpolate( identifier );
    local filename = identifier;
    return identifier, filename;
end

-- Compile C, C++, Objective-C, and Objective-C++.
function gcc.compile( toolset, target )
    local settings = toolset.settings;

    local flags = {};
    gcc.append_defines( toolset, target, flags );
    gcc.append_include_directories( toolset, target, flags );
    gcc.append_compile_flags( toolset, target, flags );
    
    local gcc_ = settings.gcc.gcc;
    local environment = settings.gcc.environment;
    local ccflags = table.concat( flags, ' ' );
    local dependencies = ('%s.d'):format( target );
    local source = target:dependency();
    local output = target:filename();
    local input = absolute( source:filename() );
    print( leaf(source:id()) );
    target:clear_implicit_dependencies();
    system( 
        gcc_, 
        ('gcc %s -MMD -MF "%s" -o "%s" "%s"'):format(ccflags, dependencies, output, input), 
        environment,
        toolset:dependencies_filter(target)
    );
end

-- Archive objects into a static library. 
function gcc.archive( toolset, target )
    printf( leaf(target) );
    local settings = toolset.settings;
    pushd( toolset:obj_directory(target) );
    local objects =  {};
    for _, object in forge:walk_dependencies( target ) do
        local prototype = object:prototype();
        if prototype ~= toolset.Directory then
            table.insert( objects, relative(object) );
        end
    end
    if #objects > 0 then
        local objects = table.concat( objects, '" "' );
        local ar = settings.gcc.ar;
        local environment = settings.gcc.environment;
        system( ar, ('ar -rcs "%s" "%s"'):format(native(target), objects), environment );
    end
    popd();
end

-- Link dynamic libraries and executables.
function gcc.link( toolset, target ) 
    printf( leaf(target) );

    local objects = {};
    local libraries = {};
    local settings = toolset.settings;
    pushd( toolset:obj_directory(target) );
    for _, dependency in forge:walk_dependencies(target) do
        local prototype = dependency:prototype();
        if prototype == toolset.StaticLibrary or prototype == toolset.DynamicLibrary then
            if dependency.whole_archive then 
                table.insert( libraries, '-Wl,--whole-archive' );
            end
            table.insert( libraries, ('-l%s'):format(dependency:id()) );
            if dependency.whole_archive then 
                table.insert( libraries, '-Wl,--no-whole-archive' );
            end
        elseif prototype ~= toolset.Directory then
            table.insert( objects, relative(dependency) );
        end
    end

    local flags = {};
    gcc.append_link_flags( toolset, target, flags );
    gcc.append_library_directories( toolset, target, flags );
    gcc.append_link_libraries( toolset, target, libraries );

    if #objects > 0 then
        local ldflags = table.concat( flags, ' ' );
        local ldobjects = table.concat( objects, '" "' );
        local ldlibs = table.concat( libraries, ' ' );
        local gxx = settings.gcc.gxx;
        local environment = settings.gcc.environment;
        system( gxx, ('g++ %s "%s" %s'):format(ldflags, ldobjects, ldlibs), environment );
    end
    popd();
end

function gcc.append_defines( toolset, target, flags )
    local settings = toolset.settings;

    if not settings.assertions then 
        table.insert( flags, '-DNDEBUG' );
    end

    local defines = settings.defines;
    if defines then
        for _, define in ipairs(defines) do
            table.insert( flags, ('-D%s'):format(define) );
        end
    end
    
    local defines = target.defines;
    if defines then
        for _, define in ipairs(defines) do
            table.insert( flags, ('-D%s'):format(define) );
        end
    end
end

function gcc.append_include_directories( toolset, target, flags )
    local settings = toolset.settings;

    if target.include_directories then
        for _, directory in ipairs(target.include_directories) do
            table.insert( flags, ('-I "%s"'):format(relative(directory)) );
        end
    end

    if settings.include_directories then
        for _, directory in ipairs(settings.include_directories) do
            table.insert( flags, ('-I "%s"'):format(directory) );
        end
    end
end

function gcc.append_compile_flags( toolset, target, flags )
    local settings = toolset.settings;

    table.insert( flags, "-c" );
    table.insert( flags, gcc.flags_by_architecture[settings.architecture] );
    table.insert( flags, "-fpic" );
    
    local language = target.language or 'c++';
    if language then
        table.insert( flags, ("-x %s"):format(language) );
        if string.find(language, "c++", 1, true) then
            if settings.exceptions then
                table.insert( flags, "-fexceptions" );
            end

            if settings.run_time_type_info then
                table.insert( flags, "-frtti" );
            end

            local standard = settings.standard;
            if standard then 
                table.insert( flags, ('-std=%s'):format(standard) );
            end                
        end
    end
        
    if settings.debug then
        table.insert( flags, "-g3" );
    end

    if settings.optimization then
        table.insert( flags, "-O3" );
        table.insert( flags, "-Ofast" );
    end
    
    if settings.preprocess then
        table.insert( flags, "-E" );
    end

    if settings.runtime_checks then
        table.insert( flags, "-fstack-protector" );
    else
        table.insert( flags, "-fno-stack-protector" );
    end

    if settings.warnings_as_errors then 
        table.insert( flags, "-Werror" );
    end

    local warning_level = settings.warning_level
    if warning_level == 0 then 
        table.insert( flags, "-w" );
    elseif warning_level == 1 then
        table.insert( flags, "-Wall" );
    elseif warning_level >= 2 then
        table.insert( flags, "-Wall -Wextra" );
    end
end

function gcc.append_library_directories( toolset, target, library_directories )
    local settings = toolset.settings;

    if target.library_directories then
        for _, directory in ipairs(target.library_directories) do
            table.insert( library_directories, ('-L "%s"'):format(directory) );
        end
    end
    
    if settings.library_directories then
        for _, directory in ipairs(settings.library_directories) do
            table.insert( library_directories, ('-L "%s"'):format(directory) );
        end
    end
end

function gcc.append_link_flags( toolset, target, flags )
    local settings = toolset.settings;

    table.insert( flags, gcc.flags_by_architecture[settings.architecture] );
    table.insert( flags, "-std=c++11" );

    if target:prototype() == toolset.DynamicLibrary then
        table.insert( flags, "-shared" );
    end
    
    if settings.verbose_linking then
        table.insert( flags, "-verbose" );
    end
    
    if settings.debug then
        table.insert( flags, "-g" );
    end

    -- The latest GCC with Android (or clang with iOS) doesn't recognize 
    -- '-Wl,map' to specify the path to output a mapfile.
    -- if settings.generate_map_file then
    --     table.insert( flags, ('-Wl,-Map,"%s"'):format(native(("%s.map"):format(target:filename()))) );
    -- end

    if settings.strip and not settings.generate_dsym_bundle then
        table.insert( flags, "-Wl,--strip-all" );
    end

    if settings.exported_symbols_list then
        table.insert( flags, ('-exported_symbols_list "%s"'):format(absolute(settings.exported_symbols_list)) );
    end

    table.insert( flags, ('-o "%s"'):format(native(target:filename())) );
end

function gcc.append_link_libraries( toolset, target, libraries )
    local settings = toolset.settings;

    if settings.libraries then 
        for _, library in ipairs(settings.libraries) do 
            table.insert( libraries, ('-l%s'):format(library) );
        end
    end

    if target.libraries then 
        for _, library in ipairs(target.libraries) do 
            table.insert( libraries, ('-l%s'):format(library) );
        end
    end
end

return gcc;
