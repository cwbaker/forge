
local android = {};

local directory_by_architecture = {
    ["armv5"] = "armeabi";
    ["armv7"] = "armeabi-v7a";
    ["mips"] = "mips";
    ["x86"] = "x86";
};

function android.configure( forge )
    local function autodetect_jdk_directory()
        if operating_system() == 'windows' then
            return 'C:/Program Files/Java/jdk1.6.0_39';
        else
            return '/Library/Java/Home';
        end
    end

    local function autodetect_ndk_directory()
        if operating_system() == 'windows' then
            return 'C:/android/android-ndk';
        else
            return home( 'Library/Android/ndk' );
        end
    end

    local function autodetect_sdk_directory()
        if operating_system() == 'windows' then
            return 'C:/Program Files (x86)/Android/android-sdk';
        else
            return home( 'Library/Android/sdk' );
        end
    end

    local function autodetect_proguard_directory()
        return home( 'proguard-6.0.3' );
    end

    local function autodetect_manifest_merger()
        return home( 'android-manifest-merger/target/manifest-merger-jar-with-dependencies.jar' );
    end

    local function autodetect_ivy()
        if operating_system() == 'windows' then 
            return 'C:/Program Files/Apache Ivy/ivy-2.5.0-rc1.jar';
        else
            return home( 'apache-ivy-2.5.0-rc1/ivy-2.5.0-rc1.jar' );
        end
    end

    local function autodetect_ivy_cache_directory() 
        return home( '.ivy2/cache' );
    end

    local function autodetect_unzip() 
        if operating_system() == 'windows' then 
            return 'unzip.exe';
        else 
            return '/usr/bin/unzip';
        end
    end

    return {
        ndk_directory = autodetect_ndk_directory();
        sdk_directory = autodetect_sdk_directory();
        jdk_directory = autodetect_jdk_directory();
        build_tools_directory = ('%s/build-tools/28.0.0'):format( autodetect_sdk_directory() );
        proguard_directory = autodetect_proguard_directory();
        manifest_merger = autodetect_manifest_merger();
        toolchain_version = '4.9';
        ndk_platform = 'android-21';
        sdk_platform = 'android-22';
        ivy = autodetect_ivy();
        ivy_cache_directory = autodetect_ivy_cache_directory();
        unzip = autodetect_unzip();
        architectures = { 'armv5', 'armv7' };
    };
end

function android.validate( forge, android_settings )
    return 
        exists( android_settings.ndk_directory ) and 
        exists( android_settings.sdk_directory ) and 
        exists( android_settings.jdk_directory )
    ;
end

function android.initialize( forge )
    if forge:configure(android, 'android') then
        local settings = forge.settings;
        local identifier = forge.settings.identifier;
        if identifier then
            add_toolset( forge:interpolate(identifier), forge );
        end
        
        local Aidl = require 'forge.android.Aidl';
        forge.Aidl = forge:PatternElement( Aidl );
        forge.AndroidManifest = require 'forge.android.AndroidManifest';
        forge.Apk = require 'forge.android.Apk';
        forge.BuildConfig = require 'forge.android.BuildConfig';
        forge.Dex = require 'forge.android.Dex';
        forge.R = require 'forge.android.R';
        forge.Jar = require 'forge.android.Jar';
        forge.Java = require 'forge.android.Java';
        forge.Ivy = require 'forge.android.Ivy';
        forge.android_jar = android.android_jar;

        if operating_system() == 'windows' then
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

        forge:defaults( forge.settings, {
            classes = forge:root( ('%s/classes/android'):format(variant) );
            gen = forge:root( ('%s/gen/android'):format(variant) );
            bootclasspaths = {
                ('%s/platforms/%s/android.jar'):format( settings.android.sdk_directory, settings.android.sdk_platform );
            };
            debug = variant == 'debug';
        } );

        settings.android.proguard_enabled = settings.android.proguard_enabled or variant == 'shipping';

        return forge;
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
        prebuilt_by_operating_system [operating_system()]
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

-- Return the full path to the Android system JAR.
function android.android_jar( forge )
    local settings = forge.settings;
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
        assertf( is_file(adb), "No 'adb' executable found at '%s'", adb );

        local device_connected = false;
        local function adb_get_state_filter( state )
            device_connected = string.find( state, "device" ) ~= nil;
        end
        system( adb, ('adb get-state'), android.environment, nil, adb_get_state_filter );
        if device_connected then
            system( adb, ('adb install -r "%s"'):format(apk:filename()), android.environment );
        end
    end
end

return android;
