
local gcc = require 'forge.gcc';
local android = require 'forge.android';

local android_ndk = {};

function android_ndk.object_filename( forge, identifier )
    return ('%s.o'):format( identifier );
end

function android_ndk.static_library_filename( forge, identifier )
    local identifier = forge:absolute( forge:interpolate(identifier) );
    local filename = ('%s/lib%s.a'):format( forge:branch(identifier), forge:leaf(identifier) );
    return identifier, filename;
end

function android_ndk.dynamic_library_filename( forge, identifier )
    local identifier = forge:absolute( forge:interpolate(identifier) );
    local filename = ('%s/lib%s.so'):format( forge:branch(identifier), forge:leaf(identifier) );
    return identifier, filename;
end

function android_ndk.executable_filename( forge, identifier )
    local identifier = forge:interpolate( identifier );
    local filename = identifier;
    return identifier, filename;
end

-- The DynamicLibrary target prototype for Android adds any C++ runtime 
-- dynamic library as a dependency that is copied into the same directory as 
-- the shared object that is linked.  This is assumed to be the architecture
-- specific native library directory of an Android APK.
--
-- The DynamicLibrary target prototype for Android implicitly copies the C++ 
-- runtime dynamic library used into the destination directory alongside the
-- built dynamic library.

-- The dummy target `group` is created so that the `Apk` target prototype's 
-- dependency walk picks up the copied C++ runtime dynamic library and adds 
-- it to the Android APK.  
--
-- If the copied C++ runtime dynamic library is a dependency of the 
-- DynamicLibrary target it is ignored by the `Apk` build's dependency walk
-- because the dependency walk terminates at the first target that has a 
-- filename, i.e. the dynamic library, and thus never reaches the copied C++
-- runtime dynamic library.
function android_ndk.dynamic_library( forge, identifier, target_prototype )
    local identifier, filename = android_ndk.dynamic_library_filename( forge, identifier );
    local dynamic_library = forge:Target( identifier, target_prototype );
    dynamic_library:set_filename( filename or dynamic_library:path() );
    dynamic_library:set_cleanable( true );
    local directory = forge:Directory( forge:branch(dynamic_library) );
    dynamic_library:add_ordering_dependency( directory );

    local group = forge:Target( forge:anonymous() );
    group:add_dependency( dynamic_library );
    group.depend = function( forge, group, ... )
        return dynamic_library.depend( dynamic_library.forge, dynamic_library, ... );
    end

    local settings = forge.settings;
    local runtime_library = settings.runtime_library;
    if runtime_library then 
        if runtime_library:match(".*_shared") then 
            local destination = ("%s/lib%s.so"):format( directory:filename(), runtime_library );
            for _, directory in ipairs(android.library_directories(settings, settings.architecture)) do
                local source = ("%s/lib%s.so"):format( directory, runtime_library );
                if forge:exists(source) then
                    group:add_dependency(
                        forge:Copy (destination) {
                            source
                        }
                    );
                    break;
                end
            end
        end
    end

    return group;
end

-- Compile C, C++, Objective-C, and Objective-C++.
function android_ndk.compile( forge, target ) 
    local settings = forge.settings;

    local flags = {};
    android_ndk.append_compile_flags( forge, target, flags );
    android_ndk.append_defines( forge, target, flags );
    android_ndk.append_include_directories( forge, target, flags );

    local ccflags = table.concat( flags, ' ' );
    local gcc_ = ('%s/bin/arm-linux-androideabi-gcc'):format( android.toolchain_directory(settings, settings.architecture) );
    local environment = android.environment;
    local source = target:dependency();
    local output = target:filename();
    local input = forge:relative( source:filename() );
    print( forge:leaf(source) );
    target:clear_implicit_dependencies();
    forge:system( 
        gcc_, 
        ('arm-linux-androideabi-gcc %s -o "%s" "%s"'):format(ccflags, output, input), 
        environment,
        forge:dependencies_filter(target)
    );
end

-- Archive objects into a static library. 
function android_ndk.archive( forge, target )
    local settings = forge.settings;

    forge:pushd( forge:obj_directory(target) );
    local objects = {};
    for _, object in forge:walk_dependencies(target) do
        local prototype = object:prototype();
        if prototype == forge.Cc or prototype == forge.Cxx then
            table.insert( objects, forge:relative(object) )
        end
    end
    
    if #objects > 0 then
        local arflags = '-rcs';
        local arobjects = table.concat( objects, '" "' );
        local ar = ('%s/bin/arm-linux-androideabi-ar'):format( android.toolchain_directory(settings, settings.architecture) );
        printf( '%s', forge:leaf(target) );
        forge:system( ar, ('ar %s "%s" "%s"'):format(arflags, forge:native(target:filename()), arobjects), android.environment );
    end
    forge:popd();
end

-- Link dynamic libraries and executables.
function android_ndk.link( forge, target ) 
    local settings = forge.settings;
   
    local objects = {};
    local libraries = {};
    forge:pushd( forge:obj_directory(target) );
    for _, dependency in forge:walk_dependencies(target) do
        local prototype = dependency:prototype();
        if prototype == forge.Cc or prototype == forge.Cxx then
            table.insert( objects, forge:relative(dependency) );
        elseif prototype == forge.StaticLibrary or prototype == forge.DynamicLibrary then
            if dependency.whole_archive then
                table.insert( libraries, ("-Wl,--whole-archive") );
            end
            table.insert( libraries, ('-l%s'):format(dependency:id()) );
            if dependency.whole_archive then
                table.insert( libraries, ("-Wl,--no-whole-archive") );
            end
        end
    end

    local flags = {};
    android_ndk.append_link_flags( forge, target, flags );
    android_ndk.append_library_directories( forge, target, flags );
    android_ndk.append_link_libraries( forge, target, libraries );

    if #objects > 0 then
        local ldflags = table.concat( flags, ' ' );
        local ldoutput = forge:native( target );
        local ldobjects = table.concat( objects, '" "' );
        local ldlibs = table.concat( libraries, ' ' );
        local environment = android.environment;
        local gxx = ('%s/bin/arm-linux-androideabi-g++'):format( android.toolchain_directory(settings, settings.architecture) );
        printf( '%s', forge:leaf(target) );
        forge:system( gxx, ('arm-linux-androideabi-g++ %s -o "%s" "%s" %s'):format(ldflags, ldoutput, ldobjects, ldlibs), environment );
    end
    forge:popd();
