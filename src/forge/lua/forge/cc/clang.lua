
local clang = {};

function clang.configure( toolset, settings )
    local paths = os.getenv( 'PATH' );
    return {
        cc = which( settings.cc or os.getenv('CC') or 'clang', paths );
        cxx = which( settings.cxx or os.getenv('CXX') or 'clang++', paths );
        ar = which( settings.ar or os.getenv('AR') or 'ar', paths );
    };
end

function clang.install( toolset )
    local settings = toolset:configure_once( 'clang', clang.configure );
    exists( settings.cc );
    exists( settings.cxx );
    exists( settings.ar );

    local Cc = PatternPrototype( 'Cc' );
    Cc.identify = clang.object_filename;
    Cc.build = function( toolset, target ) clang.compile( toolset, target, 'c' ) end;
    toolset.Cc = Cc;

    local Cxx = PatternPrototype( 'Cxx' );
    Cxx.identify = clang.object_filename;
    Cxx.build = function( toolset, target ) clang.compile( toolset, target, 'c++' ) end;
    toolset.Cxx = Cxx;

    local ObjC = PatternPrototype( 'ObjC' );
    ObjC.identify = clang.object_filename;
    ObjC.build = function( toolset, target ) clang.compile( toolset, target, 'objective-c' ) end;
    toolset.ObjC = ObjC;

    local ObjCxx = PatternPrototype( 'ObjCxx' );
    ObjCxx.identify = clang.object_filename;
    ObjCxx.build = function( toolset, target ) clang.compile( toolset, target, 'objective-c++' ) end;
    toolset.ObjCxx = ObjCxx;

    local StaticLibrary = FilePrototype( 'StaticLibrary' );
    StaticLibrary.identify = clang.static_library_filename;
    StaticLibrary.depend = cc.static_library_depend;
    StaticLibrary.build = clang.archive;
    toolset.StaticLibrary = StaticLibrary;

    local DynamicLibrary = FilePrototype( 'DynamicLibrary' );
    DynamicLibrary.identify = clang.dynamic_library_filename;
    DynamicLibrary.prepare = cc.collect_transitive_dependencies;
    DynamicLibrary.build = clang.link;
    toolset.DynamicLibrary = DynamicLibrary;

    local Executable = FilePrototype( 'Executable' );
    Executable.identify = clang.executable_filename;
    Executable.prepare = cc.collect_transitive_dependencies;
    Executable.build = clang.link;
    toolset.Executable = Executable;

    toolset:defaults {
        architecture = 'native';
        assertions = true;
        debug = true;
        exceptions = true;
        generate_map_file = true;
        objc_arc = true;
        objc_modules = true;
        optimization = false;
        preprocess = false;
        run_time_type_info = true;
        standard = 'c++17';
        standard_library = 'libc++';
        strip = false;
        toolchain = 'clang';
        verbose_linking = false;
        warning_level = 3;
        warnings_as_errors = true;
    };

    return true;
end

function clang.object_filename( toolset, identifier )
    return ('%s.o'):format( identifier );
end

function clang.static_library_filename( toolset, identifier )
    local identifier = absolute( toolset:interpolate(identifier) );
    local filename = ('%s/lib%s.a'):format( branch(identifier), leaf(identifier) );
    return identifier, filename;
end

function clang.dynamic_library_filename( toolset, identifier )
    local identifier = absolute( toolset:interpolate(identifier) );
    local filename;
    local operating_system = _G.operating_system();
    if operating_system == 'macos' then
        filename = ('%s.dylib'):format( identifier );
    elseif operating_system == 'windows' then
        filename = ('%s.dll'):format( identifier );
    else
        filename = ('%s/lib%s.so'):format( branch(identifier), leaf(identifier) );
    end
    return identifier, filename;
end

function clang.executable_filename( toolset, identifier )
    local identifier = absolute( toolset:interpolate(identifier) );
    local filename = identifier;
    if operating_system() == 'windows' then 
        filename = ('%s.exe'):format( identifier );
    end
    return identifier, filename;
end

-- Compile C, C++, Objective-C, and Objective-C++.
function clang.compile( toolset, target, language ) 
    local flags = {};
    clang.append_defines( toolset, target, flags );
    clang.append_include_directories( toolset, target, flags );
    clang.append_framework_directories( toolset, target, flags );
    clang.append_compile_flags( toolset, target, flags, language );
    
    local ccflags = table.concat( flags, ' ' );
    local cc;
    if language == 'c++' or language == 'objective-c++' then
        cc = toolset.clang.cxx;
    else
        cc = toolset.clang.cc;
    end
    local environment = { PATH = branch(cc) };
    local source = target:dependency();
    printf( leaf(source) );
    local dependencies = ('%s.d'):format( target );
    local output = target:filename();
    local input = absolute( source );
    system( 
        cc, 
        ('%s %s -MMD -MF "%s" -o "%s" "%s"'):format(leaf(cc), ccflags, dependencies, output, input),
        environment
    );
    clang.parse_dependencies_file( toolset, dependencies, target );
end

-- Archive objects into a static library. 
function clang.archive( toolset, target )
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
    if outdated_objects > 0 or not exists(target) then
        printf( leaf(target) );
        local objects = table.concat( objects, '" "' );
        local ar = toolset.clang.ar;
        local environment = { PATH = branch(ar) };
        system( ar, ('ar -rcs "%s" "%s"'):format(native(target), objects), environment );
    else
        touch( target );
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
    clang.append_framework_directories( toolset, target, flags );

    local libraries = {};
    clang.append_libraries( toolset, target, libraries );
    clang.append_third_party_libraries( toolset, target, libraries );

    if #objects > 0 then
        local cxx = toolset.clang.cxx;
        local environment = { PATH = branch(cxx) };
        local ldflags = table.concat( flags, ' ' );
        local ldobjects = table.concat( objects, '" "' );
        local ldlibs = table.concat( libraries, ' ' );
        system( cxx, ('clang++ %s "%s" %s'):format(ldflags, ldobjects, ldlibs), environment );
    end
    popd();
