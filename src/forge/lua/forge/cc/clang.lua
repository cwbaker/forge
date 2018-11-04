
local clang = {};

function clang.object_filename( forge, identifier )
    return ('%s.o'):format( identifier );
end

function clang.static_library_filename( forge, identifier )
    local identifier = forge:absolute( forge:interpolate(identifier) );
    local filename = ('%s/lib%s.a'):format( forge:branch(identifier), forge:leaf(identifier) );
    return identifier, filename;
end

function clang.dynamic_library_filename( forge, identifier )
    local identifier = forge:absolute( forge:interpolate(identifier) );
    local filename = ('%s.dylib'):format( identifier );
    return identifier, filename;
end

function clang.executable_filename( forge, identifier )
    local identifier = forge:interpolate( identifier );
    local filename = identifier;
    return identifier, filename;
end

-- Compile C, C++, Objective-C, and Objective-C++.
function clang.compile( forge, target ) 
    local settings = forge.settings;

    local flags = {};
    clang.append_defines( forge, target, flags );
    clang.append_include_directories( forge, target, flags );
    clang.append_compile_flags( forge, target, flags );
    
    local sdkroot = settings.sdkroot;
    if sdkroot == 'macosx' then 
        local macos_deployment_target = settings.macos_deployment_target;
        if macos_deployment_target then 
            table.insert( flags, ('-mmacosx-version-min=%s'):format(macos_deployment_target) );
        end
    elseif sdkroot == 'iphoneos' then
        local iphoneos_deployment_target = settings.iphoneos_deployment_target;
        if iphoneos_deployment_target then 
            table.insert( flags, ('-miphoneos-version-min=%s'):format(iphoneos_deployment_target) );
        end
    end

    local ccflags = table.concat( flags, ' ' );
    local xcrun = settings.xcrun;
    local source = target:dependency();
    print( forge:leaf(source) );
    local dependencies = ('%s.d'):format( target );
    local output = target:filename();
    local input = forge:absolute( source );
    forge:system( 
        xcrun, 
        ('xcrun --sdk %s clang %s -MMD -MF "%s" -o "%s" "%s"'):format(sdkroot, ccflags, dependencies, output, input)
    );
    clang.parse_dependencies_file( forge, dependencies, target );
end

-- Archive objects into a static library. 
function clang.archive( forge, target )
    local flags = {
        '-static'
    };

    local settings = forge.settings;
    forge:pushd( settings.obj_directory(forge, target) );
    local objects =  {};
    for _, object in forge:walk_dependencies( target ) do
        local prototype = object:prototype();
        if prototype == forge.Cc or prototype == forge.Cxx or prototype == forge.ObjC or prototype == forge.ObjCxx then
            table.insert( objects, forge:relative(object) );
        end
    end
    
    if #objects > 0 then
        local arflags = table.concat( flags, ' ' );
        local arobjects = table.concat( objects, '" "' );
        local xcrun = settings.xcrun;
        forge:system( xcrun, ('xcrun --sdk macosx libtool %s -o "%s" "%s"'):format(arflags, forge:native(target), arobjects) );
    end
    forge:popd();
end

-- Link dynamic libraries and executables.
function clang.link( forge, target ) 
    local settings = forge.settings;

    local objects = {};
    local libraries = {};
    forge:pushd( settings.obj_directory(forge, target) );
    for _, dependency in forge:walk_dependencies(target) do
        local prototype = dependency:prototype();
        if prototype == forge.Cc or prototype == forge.Cxx or prototype == forge.ObjC or prototype == forge.ObjCxx then
            table.insert( objects, forge:relative(dependency) );
        elseif prototype == forge.StaticLibrary or prototype == forge.DynamicLibrary then
            table.insert( libraries, ('-l%s'):format(dependency:id()) );
        end
    end

    local flags = {};
    clang.append_link_flags( forge, target, flags );
    clang.append_library_directories( forge, target, flags );
    clang.append_link_libraries( forge, target, libraries );

    local sdkroot = settings.sdkroot;
    if sdkroot == 'macosx' then 
        local macos_deployment_target = settings.macos_deployment_target;
        if macos_deployment_target then 
            table.insert( flags, ('-mmacosx-version-min=%s'):format(macos_deployment_target) );
        end
    elseif sdkroot == 'iphoneos' then
        local iphoneos_deployment_target = settings.iphoneos_deployment_target;
        if iphoneos_deployment_target then 
            if settings.platform == 'ios' then 
                table.insert( flags, ('-mios-version-min=%s'):format(iphoneos_deployment_target) );
            elseif settings.platform == 'ios_simulator' then
                table.insert( flags, ('-mios-simulator-version-min=%s'):format(iphoneos_deployment_target) );
            end
        end
    end

    if #objects > 0 then
        local xcrun = settings.xcrun;
        local sdkroot = settings.sdkroot;
        local ldflags = table.concat( flags, ' ' );
        local ldobjects = table.concat( objects, '" "' );
        local ldlibs = table.concat( libraries, ' ' );
        forge:system( xcrun, ('xcrun --sdk %s clang++ %s "%s" %s'):format(sdkroot, ldflags, ldobjects, ldlibs) );
    end
    forge:popd();