end

function android_ndk.initialize( forge )
    local identifier = forge.settings.identifier;
    if identifier then 
        forge:add_build( forge:interpolate(identifier), forge );
    end

    local Cc = forge:FilePrototype( 'Cc' );
    Cc.language = 'c';
    Cc.build = android_ndk.compile;
    forge.Cc = forge:PatternElement( Cc, android_ndk.object_filename );

    local Cxx = forge:FilePrototype( 'Cxx' );
    Cxx.language = 'c++';
    Cxx.build = android_ndk.compile;
    forge.Cxx = forge:PatternElement( Cxx, android_ndk.object_filename );

    local StaticLibrary = forge:FilePrototype( 'StaticLibrary', android_ndk.static_library_filename );
    StaticLibrary.build = android_ndk.archive;
    forge.StaticLibrary = StaticLibrary;

    local DynamicLibrary = forge:TargetPrototype( 'DynamicLibrary' );
    DynamicLibrary.create = android_ndk.dynamic_library;
    DynamicLibrary.build = android_ndk.link;
    forge.DynamicLibrary = DynamicLibrary;

    local directory_by_architecture = {
        ['armv5'] = 'armeabi';
        ['armv7'] = 'armeabi-v7a';
        ['arm64'] = 'arm64';
        ['mips'] = 'mips';
        ['x86'] = 'x86';
    };

    local settings = forge.settings;
    forge:defaults( 
        settings, {
        architecture = 'armv7';
        arch_directory = directory_by_architecture[settings.architecture or 'armv7'];
        assertions = true;
        compile_as_c = false;
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
        runtime_library = 'gnustl_shared';
        run_time_type_info = true;
        sse2 = true;
        stack_size = 1048576;
        standard = 'c++17';
        string_pooling = false;
        strip = false;
        subsystem = 'CONSOLE';
        verbose_linking = false;
        warning_level = 3;
        warnings_as_errors = true;
    } );

    return forge;
end

function android_ndk.append_defines( forge, target, flags )
	gcc.append_defines( forge, target, flags );
    table.insert( flags, '-D__ARM_ARCH_5__' );
    table.insert( flags, '-D__ARM_ARCH_5T__' );
    table.insert( flags, '-D__ARM_ARCH_5E__' );
    table.insert( flags, '-D__ARM_ARCH_5TE__' );
    table.insert( flags, '-DANDROID' );
end

function android_ndk.append_include_directories( forge, target, flags )
    local settings = forge.settings;
	gcc.append_include_directories( forge, target, flags );
    for _, directory in ipairs(android.include_directories(settings, settings.architecture)) do
        assert( forge:exists(directory), ('The include directory "%s" does not exist'):format(directory) );
        table.insert( flags, ('-I"%s"'):format(directory) );
    end    
end

function android_ndk.append_compile_flags( forge, target, flags )
    local settings = forge.settings;
    table.insert( flags, ('--sysroot=%s'):format(android.platform_directory(settings, settings.architecture)) );
    table.insert( flags, '-ffunction-sections' );
    table.insert( flags, '-funwind-tables' );
    table.insert( flags, '-no-canonical-prefixes' );
    table.insert( flags, '-fomit-frame-pointer' );
    table.insert( flags, '-fno-strict-aliasing' );
    table.insert( flags, '-finline' );
    table.insert( flags, '-finline-limit=64' );
    table.insert( flags, '-Wa,--noexecstack' );

    local language = target.language or 'c++';
    if language then
        if string.find(language, 'c++', 1, true) then
            table.insert( flags, '-Wno-deprecated' );
            table.insert( flags, '-fpermissive' );
        end
    end

    gcc.append_compile_flags( forge, target, flags );
end

function android_ndk.append_library_directories( forge, target, library_directories )
    local settings = forge.settings;
    for _, directory in ipairs(android.library_directories(settings, settings.architecture)) do
        table.insert( library_directories, ('-L"%s"'):format(directory) );
    end
    gcc.append_library_directories( forge, target, library_directories );
end

function android_ndk.append_link_flags( forge, target, flags )
    local settings = forge.settings;
    table.insert( flags, ('--sysroot=%s'):format( android.platform_directory(settings, settings.architecture) ) );
    table.insert( flags, '-shared' );
    table.insert( flags, '-no-canonical-prefixes' );
    table.insert( flags, '-Wl,--no-undefined' );
    table.insert( flags, '-Wl,-z,noexecstack' );
    table.insert( flags, '-Wl,-z,relro' );
    table.insert( flags, '-Wl,-z,now' );
    gcc.append_link_flags( forge, target, flags );
end

function android_ndk.append_link_libraries( forge, target, libraries )
    local settings = forge.settings;
    local runtime_library = settings.runtime_library;
    if runtime_library then 
        table.insert( libraries, ('-l%s'):format(runtime_library) );
    end
	gcc.append_link_libraries( forge, target, libraries );
end

setmetatable( android_ndk, {
    __call = function( android_ndk, settings )
        local forge = require( 'forge' ):clone( settings );
        return android_ndk.initialize( forge );
    end
} );

return android_ndk;
