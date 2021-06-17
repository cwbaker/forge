
local gcc = ToolsetPrototype( 'gcc' );

function gcc.configure( toolset, gcc_settings )
    local paths = os.getenv( 'PATH' );
    return {
        gcc = which( gcc_settings.gcc or os.getenv('CC') or 'gcc', paths );
        gxx = which( gcc_settings.gxx or os.getenv('CXX') or 'g++', paths );
        ar = which( gcc_settings.ar or os.getenv('AR') or 'ar', paths );
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
    local Cc = PatternPrototype( 'Cc' );
    Cc.identify = gcc.object_filename;
    Cc.build = function( toolset, target ) gcc.compile( toolset, target, 'c' ) end;
    toolset.Cc = Cc;

    local Cxx = PatternPrototype( 'Cxx' );
    Cxx.identify = gcc.object_filename;
    Cxx.build = function( toolset, target ) gcc.compile( toolset, target, 'c++' ) end;
    toolset.Cxx = Cxx;

    local StaticLibrary = FilePrototype( 'StaticLibrary' );
    StaticLibrary.identify = gcc.static_library_filename;
    StaticLibrary.build = gcc.archive;
    toolset.StaticLibrary = StaticLibrary;

    local DynamicLibrary = FilePrototype( 'DynamicLibrary' );
    DynamicLibrary.identify = gcc.dynamic_library_filename;
    DynamicLibrary.build = gcc.link;
    toolset.DynamicLibrary = DynamicLibrary;

    local Executable = FilePrototype( 'Executable' );
    Executable.identify = gcc.executable_filename;
    Executable.build = gcc.link;
    toolset.Executable = Executable;

    toolset:defaults {
        architecture = 'x86_64';
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

local flags_by_architecture = {
    armv5 = '-march=armv5te -mtune=xscale -mthumb';
    armv7 = '-march=armv7 -mtune=xscale -mthumb';
    armv8 = '-march=armv8-a -mtune=xscale -mthumb';
    x86_64 = '-march=westmere -maes';
};

-- Compile C and C++ source to object files.
function gcc.compile( toolset, target, language )
    local settings = toolset.settings;

    local flags = {};
    gcc.append_defines( toolset, target, flags );
    gcc.append_include_directories( toolset, target, flags );
    gcc.append_compile_flags( toolset, target, flags, language );
    
    local gcc_ = settings.gcc.gcc;
    local environment = settings.gcc.environment;
    local ccflags = table.concat( flags, ' ' );
    local dependencies = ('%s.d'):format( target );
    local source = target:dependency();
    local output = target:filename();
    local input = absolute( source:filename() );
    printf( leaf(source:id()) );
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
    local settings = toolset.settings;
    pushd( toolset:obj_directory(target) );
    local objects =  {};
    local outdated_objects = 0;
    for _, dependency in target:dependencies() do
        local prototype = dependency:prototype();
        if prototype ~= toolset.Directory and prototype ~= toolset.StaticLibrary and prototype ~= toolset.DynamicLibrary then
            table.insert( objects, relative(dependency) );
            if dependency:outdated() then
                outdated_objects = outdated_objects + 1;
            end
        end
    end
    if outdated_objects > 0 then
        printf( leaf(target) );
        local objects = table.concat( objects, '" "' );
        local ar = settings.gcc.ar;
        local environment = settings.gcc.environment;
        system( ar, ('ar -rcs "%s" "%s"'):format(native(target), objects), environment );
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
        local prototype = dependency:prototype();
        if prototype ~= toolset.StaticLibrary and prototype ~= toolset.DynamicLibrary and prototype ~= toolset.Directory then
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
        local settings = toolset.settings;
        local ldflags = table.concat( flags, ' ' );
        local ldobjects = table.concat( objects, '" "' );
        local ldlibs = table.concat( libraries, ' ' );
        local gxx = settings.gcc.gxx;
        local environment = settings.gcc.environment;
        printf( leaf(target) );
        system( gxx, ('g++ %s "%s" %s'):format(ldflags, ldobjects, ldlibs), environment );
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
    local settings = toolset.settings;

    if not settings.assertions then 
        table.insert( flags, '-DNDEBUG' );
    end

    gcc.append_flags( flags, settings.defines, '-D%s' );
    gcc.append_flags( flags, target.defines, '-D%s' );
end

function gcc.append_include_directories( toolset, target, flags )
    gcc.append_flags( flags, target.include_directories, '-I "%s"' );
    gcc.append_flags( flags, toolset.settings.include_directories, '-I "%s"' );
end

function gcc.append_compile_flags( toolset, target, flags, language )
    local settings = toolset.settings;

    table.insert( flags, '-c' );
    table.insert( flags, flags_by_architecture[settings.architecture] );
    table.insert( flags, '-fpic' );
    
    gcc.append_flags( flags, target.cppflags );
    gcc.append_flags( flags, settings.cppflags );

    local language = language or 'c++';
    if language then
        table.insert( flags, ('-x %s'):format(language) );
        if string.find(language, 'c++', 1, true) then
            if settings.exceptions then
                table.insert( flags, '-fexceptions' );
            end

            if settings.run_time_type_info then
                table.insert( flags, '-frtti' );
            end

            local standard = settings.standard;
            if standard then 
                table.insert( flags, ('-std=%s'):format(standard) );
            end

            gcc.append_flags( flags, settings.cxxflags );
            gcc.append_flags( flags, target.cxxflags );
        else
            gcc.append_flags( flags, settings.cflags );
            gcc.append_flags( flags, target.cflags );
        end
    end
        
    if settings.debug then
        table.insert( flags, '-g3' );
    end

    if settings.optimization then
        table.insert( flags, '-O3' );
        table.insert( flags, '-Ofast' );
    end
    
    if settings.preprocess then
        table.insert( flags, '-E' );
    end

    if settings.runtime_checks then
        table.insert( flags, '-fstack-protector' );
    else
        table.insert( flags, '-fno-stack-protector' );
    end

    if settings.warnings_as_errors then 
        table.insert( flags, '-Werror' );
    end

    local warning_level = settings.warning_level
    if warning_level == 0 then 
        table.insert( flags, '-w' );
    elseif warning_level == 1 then
        table.insert( flags, '-Wall' );
    elseif warning_level >= 2 then
        table.insert( flags, '-Wall -Wextra' );
    end
end

function gcc.append_library_directories( toolset, target, flags )
    gcc.append_flags( flags, target.library_directories, '-L "%s"' );
    gcc.append_flags( flags, toolset.settings.library_directories, '-L "%s"' );
end

function gcc.append_link_flags( toolset, target, flags )
    local settings = toolset.settings;

    gcc.append_flags( flags, settings.ldflags );
    gcc.append_flags( flags, target.ldflags );

    table.insert( flags, flags_by_architecture[settings.architecture] );
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

function gcc.append_libraries( toolset, target, flags )
    local libraries = gcc.find_transitive_libraries( target );
    for _, library in ipairs(libraries) do
        if library.whole_archive then 
            table.insert( flags, '-Wl,--whole-archive' );
        end
        table.insert( flags, ('-l%s'):format(library:id()) );
        if library.whole_archive then 
            table.insert( flags, '-Wl,--no-whole-archive' );
        end
    end
end

function gcc.append_third_party_libraries( toolset, target, flags )
    gcc.append_flags( flags, toolset.settings.libraries, '-l%s' );
    gcc.append_flags( flags, target.libraries, '-l%s' );
end

-- Collect transitive dependencies on static and dynamic libraries.
--
-- Walks immediate dependencies adding static and dynamic libraries to a list
-- of libraries.  Recursively walks the ordering dependencies of any static
-- libraries to collect transitive static library dependencies.  Removes
-- duplicate libraries preserving the most recently added duplicates at the
-- end of the list.
--
-- Returns the list of static libraries to link with this executable.
function gcc.find_transitive_libraries( target )
    local toolset = target.toolset;
    local function yield_recurse_on_library( target )
        local prototype = target:prototype();
        local library = prototype == toolset.StaticLibrary or prototype == toolset.DynamicLibrary;
        return library, library;
    end

    local all_libraries = {};
    local index_by_library = {};
    for _, dependency in walk_dependencies(target, yield_recurse_on_library) do
        table.insert( all_libraries, dependency );
        index_by_library[dependency] = #all_libraries;
    end

    local libraries = {};
    for index, library in ipairs(all_libraries) do
        if index == index_by_library[library] then
            table.insert( libraries, library );
        end
    end
    return libraries;
end

return gcc;
