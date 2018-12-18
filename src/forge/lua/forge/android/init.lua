
local android = {};

local directory_by_architecture = {
    ["armv5"] = "armeabi";
    ["armv7"] = "armeabi-v7a";
    ["mips"] = "mips";
    ["x86"] = "x86";
};

function android.configure( settings )
    local function autodetect_ndk_directory()
        if forge:operating_system() == 'windows' then
            return 'C:/android/android-ndk';
        else
            return forge:home( 'Library/Android/ndk' );
        end
    end

    local function autodetect_sdk_directory()
        if forge:operating_system() == 'windows' then
            return 'C:/Program Files (x86)/Android/android-sdk';
        else
            return forge:home( 'Library/Android/sdk' );
        end
    end

    local function autodetect_proguard_directory()
        return forge:home( 'proguard-6.0.3' );
    end

    local function autodetect_manifest_merger()
        return forge:home( 'android-manifest-merger/target/manifest-merger-jar-with-dependencies.jar' );
    end

    local local_settings = forge.local_settings;
    if not local_settings.android then
        local_settings.updated = true;
        local_settings.android = {
            ndk_directory = autodetect_ndk_directory();
            sdk_directory = autodetect_sdk_directory();
            build_tools_directory = ('%s/build-tools/28.0.0'):format( autodetect_sdk_directory() );
            proguard_directory = autodetect_proguard_directory();
            manifest_merger = autodetect_manifest_merger();
            toolchain_version = '4.9';
            ndk_platform = 'android-21';
            sdk_platform = 'android-22';
            architectures = { 'armv5', 'armv7' };
        };
    end
end

function android.toolchain_directory( settings, architecture )
    local android = settings.android;
    local toolchain_by_architecture = {
        ["armv5"] = "arm-linux-androideabi",
        ["armv7"] = "arm-linux-androideabi",
        ["mips"] = "mipsel-linux-android",
        ["x86"] = "x86"
    };
    local prebuilt_by_operating_system = {
        windows = "windows";
        macos = "darwin-x86_64";
    };
    return ("%s/toolchains/%s-%s/prebuilt/%s"):format( 
        android.ndk_directory, 
        toolchain_by_architecture [architecture], 
        android.toolchain_version, 
        prebuilt_by_operating_system [forge:operating_system()]
    );
end

function android.platform_directory( settings, architecture )
    local android = settings.android;
    local arch_by_architecture = {
        ["armv5"] = "arm",
        ["armv7"] = "arm",
        ["mips"] = "mips",
        ["x86"] = "x86"
    };
    return ("%s/platforms/%s/arch-%s"):format( android.ndk_directory, android.ndk_platform, arch_by_architecture[architecture] );
end

function android.include_directories( settings )
    local android = settings.android;
    local architecture = settings.architecture;
    local runtime_library = settings.runtime_library;
    if runtime_library:match("gnustl.*") then
        return {
            ("%s/sources/cxx-stl/gnu-libstdc++/%s/libs/%s/include"):format( android.ndk_directory, android.toolchain_version, directory_by_architecture[architecture] ),
            ("%s/sources/cxx-stl/gnu-libstdc++/%s/include"):format( android.ndk_directory, android.toolchain_version )
        };
    elseif runtime_library:match("stlport.*") then
        return {
            ("%s/sources/cxx-stl/stlport/stlport"):format( android.ndk_directory )
        };
    elseif runtime_library:match("c++.*") then
        return {
            ("%s/sources/cxx-stl/llvm-libc++/libcxx/include"):format( android.ndk_directory )
        };
    elseif runtime_library:match("gabi++.*") then 
        return {
            ("%s/sources/cxx-stl/gabi++/include"):format( android.ndk_directory )
        };
    else 
        assertf( false, "Unrecognized C++ runtime library '%s'", tostring(runtime_library) );
    end
end

