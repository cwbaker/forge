
ios = {};

function ios.sdkroot_by_target_and_platform( target, platform )
    local sdkroot = target.settings.sdkroot or "iphoneos";
    if platform == "ios_simulator" then 
        sdkroot = string.gsub( sdkroot, "iphoneos", "iphonesimulator" );
    end
    return sdkroot;
end

function ios.configure( settings )
    local function autodetect_iphoneos_sdk_version()
        local sdk_version = "";
        local sdk_build_version = "";

        local xcodebuild = "/usr/bin/xcodebuild";
        local arguments = "xcodebuild -sdk iphoneos -version";
        local result = execute( xcodebuild, arguments, Scanner {
            [ [[([a-zA-Z0-9]+): ([^\n]+)]] ] = function( key, value )
                if key == "ProductBuildVersion" then
                    sdk_build_version = value;
                end
            end;
            [ [[[^\(]+\([a-z]+([0-9\.]+)\)]] ] = function( version )
                sdk_version = version;
            end;
        } );
        assert( result == 0, "Running xcodebuild to extract SDK name and version failed" );

        return sdk_version, sdk_build_version;
    end

    local function autodetect_xcode_version()
        local xcode_version = "";
        local xcode_build_version = "";

        local xcodebuild = "/usr/bin/xcodebuild";
        local arguments = "xcodebuild -version";
        local result = execute( xcodebuild, arguments, Scanner {
            [ [[Xcode ([0-9]+)\.([0-9]+)]] ] = function( major, minor )
                xcode_version = ("%02d%02d"):format( tonumber(major), tonumber(minor) );
            end;
            [ [[Build version ([A-Za-z0-9]+)]] ] = function( build_version )
                xcode_build_version = build_version;
            end;
        } );
        assert( result == 0, "Running xcodebuild to extract Xcode version failed" );
        
        return xcode_version, xcode_build_version;
    end

    local function autodetect_macosx_version()
        local os_version = "";

        local sw_vers = "/usr/bin/sw_vers";
        local arguments = "sw_vers -buildVersion";
        local result = execute( sw_vers, arguments, Scanner {
            [ [[([A-Za-z0-9]+)]] ] = function( version )
                os_version = version; 
            end;
        } );
        assert( result == 0, "Running sw_vers to extract operating system version failed" );

        return os_version;
    end

    if operating_system() == "macosx" then
        local local_settings = build.local_settings;
        if not local_settings.ios then
            local sdk_version, sdk_build_version = autodetect_iphoneos_sdk_version();
            local xcode_version, xcode_build_version = autodetect_xcode_version();
            local os_version = autodetect_macosx_version();
            local_settings.updated = true;
            local_settings.ios = {
                xcrun = "/usr/bin/xcrun";
                signing_identity = "iPhone Developer";
                codesign_allocate = "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/codesign_allocate";
                sdk_name = "iphoneos";
                sdk_version = sdk_version;
                sdk_build_version = sdk_build_version;
                xcode_version = xcode_version;
                xcode_build_version = xcode_build_version;
                os_version = os_version;
            };
        end
    end
end;

function ios.initialize( settings )
    ios.configure( settings );

    if platform == "ios" or platform == "ios_simulator" then
        cc = ios.cc;
        objc = ios.objc;
        build_library = ios.build_library;
        clean_library = ios.clean_library;
        build_executable = ios.build_executable;
        clean_executable = ios.clean_executable;
        lipo_executable = ios.lipo_executable;
        obj_directory = ios.obj_directory;
        cc_name = ios.cc_name;
        cxx_name = ios.cxx_name;
        obj_name = ios.obj_name;
        lib_name = ios.lib_name;
        dll_name = ios.dll_name;
        exe_name = ios.exe_name;
        module_name = ios.module_name;
    end
end;

function ios.cc( target )
    local defines = {
        '-DBUILD_OS_IOS',
        '-DBUILD_PLATFORM_IOS',
        ('-DBUILD_VARIANT_%s'):format( upper(variant) ),
        ('-DBUILD_LIBRARY_SUFFIX="\\"_%s_%s.lib\\""'):format( platform, variant ),
        ('-DBUILD_LIBRARY_TYPE_%s'):format( upper(target.settings.library_type) ),
        ('-DBUILD_BIN_DIRECTORY="\\"%s\\""'):format( target.settings.bin ),
        ('-DBUILD_MODULE_DIRECTORY="\\"%s\\""'):format( target:get_working_directory():path() )
    };

    if target.settings.debug then
        table.insert( defines, "-D_DEBUG" );
        table.insert( defines, "-DDEBUG" );
    else 
        table.insert( defines, "-DNDEBUG" );
    end

    if target.settings.defines then
        for _, define in ipairs(target.settings.defines) do
            table.insert( defines, ("-D%s"):format(define) );
        end
    end    
    if target.defines then
        for _, define in ipairs(target.defines) do
            table.insert( defines, ("-D%s"):format(define) );
        end
    end

    local include_directories = {};
    if target.include_directories then
        for _, directory in ipairs(target.include_directories) do
            table.insert( include_directories, ('-I "%s"'):format(relative(directory)) );
        end
    end
    if target.settings.include_directories then
        for _, directory in ipairs(target.settings.include_directories) do
            table.insert( include_directories, ('-I "%s"'):format(directory) );
        end
    end
    if target.framework_directories then 
        for _, directory in ipairs(target.framework_directories) do
            table.insert( include_directories, ('-F "%s"'):format(directory) );
        end
    end
    if target.settings.framework_directories then 
        for _, directory in ipairs(target.settings.framework_directories) do
            table.insert( include_directories, ('-F "%s"'):format(directory) );
        end
    end

    local flags = {
        "-c",
        ("-arch %s"):format( target.architecture ),
        "-fasm-blocks"
    };

    local iphoneos_deployment_target = target.settings.iphoneos_deployment_target;
    if iphoneos_deployment_target then 
        table.insert( flags, ("-miphoneos-version-min=%s"):format(iphoneos_deployment_target) );
    end

    local language = target.language or "c++";
    if language then
        table.insert( flags, ("-x %s"):format(language) );
        if string.find(language, "c++", 1, true) then
            table.insert( flags, "-std=c++11" );
            table.insert( flags, "-stdlib=libstdc++" );
            if target.settings.exceptions then
                table.insert( flags, "-fexceptions" );
            end
            if target.settings.run_time_type_info then
                table.insert( flags, "-frtti" );
            end
        end

        if string.find(language, "objective", 1, true) then
            table.insert( flags, "-fobjc-abi-version=2" );
            table.insert( flags, "-fobjc-legacy-dispatch" );
            table.insert( flags, '"-DIBOutlet=__attribute__((iboutlet))"' );
            table.insert( flags, '"-DIBOutletCollection(ClassName)=__attribute__((iboutletcollection(ClassName)))"' );
            table.insert( flags, '"-DIBAction=void)__attribute__((ibaction)"' );
        end
    end
        
    if target.settings.debug or target.settings.generate_dsym_bundle then
        table.insert( flags, "-g" );
    end

    if target.settings.optimization then
        table.insert( flags, "-O3" );
        table.insert( flags, "-Ofast" );
    end
    
    if target.settings.preprocess then
        table.insert( flags, "-E" );
    end

    if target.settings.runtime_checks then
        table.insert( flags, "-fstack-protector" );
    else
        table.insert( flags, "-fno-stack-protector" );
    end

    local sdkroot = ios.sdkroot_by_target_and_platform( target, platform );
    local ccflags = table.concat( flags, " " );
    local cppdefines = table.concat( defines, " " );
    local cppdirs = table.concat( include_directories, " " );

    table.insert( defines, ('-DBUILD_VERSION="\\"%s\\""'):format(version) );
    cppdefines = table.concat( defines, " " );

    for dependency in target:get_dependencies() do
        if dependency:is_outdated() then
            if dependency:prototype() == nil then
                print( leaf(dependency.source) );
                local xcrun = target.settings.ios.xcrun;
                build.system( xcrun, ('xcrun --sdk %s clang %s %s %s -o "%s" "%s"'):format(sdkroot, ccflags, cppdirs, cppdefines, dependency:get_filename(), absolute(dependency.source)) );
            end
        end    
    end
end;

function ios.build_library( target )
    local flags = {
        "-static"
    };

    pushd( ("%s/%s"):format(obj_directory(target), target.architecture) );
    local objects =  {};
    for dependency in target:get_dependencies() do
        local prototype = dependency:prototype();
        if prototype == Cc or prototype == Cxx or prototype == ObjC or prototype == ObjCxx then
            for object in dependency:get_dependencies() do
                if object:prototype() == nil then
                    table.insert( objects, relative(object:get_filename()) );
                end
            end
        end
    end
    
    if #objects > 0 then
        local sdk = ios.sdkroot_by_target_and_platform( target, platform );
        local arflags = table.concat( flags, " " );
        local arobjects = table.concat( objects, [[" "]] );

        print( leaf(target:get_filename()) );
        local xcrun = target.settings.ios.xcrun;
        build.system( xcrun, ('xcrun --sdk %s libtool %s -o "%s" "%s"'):format(sdk, arflags, native(target:get_filename()), arobjects) );
    end
    popd();
end;

function ios.clean_library( target )
    rm( target:get_filename() );
    rmdir( obj_directory(target) );
end;

function ios.build_executable( target )
    local library_directories = {};
    if target.library_directories then
        for _, directory in ipairs(target.library_directories) do
            table.insert( library_directories, ('-L "%s"'):format(directory) );
        end
    end
    if target.settings.library_directories then
        for _, directory in ipairs(target.settings.library_directories) do
            table.insert( library_directories, ('-L "%s"'):format(directory) );
        end
    end
    if target.framework_directories then 
        for _, directory in ipairs(target.framework_directories) do
            table.insert( library_directories, ('-F "%s"'):format(directory) );
        end
    end
    if target.settings.framework_directories then 
        for _, directory in ipairs(target.settings.framework_directories) do
            table.insert( library_directories, ('-F "%s"'):format(directory) );
        end
    end
    
    local flags = {
        ("-arch %s"):format( target.architecture ),
        ('-o "%s"'):format( native(target:get_filename()) ),
        "-std=c++11",
        "-stdlib=libstdc++"
    };

    local iphoneos_deployment_target = target.settings.iphoneos_deployment_target;
    if iphoneos_deployment_target then 
        if platform == "ios" then 
            table.insert( flags, ("-mios-version-min=%s"):format(iphoneos_deployment_target) );
        elseif platform == "ios_simulator" then
            table.insert( flags, ("-mios-simulator-version-min=%s"):format(iphoneos_deployment_target) );
        end
    end

    table.insert( flags, "-ObjC" );
    table.insert( flags, "-all_load" );

    if target:prototype() == ArchivePrototype then
        table.insert( flags, "-shared" );
        table.insert( flags, ("-Wl,--out-implib,%s"):format(native(("%s/%s"):format(target.settings.lib, lib_name(target:id())))) );
    end
    
    if target.settings.verbose_linking then
        table.insert( flags, "-Wl,--verbose=31" );
    end   

    if target.settings.generate_map_file then
        table.insert( flags, ('-Wl,-map,"%s"'):format(native(("%s/%s.map"):format(obj_directory(target), target:id()))) );
    end

    if target.settings.strip and not target.settings.generate_dsym_bundle then
        table.insert( flags, "-Wl,-dead_strip" );
    end

    if target.settings.exported_symbols_list then
        table.insert( flags, ('-exported_symbols_list "%s"'):format(absolute(target.settings.exported_symbols_list)) );
    end

    local libraries = {};
    if target.settings.third_party_libraries then
        for _, library in ipairs(target.settings.third_party_libraries) do
            table.insert( libraries, ("-l%s"):format(library) );
        end
    end
    if target.third_party_libraries then
        for _, library in ipairs(target.third_party_libraries) do
            table.insert( libraries, ("-l%s"):format(library) );
        end
    end
    if target.system_libraries then
        for _, library in ipairs(target.system_libraries) do 
            table.insert( libraries, ("-l%s"):format(library) );
        end
    end
    if target.frameworks then
        for _, framework in ipairs(target.frameworks) do
            table.insert( libraries, ("-framework %s"):format(framework) );
        end
    end

    pushd( ("%s/%s"):format(obj_directory(target), target.architecture) );
    local objects = {};
    for dependency in target:get_dependencies() do
        local prototype = dependency:prototype();
        if prototype == Cc or prototype == Cxx or prototype == ObjC or prototype == ObjCxx then
            for object in dependency:get_dependencies() do
                if object:prototype() == nil then
                    table.insert( objects, relative(object:get_filename()) );
                end
            end
        elseif prototype == StaticLibrary or prototype == DynamicLibrary then
            table.insert( libraries, ("-l%s"):format(dependency:id()) );
        end
    end

    if #objects > 0 then
        local sdkroot = ios.sdkroot_by_target_and_platform( target, platform );
        local ldflags = table.concat( flags, " " );
        local lddirs = table.concat( library_directories, " " );        
        local ldobjects = table.concat( objects, '" "' );
        local ldlibs = table.concat( libraries, " " );

        print( leaf(target:get_filename()) );
        local xcrun = target.settings.ios.xcrun;
        build.system( xcrun, ('xcrun --sdk %s clang++ %s %s "%s" %s'):format(sdkroot, ldflags, lddirs, ldobjects, ldlibs) );
    end
    popd();
end

function ios.clean_executable( target )
    rm( target:get_filename() );
    rmdir( obj_directory(target) );
end

function ios.lipo_executable( target )
    local executables = {};
    for executable in target:get_dependencies() do 
        table.insert( executables, executable:get_filename() );
    end
    print( leaf(target:get_filename()) );
    local sdk = ios.sdkroot_by_target_and_platform( target, platform );
    executables = table.concat( executables, [[" "]] );
    local xcrun = target.settings.ios.xcrun;
    build.system( xcrun, ('xcrun --sdk %s lipo -create -output "%s" "%s"'):format(sdk, target:get_filename(), executables) );
end

function ios.obj_directory( target )
    return ("%s/%s_%s/%s"):format( target.settings.obj, platform, variant, relative(target:get_working_directory():path(), root()) );
end

function ios.cc_name( name )
    return ("%s.c"):format( basename(name) );
end

function ios.cxx_name( name )
    return ("%s.cpp"):format( basename(name) );
end

function ios.obj_name( name, architecture )
    return ("%s.o"):format( basename(name) );
end

function ios.lib_name( name, architecture )
    return ("lib%s_%s.a"):format( name, architecture );
end

function ios.dll_name( name )
    return ("%s.dylib"):format( name );
end

function ios.exe_name( name, architecture )
    return ("%s_%s"):format( name, architecture );
end

function ios.module_name( name, architecture )
    return ("%s_%s"):format( name, architecture );
end

require "build.ios.App";
