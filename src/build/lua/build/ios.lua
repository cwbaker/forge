
ios = {};

ios.SDK_BY_PLATFORM = {
    ios = "iphoneos8.0";
    ios_simulator = "iphonesimulator8.0";
};

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
            [ [[Xcode ([0-9]+)\.([0-9]+)\.([0-9]+)]] ] = function( major, minor, patch )
                xcode_version = "%02d%02d" % { tonumber(major), tonumber(minor) };
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
                provisioning_profile = home( "sweet/sweet_software/dev.mobileprovision" );
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
        " ",
        [[-DBUILD_OS_IOS]],
        [[-DBUILD_PLATFORM_IOS]],
        [[-DBUILD_VARIANT_%s]] % upper( variant ),
        [[-DBUILD_LIBRARY_SUFFIX="\"_%s_%s.lib\""]] % { platform, variant },
        [[-DBUILD_MODULE_%s]] % upper( string.gsub(target.module:id(), "-", "_") ),
        [[-DBUILD_LIBRARY_TYPE_%s]] % upper( target.settings.library_type ),
        [[-DBUILD_BIN_DIRECTORY="\"%s\""]] % target.settings.bin,
        [[-DBUILD_MODULE_DIRECTORY="\"%s\""]] % target:get_working_directory():path()
    };

    if target.settings.debug then
        table.insert( defines, "-D_DEBUG" );
        table.insert( defines, "-DDEBUG" );
    else 
        table.insert( defines, "-DNDEBUG" );
    end

    if target.settings.defines then
        for _, define in ipairs(target.settings.defines) do
            table.insert( defines, "-D%s" % define );
        end
    end
    
    if target.defines then
        for _, define in ipairs(target.defines) do
            table.insert( defines, "-D%s" % define );
        end
    end

    local include_directories = {};
    if target.include_directories then
        for _, directory in ipairs(target.include_directories) do
            table.insert( include_directories, [[-I "%s"]] % relative(directory) );
        end
    end
    if target.settings.include_directories then
        for _, directory in ipairs(target.settings.include_directories) do
            table.insert( include_directories, [[-I "%s"]] % directory );
        end
    end
    if target.framework_directories then 
        for _, directory in ipairs(target.framework_directories) do
            table.insert( include_directories, [[-F "%s"]] % directory );
        end
    end
    if target.settings.framework_directories then 
        for _, directory in ipairs(target.settings.framework_directories) do
            table.insert( include_directories, [[-F "%s"]] % directory );
        end
    end

    local flags = {
        "-c",
        "-arch %s" % target.architecture,
        "-fasm-blocks"
    };

    -- Add these to compile for iOS
    if platform == "ios" then 
        local sysroot = "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS8.0.sdk";
        table.insert( flags, "-miphoneos-version-min=6.1" );
        table.insert( flags, "-isysroot %s" % sysroot );
    elseif platform == "ios_simulator" then 
        local sysroot = "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator8.0.sdk";
        table.insert( flags, "-miphoneos-version-min=6.1" );
        table.insert( flags, "-isysroot %s" % sysroot );
    end

    local language = target.language or "c++";
    if language then
        table.insert( flags, "-x %s" % language );

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
            table.insert( flags, [["-DIBOutlet=__attribute__((iboutlet))"]] );
            table.insert( flags, [["-DIBOutletCollection(ClassName)=__attribute__((iboutletcollection(ClassName)))"]] );
            table.insert( flags, [["-DIBAction=void)__attribute__((ibaction)"]] );
        end
    end
        
    if target.settings.debug or target.settings.generate_dsym_bundle then
        table.insert( flags, "-g" );
    end

    if target.settings.optimization then
        table.insert( flags, "-O2" );
    end
    
    if target.settings.preprocess then
        table.insert( flags, "-E" );
    end

    if target.settings.runtime_checks then
        table.insert( flags, "-fstack-protector" );
    else
        table.insert( flags, "-fno-stack-protector" );
    end

    local sdk = ios.SDK_BY_PLATFORM[platform];
    local ccflags = table.concat( flags, " " );
    local cppdefines = table.concat( defines, " " );
    local cppdirs = table.concat( include_directories, " " );

    if target.precompiled_header ~= nil then            
        if target.precompiled_header:is_outdated() then
            print( leaf(target.precompiled_header.source) );
            local xcrun = target.settings.ios.xcrun;
            build.system( xcrun, [[xcrun --sdk %s clang %s %s %s -o %s "%s"]] % {sdk, ccflags, cppdirs, cppdefines, target.precompiled_header:get_filename(), target.precompiled_header.source} );
        end        
    end
    
    cppdefines = cppdefines.." -DBUILD_VERSION=\"\\\""..version.."\\\"\"";
    for dependency in target:get_dependencies() do
        if dependency:is_outdated() and dependency ~= target.precompiled_header then
            if dependency:prototype() == nil then
                print( leaf(dependency.source) );
                local xcrun = target.settings.ios.xcrun;
                build.system( xcrun, [[xcrun --sdk %s clang %s %s %s -o %s "%s"]] % {sdk, ccflags, cppdirs, cppdefines, dependency:get_filename(), absolute(dependency.source)} );
            elseif dependency.results then
                for _, result in ipairs(dependency.results) do
                    if result:is_outdated() then
                        print( leaf(result.source) );
                        local xcrun = target.settings.ios.xcrun;
                        build.system( xcrun, [[xcrun --sdk %s clang %s %s %s -o %s "%s"]] % {sdk, ccflags, cppdirs, cppdefines, result:get_filename(), absolute(result.source)} );
                    end
                end
            end
        end    
    end