function android.library_directories( settings, architecture )
    local runtime_library = settings.runtime_library;
    if runtime_library:match("gnustl.*") then
        return {
            ("%s/usr/lib"):format( android.platform_directory(settings, architecture) ),
            ("%s/sources/cxx-stl/gnu-libstdc++/%s/libs/%s"):format( settings.android.ndk_directory, settings.android.toolchain_version, directory_by_architecture[architecture] )
        };
    elseif runtime_library:match("stlport.*") then
        return {
            ("%s/usr/lib"):format( android.platform_directory(settings, architecture) ),
            ("%s/sources/cxx-stl/stlport/libs/%s"):format( settings.android.ndk_directory, directory_by_architecture[architecture] )
        };
    elseif runtime_library:match("c++.*") then 
        return {
            ("%s/usr/lib"):format( android.platform_directory(settings, architecture) ),
            ("%s/sources/cxx-stl/llvm-libc++/libs/%s"):format( settings.android.ndk_directory, directory_by_architecture[architecture] )
        };
    elseif runtime_library:match("gabi++.*") then 
        return {
            ("%s/usr/lib"):format( android.platform_directory(settings, architecture) ),
            ("%s/sources/cxx-stl/gabi++/libs/%s"):format( settings.android.ndk_directory, directory_by_architecture[architecture] )
        };
    else 
        assertf( false, "Unrecognized C++ runtime library '%s'", tostring(runtime_library) );
    end
end

function android.initialize( settings )
    if forge:operating_system() == 'windows' then
        local path = {
            ('%s/bin'):format( android.toolchain_directory(settings, 'armv5') )
        };
        android.environment = {
            PATH = table.concat( path, ';' );
        };
    else
        local path = {
            '/usr/bin',
            '/bin',
            ('%s/bin'):format( android.toolchain_directory(settings, 'armv5') )
        };
        android.environment = {
            PATH = table.concat( path, ':' );
        };
    end

    settings.android.proguard_enabled = settings.android.proguard_enabled or variant == 'shipping';
    
    for _, architecture in ipairs(settings.android.architectures) do 
        local android_ndk_forge = forge:configure {
            obj = ('%s/cc_android_%s'):format( settings.obj, architecture );
            platform = 'android';
            architecture = architecture;
            arch_directory = directory_by_architecture[architecture];
            runtime_library = 'gnustl_shared';
        };
        local android_ndk_gcc = require 'forge.cc.android_ndk_gcc';
        android_ndk_gcc.register( android_ndk_forge );
        forge:add_default_build( ('cc_android_%s'):format(architecture), android_ndk_forge );
    end

    local android_java_forge = forge:configure {
        classes = forge:root( ('%s/classes/java_android'):format(variant) );
        gen = forge:root( ('%s/gen/java_android'):format(variant) );
        system_jars = {
            ('%s/platforms/%s/android.jar'):format( settings.android.sdk_directory, settings.android.sdk_platform );
        };
    };
    forge:add_default_build( 'java_android', android_java_forge );
end

-- Return the full path to the Android system JAR.
function android.android_jar( settings )
    return ('%s/platforms/%s/android.jar'):format( settings.android.sdk_directory, settings.android.sdk_platform );
end

-- Find the first Android .apk package found in the dependencies of the
-- passed in directory.
function android.find_apk( directory )
    local directory = directory or forge:find_target( forge:initial("all") );
    for _, dependency in directory:dependencies() do
        if dependency:prototype() == forge.Apk then 
            return dependency;
        end
    end
end

-- Deploy the fist Android .apk package found in the dependencies of the 
-- current working directory.
function android.deploy( apk )
    local sdk_directory = forge.settings.android.sdk_directory;
    if sdk_directory then 
        assertf( apk, "No android.Apk target to deploy" );
        local adb = ("%s/platform-tools/adb"):format( sdk_directory );
        assertf( forge:is_file(adb), "No 'adb' executable found at '%s'", adb );

        local device_connected = false;
        local function adb_get_state_filter( state )
            device_connected = string.find( state, "device" ) ~= nil;
        end
        forge:system( adb, ('adb get-state'), android.environment, nil, adb_get_state_filter );
        if device_connected then
            forge:system( adb, ('adb install -r "%s"'):format(apk:filename()), android.environment );
        end
    end
end

require 'forge.android.Aidl';
require 'forge.android.AndroidManifest';
require 'forge.android.Apk';
require 'forge.android.BuildConfig';
require 'forge.android.Dex';
require 'forge.android.R';

forge:register_module( android );
forge.android = android;
return android;
