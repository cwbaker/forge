
local clang = ToolsetPrototype( 'clang' );

function clang.configure( toolset, clang_settings )
    local paths = os.getenv( 'PATH' );
    return {
        cc = toolset:which( clang_settings.cc or os.getenv('CC') or 'clang', paths );
        cxx = toolset:which( clang_settings.cxx or os.getenv('CXX') or 'clang++', paths );
        ar = toolset:which( clang_settings.ar or os.getenv('AR') or 'ar', paths );
        environment = clang_settings.environment or {
            PATH = '/usr/bin';
        };
    };
end

function clang.validate( toolset, clang_settings )
    return 
        exists( clang_settings.cc ) and 
        exists( clang_settings.cxx ) and 
        exists( clang_settings.ar )
    ;
end

function clang.initialize( toolset )
    local settings = toolset.settings;

    local Cc = PatternPrototype( 'Cc', clang.object_filename );
    Cc.language = 'c';
    Cc.build = clang.compile;

    local Cxx = PatternPrototype( 'Cxx', clang.object_filename );
    Cxx.language = 'c++';
    Cxx.build = clang.compile;

    local ObjC = PatternPrototype( 'ObjC', clang.object_filename );
    ObjC.language = 'objective-c';
    ObjC.build = clang.compile;

    local ObjCxx = PatternPrototype( 'ObjCxx', clang.object_filename );
    ObjCxx.language = 'objective-c++';
    ObjCxx.build = clang.compile;

    toolset.static_library_filename = clang.static_library_filename;
    toolset.dynamic_library_filename = clang.dynamic_library_filename;
    toolset.executable_filename = clang.executable_filename;
    toolset.compile = clang.compile;
    toolset.archive = clang.archive;
    toolset.link = clang.link;
    toolset.Cc = Cc;
    toolset.Cxx = Cxx;
    toolset.ObjC = ObjC;
    toolset.ObjCxx = ObjCxx;
    toolset.StaticLibrary = require 'forge.cc.StaticLibrary';
    toolset.DynamicLibrary = require 'forge.cc.DynamicLibrary';
    toolset.Executable = require 'forge.cc.Executable';

    toolset:defaults {
        architecture = 'x86_64';
        assertions = true;
        debug = true;
        debuggable = true;
        exceptions = true;
        fast_floating_point = false;
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
        standard_library = 'libc++';
        string_pooling = false;
        strip = false;
        verbose_linking = false;
        warning_level = 3;
        warnings_as_errors = true;
    };

    return true;
end

function clang.object_filename( forge, identifier )
    return ('%s.o'):format( identifier );
end

function clang.static_library_filename( forge, identifier )
    local identifier = absolute( forge:interpolate(identifier) );
    local filename = ('%s/lib%s.a'):format( branch(identifier), leaf(identifier) );
    return filename, identifier;
end

function clang.dynamic_library_filename( forge, identifier )
    local identifier = absolute( forge:interpolate(identifier) );
    local filename;
    local operating_system = _G.operating_system();
    if operating_system == 'macos' then
        filename = ('%s.dylib'):format( identifier );
    elseif operating_system == 'windows' then
        filename = ('%s.dll'):format( identifier );
    else
        filename = ('lib%s.so'):format( identifier );
    end
    return filename, identifier;
end

function clang.executable_filename( forge, identifier )
    local identifier = forge:interpolate( identifier );
    local filename = identifier;
    if operating_system() == 'windows' then 
        filename = ('%s.exe'):format( identifier );
    end
    return filename, identifier;
end

