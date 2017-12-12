
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
    local flags = {
        '-DBUILD_OS_IOS'
    };
    clang.append_defines( target, flags );
    clang.append_include_directories( target, flags );
    clang.append_compile_flags( target, flags );

    local iphoneos_deployment_target = target.settings.iphoneos_deployment_target;
    if iphoneos_deployment_target then 
        table.insert( flags, ("-miphoneos-version-min=%s"):format(iphoneos_deployment_target) );
    end

    local sdkroot = ios.sdkroot_by_target_and_platform( target, platform );
    local ccflags = table.concat( flags, " " );
    local xcrun = target.settings.ios.xcrun;

    for dependency in target:get_dependencies() do
        if dependency:is_outdated() then
            print( leaf(dependency.source) );
            build.system( xcrun, ('xcrun --sdk %s clang %s -o "%s" "%s"'):format(sdkroot, ccflags, dependency:get_filename(), absolute(dependency.source)) );
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
                table.insert( objects, relative(object:get_filename()) );
            end
        end
    end
    
    if #objects > 0 then
        local sdkroot = ios.sdkroot_by_target_and_platform( target, platform );
        local arflags = table.concat( flags, " " );
        local arobjects = table.concat( objects, [[" "]] );
        local xcrun = target.settings.ios.xcrun;

        print( leaf(target:get_filename()) );
        build.system( xcrun, ('xcrun --sdk %s libtool %s -o "%s" "%s"'):format(sdkroot, arflags, native(target:get_filename()), arobjects) );
    end
    popd();
end;

function ios.clean_library( target )
    rm( target:get_filename() );
    rmdir( obj_directory(target) );
end;

function ios.build_executable( target )
    local flags = {};
    clang.append_link_flags( target, flags );
    table.insert( flags, "-ObjC" );
    table.insert( flags, "-all_load" );

    local iphoneos_deployment_target = target.settings.iphoneos_deployment_target;
    if iphoneos_deployment_target then 
        if platform == "ios" then 
            table.insert( flags, ("-mios-version-min=%s"):format(iphoneos_deployment_target) );
        elseif platform == "ios_simulator" then
            table.insert( flags, ("-mios-simulator-version-min=%s"):format(iphoneos_deployment_target) );
        end
    end

    clang.append_library_directories( target, flags );

    local objects = {};
    local libraries = {};

    pushd( ("%s/%s"):format(obj_directory(target), target.architecture) );
    for dependency in target:get_dependencies() do
        local prototype = dependency:prototype();
        if prototype == Cc or prototype == Cxx or prototype == ObjC or prototype == ObjCxx then
            for object in dependency:get_dependencies() do
                table.insert( objects, relative(object:get_filename()) );
            end
        elseif prototype == StaticLibrary or prototype == DynamicLibrary then
            table.insert( libraries, ("-l%s"):format(dependency:id()) );
        end
    end

    clang.append_link_libraries( target, libraries );

    if #objects > 0 then
        local sdkroot = ios.sdkroot_by_target_and_platform( target, platform );
        local ldflags = table.concat( flags, " " );
        local ldobjects = table.concat( objects, '" "' );
        local ldlibs = table.concat( libraries, " " );
        local xcrun = target.settings.ios.xcrun;

        print( leaf(target:get_filename()) );
        build.system( xcrun, ('xcrun --sdk %s clang++ %s "%s" %s'):format(sdkroot, ldflags, ldobjects, ldlibs) );
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
