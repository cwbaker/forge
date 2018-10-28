
-- Switch platform to 'ios_simulator' if the specified platform is 'ios' but
-- the environment variable 'SDKROOT' contains 'iPhoneSimulator'.  This is to
-- accomodate builds triggered from Xcode that always specify 'ios' as the 
-- platform but might be building for the iOS simulator.
local sdkroot = os.getenv( "SDKROOT" );
if platform and platform == "ios" and sdkroot and sdkroot:find("iPhoneSimulator") then 
    platform = "ios_simulator";
end

local ios = {};

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
        local result = forge:execute( xcodebuild, arguments, nil, nil, function(line)
            local key, value = line:match( "(%w+): ([^\n]+)" );
            if key and value then 
                if key == "ProductBuildVersion" then 
                    sdk_build_version = value;
                elseif key == "SDKVersion" then
                    sdk_version = value;
                end
            end
        end );
        assert( result == 0, "Running xcodebuild to extract SDK name and version failed" );

        return sdk_version, sdk_build_version;
    end

    local function autodetect_xcode_version()
        local xcode_version = "";
        local xcode_build_version = "";

        local xcodebuild = "/usr/bin/xcodebuild";
        local arguments = "xcodebuild -version";
        local result = forge:execute( xcodebuild, arguments, nil, nil, function(line)
            local major, minor = line:match( "Xcode (%d+)%.(%d+)" );
            if major and minor then 
                xcode_version = ("%02d%02d"):format( tonumber(major), tonumber(minor) );
            end

            local build_version = line:match( "Build version (%w+)" )
            if build_version then
                xcode_build_version = build_version;
            end
        end );
        assert( result == 0, "Running xcodebuild to extract Xcode version failed" );
        
        return xcode_version, xcode_build_version;
    end

    local function autodetect_macos_version()
        local os_version = "";

        local sw_vers = "/usr/bin/sw_vers";
        local arguments = "sw_vers -buildVersion";
        local result = forge:execute( sw_vers, arguments, nil, nil, function(line)
            local version = line:match( "%w+" );
            if version then 
                os_version = version;
            end
        end );
        assert( result == 0, "Running sw_vers to extract operating system version failed" );

        return os_version;
    end

    local local_settings = forge.local_settings;
    if not local_settings.ios then
        local sdk_version, sdk_build_version = autodetect_iphoneos_sdk_version();
        local xcode_version, xcode_build_version = autodetect_xcode_version();
        local os_version = autodetect_macos_version();
        local_settings.updated = true;
        local_settings.ios = {
            xcrun = "/usr/bin/xcrun";
            signing_identity = "iPhone Developer";
            codesign = "/usr/bin/codesign";
            plutil = "/usr/bin/plutil";
            sdk_name = "iphoneos";
            sdk_version = sdk_version;
            sdk_build_version = sdk_build_version;
            xcode_version = xcode_version;
            xcode_build_version = xcode_build_version;
            os_version = os_version;
            architectures = { "armv7", "arm64" };
        };
    end
end;

function ios.initialize( settings )
    for _, architecture in ipairs(settings.ios.architectures) do 
        local clang_forge = forge:configure {
            obj = ('%s/cc_ios_%s'):format( settings.obj, architecture );
            platform = 'ios';
            sdkroot = 'iphoneos';
            xcrun = settings.ios.xcrun;
            architecture = architecture;
            default_architecture = architecture;
            iphoneos_deployment_target = '8.0';
            targeted_device_family = '1,2';
            provisioning_profile = forge:home( 'sweet/sweet_software/dev.mobileprovision' );
            lipo_executable = ios.lipo_executable;
            obj_directory = ios.obj_directory;
        };
        local clang = require 'forge.cc.clang';
        clang.register( clang_forge );
        forge:add_default_build( ('cc_ios_%s'):format(architecture), clang_forge );
    end
end;

function ios.cc( target )
    local flags = {
        '-DBUILD_OS_IOS'
    };
    clang.append_defines( target, flags );
    clang.append_include_directories( target, flags );
    clang.append_compile_flags( target, flags );

    local settings = target.settings;
    local iphoneos_deployment_target = settings.iphoneos_deployment_target;
    if iphoneos_deployment_target then 
        table.insert( flags, ("-miphoneos-version-min=%s"):format(iphoneos_deployment_target) );
    end

    local sdkroot = ios.sdkroot_by_target_and_platform( target, settings.platform );
    local ccflags = table.concat( flags, " " );
    local xcrun = settings.ios.xcrun;

    for _, object in target:dependencies() do
        if object:outdated() then
            object:set_built( false );
            local source = object:dependency();
            print( forge:leaf(source) );
            local dependencies = ("%s.d"):format( object );
            local output = object:filename();
            local input = forge:absolute( source );
            forge:system( 
                xcrun, 
                ('xcrun --sdk %s clang %s -MMD -MF "%s" -o "%s" "%s"'):format(sdkroot, ccflags, dependencies, output, input)
            );
            clang.parse_dependencies_file( dependencies, object );
            object:set_built( true );
        end
    end
end;