-- Compile C, C++, Objective-C, and Objective-C++.
function clang.compile( toolset, target ) 
    local settings = toolset.settings;

    local flags = {};
    clang.append_defines( toolset, target, flags );
    clang.append_include_directories( toolset, target, flags );
    clang.append_compile_flags( toolset, target, flags );
    
    local ccflags = table.concat( flags, ' ' );
    local cc;
    if target.language == 'c++' or target.language == 'objective-c++' then
        cc = settings.clang.cxx;
    else
        cc = settings.clang.cc;
    end
    local source = target:dependency();
    print( leaf(source) );
    local dependencies = ('%s.d'):format( target );
    local output = target:filename();
    local input = absolute( source );
    system( 
        cc, 
        ('%s %s -MMD -MF "%s" -o "%s" "%s"'):format(leaf(cc), ccflags, dependencies, output, input)
    );
    clang.parse_dependencies_file( toolset, dependencies, target );
end

-- Archive objects into a static library. 
function clang.archive( toolset, target )
    local settings = toolset.settings;
    pushd( toolset:obj_directory(target) );
    local objects =  {};
    for _, object in walk_dependencies( target ) do
        local prototype = object:prototype();
        if prototype ~= toolset.Directory then
            table.insert( objects, relative(object) );
        end
    end
    if #objects > 0 then
        printf( leaf(target) );
        local objects = table.concat( objects, '" "' );
        local ar = settings.clang.ar;
        local environment = settings.clang.environment;
        system( ar, ('ar -rcs "%s" "%s"'):format(native(target), objects), environment );
    end
    popd();
end

-- Link dynamic libraries and executables.
function clang.link( toolset, target ) 
    local objects = {};
    pushd( toolset:obj_directory(target) );
    for _, dependency in walk_dependencies(target) do
        local prototype = dependency:prototype();
        if prototype ~= toolset.StaticLibrary and prototype ~= toolset.DynamicLibrary and prototype ~= toolset.Directory then
            table.insert( objects, relative(dependency) );
        end
    end

    local flags = {};
    clang.append_link_flags( toolset, target, flags );
    clang.append_library_directories( toolset, target, flags );

    local libraries = {};
    clang.append_libraries( toolset, target, libraries );
    clang.append_third_party_libraries( toolset, target, libraries );

    if #objects > 0 then
        local settings = toolset.settings;
        local cxx = settings.clang.cxx;
        local ldflags = table.concat( flags, ' ' );
        local ldobjects = table.concat( objects, '" "' );
        local ldlibs = table.concat( libraries, ' ' );
        system( cxx, ('clang++ %s "%s" %s'):format(ldflags, ldobjects, ldlibs) );
    end
    popd();
end

function clang.append_defines( toolset, target, flags )
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

function clang.append_include_directories( toolset, target, flags )
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

function clang.append_compile_flags( toolset, target, flags )
    local settings = toolset.settings;

    table.insert( flags, '-c' );
    table.insert( flags, ('-arch %s'):format(settings.architecture) );
    table.insert( flags, '-fasm-blocks' );

    -- Set "-maes" when compiling for x86_64 only to use Meow hash when 
    -- building Forge itself.  This should be passed through settings.
    if settings.architecture == 'x86_64' then
        table.insert( flags, '-maes' );
    end
    
    local language = target.language or 'c++';
    if language then
        table.insert( flags, ('-x %s'):format(language) );
        if string.find(language, 'c++', 1, true) then
            table.insert( flags, '-stdlib=libc++' );
            table.insert( flags, '-Wno-deprecated' );

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
        end

        if string.find(language, 'objective', 1, true) then
            table.insert( flags, '-fobjc-abi-version=2' );
            table.insert( flags, '-fobjc-legacy-dispatch' );
            table.insert( flags, '"-DIBOutlet=__attribute__((iboutlet))"' );
            table.insert( flags, '"-DIBOutletCollection(ClassName)=__attribute__((iboutletcollection(ClassName)))"' );
            table.insert( flags, '"-DIBAction=void)__attribute__((ibaction)"' );
            if settings.objc_arc then
                table.insert( flags, '-fobjc-arc' );
            end
            if settings.objc_modules then
                if language == 'objective-c' then
                    table.insert( flags, '-fmodules' );
                end
            end
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
    elseif warning_level == 2 then
        table.insert( flags, '-Wall -Wextra' );
    elseif warning_level == 3 then
        table.insert( flags, '-Wall -Wextra' );
    else
        table.insert( flags, '-Wall -Wextra -Weverything' );
    end