end

local Cc = forge:PatternPrototype( 'Cc', clang.object_filename );
Cc.language = 'c';
Cc.build = clang.compile;
clang.Cc = Cc;

local Cxx = forge:PatternPrototype( 'Cxx', clang.object_filename );
Cxx.language = 'c++';
Cxx.build = clang.compile;
clang.Cxx = Cxx;

local ObjC = forge:PatternPrototype( 'ObjC', clang.object_filename );
ObjC.language = 'objective-c';
ObjC.build = clang.compile;
clang.ObjC = ObjC;

local ObjCxx = forge:PatternPrototype( 'ObjCxx', clang.object_filename );
ObjCxx.language = 'objective-c++';
ObjCxx.build = clang.compile;
clang.ObjCxx = ObjCxx;

local StaticLibrary = forge:FilePrototype( 'StaticLibrary', clang.static_library_filename );
StaticLibrary.build = clang.archive;
clang.StaticLibrary = StaticLibrary;

local DynamicLibrary = forge:FilePrototype( 'DynamicLibrary', clang.dynamic_library_filename );
DynamicLibrary.build = clang.link;
clang.DynamicLibrary = DynamicLibrary;

local Executable = forge:FilePrototype( 'Executable', clang.executable_filename );
Executable.build = clang.link;
clang.Executable = Executable;

-- Register the clang C/C++ toolset in *forge*.
function clang.register( forge )
    forge.Cc = clang.Cc;
    forge.Cxx = clang.Cxx;
    forge.ObjC = clang.ObjC;
    forge.ObjCxx = clang.ObjCxx;
    forge.StaticLibrary = clang.StaticLibrary;
    forge.DynamicLibrary = clang.DynamicLibrary;
    forge.Executable = clang.Executable;
end

function clang.append_defines( forge, target, flags )
    local settings = forge.settings;

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

function clang.append_include_directories( forge, target, flags )
    local settings = forge.settings;

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

    if target.framework_directories then 
        for _, directory in ipairs(target.framework_directories) do
            table.insert( flags, ('-F "%s"'):format(directory) );
        end
    end

    if settings.framework_directories then 
        for _, directory in ipairs(settings.framework_directories) do
            table.insert( flags, ('-F "%s"'):format(directory) );
        end
    end
end

function clang.append_compile_flags( forge, target, flags )
    local settings = forge.settings;

    table.insert( flags, '-c' );
    table.insert( flags, ('-arch %s'):format(settings.architecture) );
    table.insert( flags, '-fasm-blocks' );
    
    local language = target.language or 'c++';
    if language then
        table.insert( flags, ('-x %s'):format(language) );
        if string.find(language, 'c++', 1, true) then
            table.insert( flags, '-std=c++11' );
            table.insert( flags, '-stdlib=libc++' );
            table.insert( flags, '-Wno-deprecated' );
            if target.settings.exceptions then
                table.insert( flags, '-fexceptions' );
            end
            if target.settings.run_time_type_info then
                table.insert( flags, '-frtti' );
            end
        end

        if string.find(language, 'objective', 1, true) then
            table.insert( flags, '-fobjc-abi-version=2' );
            table.insert( flags, '-fobjc-legacy-dispatch' );
            table.insert( flags, '"-DIBOutlet=__attribute__((iboutlet))"' );
            table.insert( flags, '"-DIBOutletCollection(ClassName)=__attribute__((iboutletcollection(ClassName)))"' );
            table.insert( flags, '"-DIBAction=void)__attribute__((ibaction)"' );
            if target.settings.objc_arc then
                table.insert( flags, '-fobjc-arc' );
            end
            if target.settings.objc_modules then
                if language == 'objective-c' then
                    table.insert( flags, '-fmodules' );
                end
            end
        end
    end
        
    if target.settings.debug then
        table.insert( flags, '-g3' );
    end

    if target.settings.optimization then
        table.insert( flags, '-O3' );
        table.insert( flags, '-Ofast' );
    end
    
    if target.settings.preprocess then
        table.insert( flags, '-E' );
    end

    if target.settings.runtime_checks then
        table.insert( flags, '-fstack-protector' );
    else
        table.insert( flags, '-fno-stack-protector' );
    end

    if target.settings.warnings_as_errors then 
        table.insert( flags, '-Werror' );
    end

    local warning_level = target.settings.warning_level
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

