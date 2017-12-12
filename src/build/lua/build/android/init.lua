
android = {};

local directory_by_architecture = {
    ["armv5"] = "armeabi";
    ["armv7"] = "armeabi-v7a";
    ["mips"] = "mips";
    ["x86"] = "x86";
};

function android.configure( settings )
    function autodetect_jdk_directory()
        if operating_system() == "windows" then
            return "C:/Program Files/Java/jdk1.6.0_39";
        else
            return "/Library/Java/JavaVirtualMachines/1.7.0.jdk/Contents/Home";
        end
    end

    function autodetect_ndk_directory()
        if operating_system() == "windows" then
            return "C:/android/android-ndk";
        else
            return home( "android-ndk" );
        end
    end

    function autodetect_sdk_directory()
        if operating_system() == "windows" then
            return "C:/Program Files (x86)/Android/android-sdk";
        else
            return home( "android-sdk-macosx" );
        end
    end

    local local_settings = build.local_settings;
    if not local_settings.android then
        local_settings.updated = true;
        local_settings.android = {
            jdk_directory = autodetect_jdk_directory();
            ndk_directory = autodetect_ndk_directory();
            sdk_directory = autodetect_sdk_directory();
            build_tools_directory = ("%s/build-tools/19.0.0"):format( autodetect_sdk_directory() );
            toolchain_version = "4.6";
            ndk_platform = "android-14";
            sdk_platform = "android-16";
            architectures = { "armv5", "armv7" };
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
        macosx = "darwin-x86_64";
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

function android.include_directories( settings, architecture )
    local android = settings.android;
    return {
        ("%s/sources/cxx-stl/gnu-libstdc++/%s/libs/%s/include"):format( android.ndk_directory, android.toolchain_version, directory_by_architecture[architecture] ),
        ("%s/sources/cxx-stl/gnu-libstdc++/%s/include"):format( android.ndk_directory, android.toolchain_version )
    };
end

function android.library_directories( settings, architecture )
    return {
        ("%s/usr/lib"):format( android.platform_directory(settings, architecture) ),
        ("%s/sources/cxx-stl/gnu-libstdc++/%s/libs/%s"):format( settings.android.ndk_directory, settings.android.toolchain_version, directory_by_architecture[architecture] )
    };
end

function android.initialize( settings )
    android.configure( settings );
    if platform == "android" then
        -- Make sure that the environment variable VS_UNICODE_OUTPUT is not set.  
        -- Visual Studio sets this to signal its tools to communicate back to 
        -- Visual Studio using named pipes rather than stdout so that unicode output 
        -- works better but this then prevents the build tool from intercepting
        -- and collating this output.
        -- See http://blogs.msdn.com/freik/archive/2006/04/05/569025.aspx.
        putenv( "VS_UNICODE_OUTPUT", "" );

        if operating_system() == "windows" then
            local path = {
                ("%s/bin"):format( android.toolchain_directory(settings, "armv5") )
            };
            putenv( "PATH", table.concat(path, ";") );
        else
            local path = {
                "/usr/bin",
                "/bin",
                ("%s/bin"):format( android.toolchain_directory(settings, "armv5") )
            };
            putenv( "PATH", table.concat(path, ":") );
        end

        settings.android.proguard_enabled = settings.android.proguard_enabled or variant == "shipping";
        
        cc = android.cc;
        build_library = android.build_library;
        clean_library = android.clean_library;
        build_executable = android.build_executable;
        clean_executable = android.clean_executable;
        obj_directory = android.obj_directory;
        cc_name = android.cc_name;
        cxx_name = android.cxx_name;
        pch_name = android.pch_name;
        pdb_name = android.pdb_name;
        obj_name = android.obj_name;
        lib_name = android.lib_name;
        exp_name = android.exp_name;
        dll_name = android.dll_name;
        exe_name = android.exe_name;        
        ilk_name = android.ilk_name;
        module_name = android.module_name;
    end
end

function android.cc( target )
    local flags = {
        "-DBUILD_OS_ANDROID";
        "-D__ARM_ARCH_5__",
        "-D__ARM_ARCH_5T__",
        "-D__ARM_ARCH_5E__",
        "-D__ARM_ARCH_5TE__",
        "-DANDROID"
    };
    gcc.append_defines( target, flags );
    gcc.append_version_defines( target, flags );

    table.insert( flags, ("--sysroot=%s"):format(android.platform_directory(target.settings, target.architecture)) );

    gcc.append_include_directories( target, flags );
    for _, directory in ipairs(android.include_directories(target.settings, target.architecture)) do
        assert( exists(directory), ("The include directory '%s' does not exist"):format(directory) );
        table.insert( flags, ([[-I"%s"]]):format(directory) );
    end

    gcc.append_compile_flags( target, flags );

    local GccScanner = Scanner {
        [ [[((?:[A-Z]\:)?[^\:]+)\:([0-9]+)\:[0-9]+\: ([^\:]+)\:(.*)]] ] = function( filename, line, class, message )
            print( ("%s(%s): %s: %s"):format(filename, line, class, message) );
        end;
    };

    local ccflags = table.concat( flags, " " );
    local gcc = ("%s/bin/arm-linux-androideabi-gcc"):format( android.toolchain_directory(target.settings, target.architecture) );
    for dependency in target:get_dependencies() do
        if dependency:is_outdated() then
            print( leaf(dependency.source) );
            build.system( gcc, ('arm-linux-androideabi-gcc %s -o "%s" "%s"'):format(ccflags, dependency:filename(), dependency.source), GccScanner );
        end    
    end
end

function android.build_library( target )
    local flags = {
        "-rcs"
    };
    
    pushd( ("%s/%s"):format(obj_directory(target), target.architecture) );
    local objects = {};
    for compile in target:get_dependencies() do
        local prototype = compile:prototype();
        if prototype == Cc or prototype == Cxx then
            for object in compile:get_dependencies() do
                table.insert( objects, relative(object:filename()) )
            end
        end
    end
    
    if #objects > 0 then
        print( leaf(target:filename()) );
        local arflags = table.concat( flags, " " );
        local arobjects = table.concat( objects, '" "' );
        local ar = ("%s/bin/arm-linux-androideabi-ar"):format( android.toolchain_directory(target.settings, target.architecture) );
        build.system( ar, ('ar %s "%s" "%s"'):format(arflags, native(target:filename()), arobjects) );
    end
    popd();
end

function android.clean_library( target )
    rm( target:filename() );
    rmdir( obj_directory(target) );
end

function android.build_executable( target )
    local flags = { 
        ("--sysroot=%s"):format( android.platform_directory(target.settings, target.architecture) ),
        ("-Wl,-soname,%s"):format( leaf(target:filename()) ),
        "-shared",
        "-no-canonical-prefixes",
        "-Wl,--no-undefined",
        "-Wl,-z,noexecstack",
        "-Wl,-z,relro",
        "-Wl,-z,now",
        ('-o "%s"'):format( native(target:filename()) )
    };
    gcc.append_link_flags( target, flags );
    gcc.append_library_directories( target, flags );

    for _, directory in ipairs(android.library_directories(target.settings, target.architecture)) do
        table.insert( flags, ('-L"%s"'):format(directory) );
    end

    local objects = {};
    local libraries = {};

    pushd( ("%s/%s"):format(obj_directory(target), target.architecture) );
    for dependency in target:get_dependencies() do
        local prototype = dependency:prototype();
        if prototype == Cc or prototype == Cxx or prototype == ObjC or prototype == ObjCxx then
            for object in dependency:get_dependencies() do
                if object:prototype() == nil then
                    table.insert( objects, relative(object:filename()) );
                end
            end
        elseif prototype == StaticLibrary or prototype == DynamicLibrary then
            table.insert( libraries, ("-l%s"):format(dependency:id()) );
        end
    end

    gcc.append_link_libraries( target, libraries );

    if target.system_libraries then 
        for _, library in ipairs(target.system_libraries) do 
            local destination = ("%s/lib%s.so"):format( branch(target:filename()), library );
            if not exists(destination) then 
                print( ("lib%s.so"):format(library) );
                for _, directory in ipairs(android.library_directories(target.settings, target.architecture)) do
                    local source = ("%s/lib%s.so"):format( directory, library );
                    if exists(source) then
                        cp( source, destination );
                        break;
                    end
                end
            end
        end
    end

    if #objects > 0 then
        local ldflags = table.concat( flags, " " );
        local ldobjects = table.concat( objects, '" "' );
        local ldlibs = table.concat( libraries, " " );
        local gxx = ("%s/bin/arm-linux-androideabi-g++"):format( android.toolchain_directory(target.settings, target.architecture) );

        print( leaf(target:filename()) );
        build.system( gxx, ('arm-linux-androideabi-g++ %s "%s" %s'):format(ldflags, ldobjects, ldlibs) );
    end
    popd();
end 

function android.clean_executable( target )
    rm( target:filename() );
    rmdir( obj_directory(target) );
end

-- Deploy the fist Android .apk package found in the dependencies of the 
-- current working directory.
function android.deploy( directory )
    local sdk_directory = build.settings.android.sdk_directory;
    if sdk_directory then 
        local directory = directory or find_target( initial() );
        local apk = nil;
        for dependency in directory:get_dependencies() do
            if dependency:prototype() == android.Apk then 
                apk = dependency;
                break;
            end
        end
        assertf( apk, "No android.Apk target found as a dependency of '%s'", directory:path() );
        local adb = ("%s/platform-tools/adb"):format( sdk_directory );
        assertf( is_file(adb), "No 'adb' executable found at '%s'", adb );

        local device_connected = false;
        local AdbGetStateScanner = Scanner {
            [ [[(.*)]] ] = function( state )
                device_connected = state == "device";
            end
        };
        build.system( adb, ('adb get-state'), AdbGetStateScanner );
        if device_connected then
            printf( "Deploying '%s'...", apk:filename() );
            build.system( adb, ('adb install -r "%s"'):format(apk:filename()) );
        end
    else
        printf( ios_deploy, "No 'ios-deploy' executable specified in settings" );
    end
end

function android.obj_directory( target )
    return ("%s/%s_%s/%s"):format( target.settings.obj, platform, variant, relative(target:get_working_directory():path(), root()) );
end

function android.cc_name( name )
    return ("%s.c"):format( basename(name) );
end

function android.cxx_name( name )
    return ("%s.cpp"):format( basename(name) );
end

function android.obj_name( name, architecture )
    return ("%s.o"):format( basename(name) );
end

function android.lib_name( name, architecture )
    return ("lib%s_%s.a"):format( name, architecture );
end

function android.dll_name( name, architecture )
    return ("lib/%s/lib%s.so"):format( directory_by_architecture[architecture], name );
end

function android.exe_name( name, architecture )
    return ("%s_%s_%s_%s"):format( name, architecture, platform, variant );
end

function android.module_name( name, architecture )
    return ("%s_%s"):format( name, architecture );
end

require "build.android.Aidl";
require "build.android.Apk";
require "build.android.BuildConfig";
require "build.android.Dex";
require "build.android.Jar";
require "build.android.Java";
require "build.android.R";