end

function clang.append_library_directories( toolset, target, library_directories )
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

function clang.append_link_flags( toolset, target, flags )
    local settings = toolset.settings;

    table.insert( flags, ('-arch %s'):format(settings.architecture) );

    local standard = settings.standard;
    if standard then 
        table.insert( flags, ('-std=%s'):format(standard) );
    end

    local standard_library = settings.standard_library;
    if standard_library then
        table.insert( flags, ('-stdlib=%s'):format(standard_library) );
    end

    if target:prototype() == toolset.DynamicLibrary then
        table.insert( flags, '-Xlinker -dylib' );
    end
    
    if settings.verbose_linking then
        table.insert( flags, '-Wl,--verbose=31' );
    end
    
    if settings.generate_map_file then
        table.insert( flags, ('-Wl,-map,"%s"'):format(native(('%s/%s.map'):format(toolset:obj_directory(target), target:id()))) );
    end

    if settings.strip and not settings.generate_dsym_bundle then
        table.insert( flags, '-Wl,-dead_strip' );
    end

    if settings.exported_symbols_list then
        table.insert( flags, ('-exported_symbols_list "%s"'):format(absolute(settings.exported_symbols_list)) );
    end

    table.insert( flags, ('-o "%s"'):format(native(target:filename())) );
end

function clang.append_libraries( toolset, target, flags )
    local libraries = target:find_transitive_libraries();
    for _, library in ipairs(libraries) do
        table.insert( flags, ('-l%s'):format(library:id()) );
    end
end

function clang.append_third_party_libraries( toolset, target, flags )
    local settings = toolset.settings;

    local frameworks = settings.frameworks;
    if frameworks then
        for _, framework in ipairs(frameworks) do
            table.insert( flags, ('-framework %s'):format(framework) );
        end
    end

    local libraries = settings.libraries;
    if libraries then
        for _, library in ipairs(libraries) do
            table.insert( flags, ('-l%s'):format(library) );
        end
    end

    local frameworks = target.frameworks;
    if frameworks then
        for _, framework in ipairs(frameworks) do
            table.insert( flags, ('-framework %s'):format(framework) );
        end
    end

    local libraries = target.libraries;
    if libraries then
        for _, library in ipairs(libraries) do
            table.insert( flags, ('-l%s'):format(library) );
        end
    end
end

function clang.parse_dependencies_file( toolset, filename, object )
    object:clear_implicit_dependencies();

    local file = io.open( filename, 'r' );
    assertf( file, 'Opening "%s" to parse dependencies failed', filename );
    local dependencies = file:read( 'a' );
    file:close();
    file = nil;

    local TARGET_PATTERN = '([^:]+):[%s\\]+';
    local DEPENDENCY_PATTERN = '([^\n\r]+) \\[%s]+';
    local FINAL_DEPENDENCY_PATTERN = '([^\n\r]+)%s*';

    local start, finish, path = dependencies:find( TARGET_PATTERN );
    if start and finish then 
        local start, finish, path = dependencies:find( DEPENDENCY_PATTERN, finish + 1 );
        while start and finish do 
            local filename = path:gsub( '\\ ', ' ' );
            local within_source_tree = relative( absolute(filename), root() ):find( '..', 1, true ) == nil;
            if within_source_tree then 
                local dependency = toolset:SourceFile( filename );
                object:add_implicit_dependency( dependency );
            end

            local next = finish + 1;
            start, finish, path = dependencies:find( DEPENDENCY_PATTERN, next );
            if not start and not finish then
                start, finish, path = dependencies:find( FINAL_DEPENDENCY_PATTERN, next );
            end
        end
    end
end

return clang;
