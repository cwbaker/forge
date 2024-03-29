
local gcc = {};

function gcc.configure( toolset, settings )
    local paths = os.getenv( 'PATH' );
    return {
        gcc = which( settings.gcc or os.getenv('CC') or 'gcc', paths );
        gxx = which( settings.gxx or os.getenv('CXX') or 'g++', paths );
        ar = which( settings.ar or os.getenv('AR') or 'ar', paths );
    };
end

function gcc.install( toolset )
    local settings = toolset:configure_once( 'gcc', gcc.configure );
    assert( exists(settings.gcc) );
    assert( exists(settings.gxx) );
    assert( exists(settings.ar) );

    local Cc = PatternRule( 'Cc', gcc.object_filename );
    Cc.build = function( toolset, target ) gcc.compile( toolset, target, 'c' ) end;
    toolset.Cc = Cc;

    local Cxx = PatternRule( 'Cxx', gcc.object_filename );
    Cxx.build = function( toolset, target ) gcc.compile( toolset, target, 'c++' ) end;
    toolset.Cxx = Cxx;

    local StaticLibrary = FileRule( 'StaticLibrary', gcc.static_library_filename );
    StaticLibrary.build = gcc.archive;
    StaticLibrary.depend = cc.static_library_depend;
    toolset.StaticLibrary = StaticLibrary;

    local DynamicLibrary = FileRule( 'DynamicLibrary', gcc.dynamic_library_filename );
    DynamicLibrary.prepare = cc.collect_transitive_dependencies;
    DynamicLibrary.build = gcc.link;
    toolset.DynamicLibrary = DynamicLibrary;

    local Executable = FileRule( 'Executable', gcc.executable_filename );
    Executable.prepare = cc.collect_transitive_dependencies;
    Executable.build = gcc.link;
    toolset.Executable = Executable;

    toolset:defaults {
        architecture = 'native';
        assertions = true;
        debug = true;
        exceptions = true;
        fast_floating_point = false;
        generate_map_file = true;
        optimization = false;
        preprocess = false;
        run_time_type_info = true;
        standard = 'c++17';
        strip = false;
        toolchain = 'gcc';
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
    local identifier = absolute( toolset:interpolate(identifier) );
    local filename = identifier;
    return identifier, filename;
end

-- Compile C and C++ source to object files.
function gcc.compile( toolset, target, language )
    local flags = {};
    gcc.append_defines( toolset, target, flags );
    gcc.append_include_directories( toolset, target, flags );
    gcc.append_compile_flags( toolset, target, flags, language );
    
    local gcc_ = toolset.gcc.gcc;
    local environment = { PATH = branch(gcc_) };
    local ccflags = table.concat( flags, ' ' );
    local dependencies = ('%s.d'):format( target );
    local source = target:dependency();
    local output = target:filename();
    local input = absolute( source:filename() );
    printf( leaf(source:id()) );
    target:clear_implicit_dependencies();
    run(
        gcc_, 
        ('gcc %s -MMD -MF "%s" -o "%s" "%s"'):format(ccflags, dependencies, output, input), 
        environment,
        toolset:dependencies_filter(target)
    );
end

-- Archive objects into a static library. 
function gcc.archive( toolset, target )
    pushd( toolset:obj_directory(target) );
    local objects =  {};
    local outdated_objects = 0;
    for _, dependency in target:dependencies() do
        local rule = dependency:rule();
        if rule ~= toolset.Directory and rule ~= toolset.StaticLibrary and rule ~= toolset.DynamicLibrary then
            table.insert( objects, relative(dependency) );
            if dependency:outdated() then
                outdated_objects = outdated_objects + 1;
            end
        end
    end
    if outdated_objects > 0 or not exists(target) then
        printf( leaf(target) );
        local objects = table.concat( objects, '" "' );
        local ar = toolset.gcc.ar;
        local environment = { PATH = branch(ar) };
        run( ar, ('ar -rcs "%s" "%s"'):format(native(target), objects), environment );
    else
        touch( target );
    end
    popd();
end

-- Link dynamic libraries and executables.
function gcc.link( toolset, target ) 
    pushd( toolset:obj_directory(target) );

    local objects = {};
    for _, dependency in walk_dependencies(target) do
        local rule = dependency:rule();
        if rule ~= toolset.StaticLibrary and rule ~= toolset.DynamicLibrary and rule ~= toolset.Directory then
            table.insert( objects, relative(dependency) );
        end
    end

    local flags = {};
    gcc.append_link_flags( toolset, target, flags );
    gcc.append_library_directories( toolset, target, flags );
    
    local libraries = {};
    gcc.append_libraries( toolset, target, libraries );
    gcc.append_third_party_libraries( toolset, target, libraries );

    if #objects > 0 then
        local ldflags = table.concat( flags, ' ' );
        local ldobjects = table.concat( objects, '" "' );
        local ldlibs = table.concat( libraries, ' ' );
        local gxx = toolset.gcc.gxx;
        local environment = { PATH = branch(gxx) };
        printf( leaf(target) );
        run( gxx, ('g++ %s "%s" %s'):format(ldflags, ldobjects, ldlibs), environment );
    end

    popd();
end

function gcc.append_flags( flags, values, format )
    local format = format or '%s';
    if values then
        for _, flag in ipairs(values) do
            table.insert( flags, format:format(flag) );
        end
    end
end

function gcc.append_defines( toolset, target, flags )
    if not toolset.assertions then 
        table.insert( flags, '-DNDEBUG' );
    end
    gcc.append_flags( flags, toolset.defines, '-D%s' );
    gcc.append_flags( flags, target.defines, '-D%s' );
end

function gcc.append_include_directories( toolset, target, flags )
    gcc.append_flags( flags, target.include_directories, '-I "%s"' );
    gcc.append_flags( flags, toolset.include_directories, '-I "%s"' );
end

function gcc.append_compile_flags( toolset, target, flags, language )
    table.insert( flags, '-c' );
    table.insert( flags, ('-march=%s'):format(toolset.architecture) );
    table.insert( flags, '-fpic' );
    
    gcc.append_flags( flags, target.cppflags );
    gcc.append_flags( flags, toolset.cppflags );

    local language = language or 'c++';
    table.insert( flags, ('-x %s'):format(language) );
    if string.find(language, 'c++', 1, true) then
        if toolset.exceptions then
            table.insert( flags, '-fexceptions' );
        end

        if toolset.run_time_type_info then
            table.insert( flags, '-frtti' );
        end

        local standard = toolset.standard;
        if standard then 
            table.insert( flags, ('-std=%s'):format(standard) );
        end
    end
        
    if toolset.debug then
        table.insert( flags, '-g3' );
    end

    if toolset.optimization then
        table.insert( flags, '-O3' );
        table.insert( flags, '-Ofast' );
    end
    
    if toolset.preprocess then
        table.insert( flags, '-E' );
    end

    if toolset.runtime_checks then
        table.insert( flags, '-fstack-protector' );
    else
        table.insert( flags, '-fno-stack-protector' );
    end

    if toolset.warnings_as_errors then 
        table.insert( flags, '-Werror' );
    end

    local warning_level = toolset.warning_level
    if warning_level == 0 then 
        table.insert( flags, '-w' );
    elseif warning_level == 1 then
        table.insert( flags, '-Wall' );
    elseif warning_level >= 2 then
        table.insert( flags, '-Wall -Wextra' );
    end

    if language:find('c++', 1, true) then
        gcc.append_flags( flags, toolset.cxxflags );
        gcc.append_flags( flags, target.cxxflags );
    else
        gcc.append_flags( flags, toolset.cflags );
        gcc.append_flags( flags, target.cflags );
    end
end

function gcc.append_library_directories( toolset, target, flags )
    gcc.append_flags( flags, target.library_directories, '-L "%s"' );
    gcc.append_flags( flags, toolset.library_directories, '-L "%s"' );
end

function gcc.append_link_flags( toolset, target, flags )
    gcc.append_flags( flags, toolset.ldflags );
    gcc.append_flags( flags, target.ldflags );

    table.insert( flags, ('-march=%s'):format(toolset.architecture) );
    table.insert( flags, "-std=c++11" );

    if target:rule() == toolset.DynamicLibrary then
        table.insert( flags, "-shared" );
    end
    
    if toolset.verbose_linking then
        table.insert( flags, "--verbose" );
    end
    
    if toolset.debug then
        table.insert( flags, "-g" );
    end

    -- The latest GCC with Android (or clang with iOS) doesn't recognize 
    -- '-Wl,map' to specify the path to output a mapfile.
    -- if toolset.generate_map_file then
    --     table.insert( flags, ('-Wl,-Map,"%s"'):format(native(("%s.map"):format(target:filename()))) );
    -- end

    if toolset.strip and not toolset.generate_dsym_bundle then
        table.insert( flags, "-Wl,--strip-all" );
    end

    if toolset.exported_symbols_list then
        table.insert( flags, ('-exported_symbols_list "%s"'):format(absolute(toolset.exported_symbols_list)) );
    end

    table.insert( flags, ('-o "%s"'):format(native(target:filename())) );
end

function gcc.append_libraries( toolset, target, flags )
    for _, dependency in target:dependencies() do
        local rule = dependency:rule();
        if rule == toolset.StaticLibrary or rule == toolset.DynamicLibrary then
            local library = dependency;
            if library.whole_archive then 
                table.insert( flags, '-Wl,--whole-archive' );
            end
            table.insert( flags, ('-l%s'):format(library:id()) );
            if library.whole_archive then 
                table.insert( flags, '-Wl,--no-whole-archive' );
            end
        end
    end
end

function gcc.append_third_party_libraries( toolset, target, flags )
    gcc.append_flags( flags, toolset.libraries, '-l%s' );
    gcc.append_flags( flags, target.libraries, '-l%s' );
end

return gcc;