end

function clang.append_flags( flags, values, format )
    local format = format or '%s';
    if values then
        for _, flag in ipairs(values) do
            table.insert( flags, format:format(flag) );
        end
    end
end

function clang.append_defines( toolset, target, flags )
    if not toolset.assertions then
        table.insert( flags, '-DNDEBUG' );
    end
    clang.append_flags( flags, toolset.defines, '-D%s' );
    clang.append_flags( flags, target.defines, '-D%s' );
end

function clang.append_include_directories( toolset, target, flags )
    clang.append_flags( flags, target.include_directories, '-I "%s"' );
    clang.append_flags( flags, toolset.include_directories, '-I "%s"' );
end

function clang.append_framework_directories( toolset, target, flags )
    clang.append_flags( flags, target.framework_directories, '-F "%s"' );
    clang.append_flags( flags, toolset.framework_directories, '-F "%s"' );
end

function clang.append_compile_flags( toolset, target, flags, language )
    table.insert( flags, '-c' );
    table.insert( flags, '-fasm-blocks' );

    clang.append_arch_flags( flags, toolset.architecture );
    clang.append_flags( flags, target.cppflags );
    clang.append_flags( flags, toolset.cppflags );
    
    local language = language or 'c++';
    table.insert( flags, ('-x %s'):format(language) );
    if language:find('c++', 1, true) then
        table.insert( flags, '-stdlib=libc++' );
        table.insert( flags, '-Wno-deprecated' );

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

    if language:find('objective', 1, true) then
        table.insert( flags, '-fobjc-abi-version=2' );
        table.insert( flags, '-fobjc-legacy-dispatch' );
        table.insert( flags, '"-DIBOutlet=__attribute__((iboutlet))"' );
        table.insert( flags, '"-DIBOutletCollection(ClassName)=__attribute__((iboutletcollection(ClassName)))"' );
        table.insert( flags, '"-DIBAction=void)__attribute__((ibaction)"' );
        if toolset.objc_arc then
            table.insert( flags, '-fobjc-arc' );
        end
        if toolset.objc_modules then
            if language == 'objective-c' then
                table.insert( flags, '-fmodules' );
            end
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
    elseif warning_level == 2 then
        table.insert( flags, '-Wall -Wextra' );
    elseif warning_level == 3 then
        table.insert( flags, '-Wall -Wextra' );
    else
        table.insert( flags, '-Wall -Wextra -Weverything' );
    end

    if language:find('c++', 1, true) then
        clang.append_flags( flags, toolset.cxxflags );
        clang.append_flags( flags, target.cxxflags );
    else
        clang.append_flags( flags, toolset.cflags );
        clang.append_flags( flags, target.cflags );
    end
end

function clang.append_library_directories( toolset, target, flags )
    clang.append_flags( flags, target.library_directories, '-L "%s"' );
    clang.append_flags( flags, toolset.library_directories, '-L "%s"' );
end

function clang.append_arch_flags( flags, architecture )
    if architecture ~= 'native' then
        if architecture == 'x86-64' then
            architecture = 'x86_64';
        end
        table.insert( flags, ('-arch %s'):format(architecture) );
    end
end

function clang.append_link_flags( toolset, target, flags )
    clang.append_arch_flags( flags, toolset.architecture );
    clang.append_flags( flags, toolset.ldflags );
    clang.append_flags( flags, target.ldflags );

    local standard = toolset.standard;
    if standard then 
        table.insert( flags, ('-std=%s'):format(standard) );
    end

    local standard_library = toolset.standard_library;
    if standard_library then
        table.insert( flags, ('-stdlib=%s'):format(standard_library) );
    end

    if target:prototype() == toolset.DynamicLibrary then
        table.insert( flags, '-Xlinker -dylib' );
    end
    
    if toolset.verbose_linking then
        table.insert( flags, '-Wl,--verbose=31' );
    end
    
    if toolset.generate_map_file then
        table.insert( flags, ('-Wl,-map,"%s"'):format(native(('%s/%s.map'):format(toolset:obj_directory(target), target:id()))) );
    end

    if toolset.strip and not toolset.generate_dsym_bundle then
        table.insert( flags, '-Wl,-dead_strip' );
    end

    if toolset.exported_symbols_list then
        table.insert( flags, ('-exported_symbols_list "%s"'):format(absolute(toolset.exported_symbols_list)) );
    end

    table.insert( flags, ('-o "%s"'):format(native(target:filename())) );
end

function clang.append_libraries( toolset, target, flags )
    for _, dependency in target:dependencies() do
        local prototype = dependency:prototype();
        if prototype == toolset.StaticLibrary or prototype == toolset.DynamicLibrary then
            local library = dependency;
            if library.whole_archive then
                table.insert( flags, ('-force_load "%s"'):format(library:filename()) );
            else
                table.insert( flags, ('-l%s'):format(library:id()) );
            end
        end
    end
end

function clang.append_third_party_libraries( toolset, target, flags )
    clang.append_flags( flags, toolset.frameworks, '-framework %s' );
    clang.append_flags( flags, toolset.libraries, '-l%s' );
    clang.append_flags( flags, target.frameworks, '-framework %s' );
    clang.append_flags( flags, target.libraries, '-l%s' );
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