end;

function ios.build_library( target )
    local arflags = "";
    arflags = [[%s -static]] % arflags;

    local flags = {
        "-static"
    };

    local objects =  {
    };
    for compile in target:get_dependencies() do
        if compile:prototype() == CcPrototype then
            if compile.precompiled_header then
                table.insert( objects, leaf(compile.precompiled_header:get_filename()) );
            end
            
            for object in compile:get_dependencies() do
                if object:prototype() == nil and object ~= compile.precompiled_header then
                    table.insert( objects, leaf(object:get_filename()) );
                end
            end
        end
    end
    
    if #objects > 0 then
        local sdk = ios.SDK_BY_PLATFORM[platform];
        local arflags = table.concat( flags, " " );
        local arobjects = table.concat( objects, " " );

        print( leaf(target:get_filename()) );
        pushd( "%s/%s" % {obj_directory(target), target.architecture} );
        local xcrun = target.settings.ios.xcrun;
        build.system( xcrun, [[xcrun --sdk %s libtool %s -o %s %s]] % {sdk, arflags, native(target:get_filename()), arobjects} );
        popd();
    end
end;

function ios.clean_library( target )
    rm( target:get_filename() );
    rmdir( obj_directory(target) );
end;

function ios.build_executable( target )
    local library_directories = {};
    if target.library_directories then
        for _, directory in ipairs(target.library_directories) do
            table.insert( library_directories, [[-L "%s"]] % directory );
        end
    end
    if target.settings.library_directories then
        for _, directory in ipairs(target.settings.library_directories) do
            table.insert( library_directories, [[-L "%s"]] % directory );
        end
    end
    if target.framework_directories then 
        for _, directory in ipairs(target.framework_directories) do
            table.insert( library_directories, [[-F "%s"]] % directory );
        end
    end
    if target.settings.framework_directories then 
        for _, directory in ipairs(target.settings.framework_directories) do
            table.insert( library_directories, [[-F "%s"]] % directory );
        end
    end
    
    local flags = {
        "-arch %s" % target.architecture,
        "-o %s" % native( target:get_filename() ),
        "-std=c++11",
        "-stdlib=libstdc++"
    };

    -- Add these to compile for iOS
    local sysroot;
    if platform == "ios" then 
        sysroot = "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS8.0.sdk";
        table.insert( flags, "-mios-version-min=6.1" );
        table.insert( flags, "-isysroot %s" % sysroot );
    elseif platform == "ios_simulator" then 
        sysroot = "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator8.0.sdk";
        table.insert( flags, "-mios-simulator-version-min=6.1" );
        table.insert( flags, "-isysroot %s" % sysroot );
    end
    table.insert( flags, "-ObjC" );
    table.insert( flags, "-all_load" );

    if target:prototype() == ArchivePrototype then
        table.insert( flags, "-shared" );
        table.insert( flags, "-Wl,--out-implib,%s" % native("%s/%s" % {target.settings.lib, lib_name(target:id())}) );
    end
    
    if target.settings.verbose_linking then
        table.insert( flags, "-Wl,--verbose=31" );
    end   

    if target.settings.strip and not target.settings.generate_dsym_bundle then
        table.insert( flags, "-Wl,-dead_strip" );
    end

    if target.settings.exported_symbols_list then
        table.insert( flags, [[-exported_symbols_list "%s"]] % absolute(target.settings.exported_symbols_list) );
    end

    local libraries = {
    };
    if target.libraries then
        for _, library in ipairs(target.libraries) do
            table.insert( libraries, "-l%s_%s" % {library:id(), variant} );
        end
    end
    if target.third_party_libraries then
        for _, library in ipairs(target.third_party_libraries) do
            table.insert( libraries, "-l%s" % library );
        end
    end
    if target.system_libraries then
        for _, library in ipairs(target.system_libraries) do 
            table.insert( libraries, "-l%s" % library );
        end
    end
    if target.frameworks then
        for _, framework in ipairs(target.frameworks) do
            table.insert( libraries, "-framework %s" % framework );
        end
    end

    local objects = {
    };
    for dependency in target:get_dependencies() do
        if dependency:prototype() == CcPrototype then
            if dependency.precompiled_header then
                table.insert( objects, leaf(dependency.precompiled_header:get_filename()) );
            end
            
            for object in dependency:get_dependencies() do
                if object:prototype() == nil and object ~= dependency.precompiled_header then
                    table.insert( objects, leaf(object:get_filename()) );
                end
            end
        end
    end

    if #objects > 0 then
        local sdk = ios.SDK_BY_PLATFORM[platform];
        local ldflags = table.concat( flags, " " );
        local lddirs = table.concat( library_directories, " " );        
        local ldobjects = table.concat( objects, " " );
        local ldlibs = table.concat( libraries, " " );

        print( leaf(target:get_filename()) );
        pushd( "%s/%s" % {obj_directory(target), target.architecture} );
        local xcrun = target.settings.ios.xcrun;
        build.system( xcrun, "xcrun --sdk %s clang++ %s %s %s %s" % {sdk, ldflags, lddirs, ldobjects, ldlibs} );
        popd();
    end