function ios.build_library( target )
    local flags = {
        "-static"
    };

    local settings = target.settings;
    forge:pushd( settings.obj_directory(forge, target) );
    local objects =  {};
    for _, dependency in target:dependencies() do
        local prototype = dependency:prototype();
        if prototype == forge.Cc or prototype == forge.Cxx or prototype == forge.ObjC or prototype == forge.ObjCxx then
            for _, object in dependency:dependencies() do
                table.insert( objects, forge:relative(object:filename()) );
            end
        end
    end
    
    if #objects > 0 then
        local sdkroot = ios.sdkroot_by_target_and_platform( target, settings.platform );
        local arflags = table.concat( flags, " " );
        local arobjects = table.concat( objects, [[" "]] );
        local xcrun = target.settings.ios.xcrun;
        forge:system( xcrun, ('xcrun --sdk %s libtool %s -o "%s" "%s"'):format(sdkroot, arflags, forge:native(target:filename()), arobjects) );
    end
    forge:popd();
end;

function ios.clean_library( target )
    forge:rm( target:filename() );
    forge:rmdir( obj_directory(forge, target) );
end;

function ios.build_executable( target )
    local flags = {};
    clang.append_link_flags( target, flags );
    table.insert( flags, "-ObjC" );
    table.insert( flags, "-all_load" );

    local settings = target.settings;
    local iphoneos_deployment_target = settings.iphoneos_deployment_target;
    if iphoneos_deployment_target then 
        if settings.platform == "ios" then 
            table.insert( flags, ("-mios-version-min=%s"):format(iphoneos_deployment_target) );
        elseif settings.platform == "ios_simulator" then
            table.insert( flags, ("-mios-simulator-version-min=%s"):format(iphoneos_deployment_target) );
        end
    end

    clang.append_library_directories( target, flags );

    local objects = {};
    local libraries = {};

    forge:pushd( settings.obj_directory(forge, target) );
    for _, dependency in target:dependencies() do
        local prototype = dependency:prototype();
        if prototype == forge.Cc or prototype == forge.Cxx or prototype == forge.ObjC or prototype == forge.ObjCxx then
            for _, object in dependency:dependencies() do
                table.insert( objects, forge:relative(object:filename()) );
            end
        elseif prototype == forge.StaticLibrary or prototype == forge.DynamicLibrary then
            table.insert( libraries, ("-l%s"):format(dependency:id()) );
        end
    end

    clang.append_link_libraries( target, libraries );

    if #objects > 0 then
        local sdkroot = ios.sdkroot_by_target_and_platform( target, settings.platform );
        local ldflags = table.concat( flags, " " );
        local ldobjects = table.concat( objects, '" "' );
        local ldlibs = table.concat( libraries, " " );
        local xcrun = settings.ios.xcrun;
        forge:system( xcrun, ('xcrun --sdk %s clang++ %s "%s" %s'):format(sdkroot, ldflags, ldobjects, ldlibs) );
    end
    forge:popd();
end

function ios.clean_executable( target )
    forge:rm( target:filename() );
end

function ios.lipo_executable( target )
    local executables = {};
    for _, executable in target:dependencies() do 
        table.insert( executables, executable:filename() );
    end
    local settings = target.settings;
    local sdk = ios.sdkroot_by_target_and_platform( target, settings.platform );
    executables = table.concat( executables, '" "' );
    local xcrun = target.settings.ios.xcrun;
    forge:system( xcrun, ('xcrun --sdk %s lipo -create -output "%s" "%s"'):format(sdk, target:filename(), executables) );
end

-- Find the first iOS .app bundle found in the dependencies of the
-- passed in directory.
function ios.find_app( directory )
    local directory = directory or forge:find_target( forge:initial("all") );
    for _, dependency in directory:dependencies() do
        if dependency:prototype() == ios.App then 
            return dependency;
        end
    end
end

-- Deploy the fist iOS .app bundle found in the dependencies of the current
-- working directory.
function ios.deploy( app )
    local ios_deploy = forge.settings.ios.ios_deploy;
    if ios_deploy then 
        assertf( app, "No ios.App target to deploy" );
        assertf( forge:is_file(ios_deploy), "No 'ios-deploy' executable found at '%s'", ios_deploy );
        forge:system( ios_deploy, ('ios-deploy --timeout 1 --bundle "%s"'):format(app:filename()) );
    else
        printf( "No 'ios-deploy' executable specified by 'settings.ios.ios_deploy'" );
    end
end

function ios.obj_directory( forge, target )
    local relative_path = forge:relative( target:working_directory():path(), forge:root() );
    return forge:absolute( relative_path, forge.settings.obj );
end

function ios.cc_name( name )
    return ("%s.c"):format( forge:basename(name) );
end

function ios.cxx_name( name )
    return ("%s.cpp"):format( forge:basename(name) );
end

function ios.obj_name( name )
    return ('%s.o'):format( name );
end

function ios.lib_name( name )
    return ("lib%s.a"):format( name );
end

function ios.dll_name( name )
    return ("%s.dylib"):format( name );
end

function ios.exe_name( name )
    return ("%s"):format( name );
end

require "forge.ios.App";

forge:register_module( ios );
return ios;