function clang.append_library_directories( forge, target, library_directories )
    local settings = forge.settings;

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
    
    if target.framework_directories then 
        for _, directory in ipairs(target.framework_directories) do
            table.insert( library_directories, ('-F "%s"'):format(directory) );
        end
    end
    
    if settings.framework_directories then 
        for _, directory in ipairs(settings.framework_directories) do
            table.insert( library_directories, ('-F "%s"'):format(directory) );
        end
    end
end

function clang.append_link_flags( forge, target, flags )
    local settings = forge.settings;

    local rpaths = target.rpaths;
    if rpaths then 
        for _, rpath in ipairs(rpaths) do 
            table.insert( flags, ('-rpath "%s"'):format(rpath) );
        end
    end

    table.insert( flags, ('-arch %s'):format(settings.architecture) );
    table.insert( flags, '-std=c++11' );
    table.insert( flags, '-stdlib=libc++' );

    if target:prototype() == forge.DynamicLibrary then
        table.insert( flags, '-Xlinker -dylib' );
    end
    
    if settings.verbose_linking then
        table.insert( flags, '-Wl,--verbose=31' );
    end
    
    if settings.generate_map_file then
        table.insert( flags, ('-Wl,-map,"%s"'):format(forge:native(('%s/%s.map'):format(settings.obj_directory(forge, target), target:id()))) );
    end

    if settings.strip and not settings.generate_dsym_bundle then
        table.insert( flags, '-Wl,-dead_strip' );
    end

    if settings.exported_symbols_list then
        table.insert( flags, ('-exported_symbols_list "%s"'):format(absolute(settings.exported_symbols_list)) );
    end

    table.insert( flags, ('-o "%s"'):format(forge:native(target:filename())) );
end

function clang.append_link_libraries( forge, target, libraries )
    local settings = forge.settings;

    if settings.libraries then
        for _, library in ipairs(settings.libraries) do
            table.insert( libraries, ("-l%s"):format(library) );
        end
    end

    if target.libraries then
        for _, library in ipairs(target.libraries) do
            table.insert( libraries, ("-l%s"):format(library) );
        end
    end

    if settings.frameworks then 
        for _, framework in ipairs(settings.frameworks) do
            table.insert( libraries, ('-framework "%s"'):format(framework) );
        end
    end

    if target.frameworks then
        for _, framework in ipairs(target.frameworks) do
            table.insert( libraries, ('-framework "%s"'):format(framework) );
        end
    end
end

function clang.parse_dependencies_file( forge, filename, object )
    object:clear_implicit_dependencies();

    local file = io.open( filename, "r" );
    assertf( file, "Opening '%s' to parse dependencies failed", filename );
    local dependencies = file:read( "a" );
    file:close();
    file = nil;

    local TARGET_PATTERN = "([^:]+):[ \t\n\r\\]+";
    local DEPENDENCY_PATTERN = "([^\n\r]+) \\[ \t\n\r]+";
    local start, finish, path = dependencies:find( TARGET_PATTERN );
    if start and finish then 
        local start, finish, path = dependencies:find( DEPENDENCY_PATTERN, finish + 1 );
        while start and finish do 
            local filename = path:gsub( "\\ ", " " );
            local within_source_tree = forge:relative( forge:absolute(filename), forge:root() ):find( "..", 1, true ) == nil;
            if within_source_tree then 
                local dependency = forge:SourceFile( path:gsub("\\ ", " ") );
                object:add_implicit_dependency( dependency );
            end
            start, finish, path = dependencies:find( DEPENDENCY_PATTERN, finish + 1 );
        end
    end
end

forge:register_module( clang );
return clang;