end;

function ios.clean_executable( target )
    rm( target:get_filename() );
    rmdir( obj_directory(target) );
end;

function ios.lipo_executable( target )
    local executables = {};
    for executable in target:get_dependencies() do 
        if executable:prototype() == ExecutablePrototype then
            table.insert( executables, executable:get_filename() );
        end
    end
    print( leaf(target:get_filename()) );
    local sdk = ios.SDK_BY_PLATFORM[platform];
    executables = table.concat( executables, [[" "]] );
    local xcrun = target.settings.ios.xcrun;
    build.system( xcrun, [[xcrun --sdk %s lipo -create -output "%s" "%s"]] % {sdk, target:get_filename(), executables} );
end

function ios.obj_directory( target )
    return "%s/%s_%s/%s" % { target.settings.obj, platform, variant, relative(target:get_working_directory():path(), root()) };
end;

function ios.cc_name( name )
    return "%s.c" % basename( name );
end;

function ios.cxx_name( name )
    return "%s.cpp" % basename( name );
end;

function ios.obj_name( name, architecture )
    return "%s.o" % basename( name );
end;

function ios.lib_name( name, architecture )
    return "lib%s_%s_%s.a" % { name, architecture, variant };
end;

function ios.dll_name( name )
    return "%s.dylib" % { name };
end;

function ios.exe_name( name, architecture )
    return "%s_%s" % { name, architecture };
end;

function ios.module_name( name, architecture )
    return "%s_%s" % { name, architecture };
end
