
android = {};

local directory_by_architecture = {
    ["armv5"] = "armeabi";
    ["armv7"] = "armeabi-v7a";
    ["mips"] = "mips";
    ["x86"] = "x86";
};

function android.configure( settings )
    function autodetect_jdk_directory()
        return "C:/Program Files/Java/jdk1.6.0_39";
    end

    function autodetect_ndk_directory()
        return "C:/android/android-ndk";
    end

    function autodetect_sdk_directory()
        return "C:/Program Files (x86)/Android/android-sdk";
    end

    local local_settings = build.local_settings;
    if not local_settings.android then
        local_settings.updated = true;
        local_settings.android = {
            jdk_directory = autodetect_jdk_directory() or "C:/Program Files/Java/jdk1.6.0_39";
            ndk_directory = autodetect_ndk_directory() or "C:/android/android-ndk";
            sdk_directory = autodetect_sdk_directory() or "C:/Program Files (x86)/Android/android-sdk";
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
    return "%s/toolchains/%s-%s/prebuilt/%s" % { android.ndk_directory, toolchain_by_architecture[architecture], android.toolchain_version, "windows" };
end

function android.platform_directory( settings, architecture )
    local android = settings.android;
    local arch_by_architecture = {
        ["armv5"] = "arm",
        ["armv7"] = "arm",
        ["mips"] = "mips",
        ["x86"] = "x86"
    };
    return "%s/platforms/%s/arch-%s" % { android.ndk_directory, android.ndk_platform, arch_by_architecture[architecture] };
end

function android.include_directories( settings, architecture )
    local android = settings.android;
    return {
        "%s/sources/cxx-stl/gnu-libstdc++/%s/libs/%s/include" % { android.ndk_directory, android.toolchain_version, directory_by_architecture[architecture] },
        "%s/sources/cxx-stl/gnu-libstdc++/%s/include" % { android.ndk_directory, android.toolchain_version }
    };
end

function android.library_directories( settings, architecture )
    return {
        "%s/usr/lib" % android.platform_directory(settings, architecture),
        "%s/sources/cxx-stl/gnu-libstdc++/%s/libs/%s" % { settings.android.ndk_directory, settings.android.toolchain_version, directory_by_architecture[architecture] }
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

        local path = {
            "%s/bin" % android.toolchain_directory(settings, "armv5")
        };
        putenv( "PATH", table.concat(path, ";") );
        
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
    local gcc = "%s/bin/arm-linux-androideabi-gcc.exe" % android.toolchain_directory( target.settings, target.architecture );

    local defines = {
        " ",
        "-DBUILD_OS_ANDROID";
        "-DBUILD_PLATFORM_%s" % upper( platform ),
        "-DBUILD_VARIANT_%s" % upper( variant ),
        "-DBUILD_LIBRARY_SUFFIX=\"\\\"_%s_%s.lib\\\"\"" % { platform, variant },
        "-DBUILD_MODULE_%s" % upper( string.gsub(target.module:id(), "-", "_") ),
        "-DBUILD_LIBRARY_TYPE_%s" % upper( target.settings.library_type ),
        "-D__ARM_ARCH_5__",
        "-D__ARM_ARCH_5T__",
        "-D__ARM_ARCH_5E__",
        "-D__ARM_ARCH_5TE__",
        "-DANDROID"
    };

    if target.settings.debug then
        table.insert( defines, "-D_DEBUG" );
        table.insert( defines, "-DDEBUG" );
    else
        table.insert( defines, "-DNDEBUG" );
    end

    if target.settings.defines then
        for _, define in ipairs(target.settings.defines) do
            table.insert( defines, " -D%s" % define );
        end
    end    
    if target.defines then
        for _, define in ipairs(target.defines) do
            table.insert( defines, " -D%s" % define );
        end
    end

    local include_directories = {
        " "    
    };
    for _, directory in ipairs(android.include_directories(target.settings, target.architecture)) do
        assert( exists(directory), "The include directory '%s' does not exist" % directory );
        table.insert( include_directories, [[ -I"%s"]] % directory );
    end
    if target.include_directories then
        for _, directory in ipairs(target.include_directories) do
            table.insert( include_directories, [[ -I"%s"]] % relative(directory) );
        end
    end
    if target.settings.include_directories then
        for _, directory in ipairs(target.settings.include_directories) do
            table.insert( include_directories, [[ -I"%s"]] % directory );
        end
    end

    local flags = {
        " ",
        "--sysroot=%s" % android.platform_directory( target.settings, target.architecture ),
        "-c",
        "-fpic",
        "-ffunction-sections",
        "-funwind-tables",
        "-fstack-protector",
        "-no-canonical-prefixes",
        "-fomit-frame-pointer",
        "-fno-strict-aliasing",
        "-finline",
        "-finline-limit=64",
        "-march=armv5te",
        "-mtune=xscale",
        "-msoft-float",
        "-mthumb";
        "-Wa,--noexecstack"
    };

    if target.settings.compile_as_c then
        table.insert( flags, "-x c" );
    else
        table.insert( flags, "-fpermissive" );
        table.insert( flags, "-Wno-deprecated" );
        table.insert( flags, "-x c++" );
    end
    
    if target.settings.debug then
        table.insert( flags, "-g" );
    end

    if target.settings.optimization then
        table.insert( flags, "-O2" );
        table.insert( flags, "-Os" );
    end
    
    if target.settings.preprocess then
        table.insert( flags, "-E" );
    end

    if target.settings.exceptions and not target.settings.compile_as_c then
        table.insert( flags, "-fexceptions" );
    end

    if target.settings.run_time_type_info and not target.settings.compile_as_c then
        table.insert( flags, "-frtti" );
    end

    local cppdefines = table.concat( defines, " " );
    local cppdirs = table.concat( include_directories, " " );
    local ccflags = table.concat( flags, " " );

    local GccScanner = Scanner {
        [ [[((?:[A-Z]\:)?[^\:]+)\:([0-9]+)\:[0-9]+\: ([^\:]+)\:(.*)]] ] = function( filename, line, class, message )
            print( "%s(%s): %s: %s" % {filename, line, class, message} );
        end;
    };

    if target.precompiled_header ~= nil then
        if target.precompiled_header:is_outdated() then
            print( leaf(target.precompiled_header.source) );
            build.system( gcc, "arm-linux-androideabi-gcc"..cppdirs..cppdefines..ccflags.." -o"..obj_directory(target)..obj_name(target.precompiled_header.source).." "..target.precompiled_header.source, GccScanner );
        end        
    end
    
    cppdefines = cppdefines.." -DBUILD_VERSION=\"\\\""..version.."\\\"\"";
    for dependency in target:get_dependencies() do
        if dependency:is_outdated() and dependency ~= target.precompiled_header then
            if dependency:prototype() == nil then
                print( leaf(dependency.source) );
                build.system( gcc, "arm-linux-androideabi-gcc"..cppdirs..cppdefines..ccflags.." -o"..dependency:get_filename().." "..dependency.source, GccScanner );
            elseif dependency.results then
                for _, result in ipairs(dependency.results) do
                    if result:is_outdated() then
                        print( leaf(result.source) );
                        build.system( gcc, "arm-linux-androideabi-gcc"..cppdirs..cppdefines..ccflags.." -o"..obj_directory(target)..obj_name(result.source).." "..result.source, GccScanner );
                    end
                end
            end
        end    
    end
end

function android.build_library( target )
    local ar = "%s/bin/arm-linux-androideabi-ar.exe" % android.toolchain_directory( target.settings, target.architecture );

    local arflags = " ";
    
    local objects = "";
    for dependency in target:get_dependencies() do
        if dependency:prototype() == CxxPrototype then
            if dependency.precompiled_header ~= nil then
                objects = objects.." "..obj_name( dependency.precompiled_header:id() );
            end
            
            for object in dependency:get_dependencies() do
                if object:prototype() == nil and object ~= dependency.precompiled_header then
                    objects = objects.." "..obj_name( object:id() );
                end
            end
        end
    end
    
    if objects ~= "" then
        print( leaf(target:get_filename()) );
        pushd( "%s/%s" % {obj_directory(target), target.architecture} );
        build.system( ar, "ar"..arflags.." -rcs "..native(target:get_filename())..objects );
        popd();
    end
end

function android.clean_library( target )
    rm( target:get_filename() );
    rmdir( obj_directory(target) );
end

function android.build_executable( target )
    local gxx = "%s/bin/arm-linux-androideabi-g++.exe" % android.toolchain_directory( target.settings, target.architecture );

    local flags = { 
        " ",
        "--sysroot=%s" % android.platform_directory( target.settings, target.architecture ),
        "-Wl,-soname,%s" % leaf(target:get_filename()),
        "-shared",
        "-no-canonical-prefixes",
        "-Wl,--no-undefined",
        "-Wl,-z,noexecstack",
        "-Wl,-z,relro",
        "-Wl,-z,now",
        "-o %s" % native( target:get_filename() )
    };
    
    if target.settings.verbose_linking then
        table.insert( flags, "--verbose" );
    end
    
    if target.settings.debug then
        table.insert( flags, "-debug" );
    end

    if target.settings.generate_map_file then
        table.insert( flags, "-Wl,-Map,%s" % native("%s/%s.map" % {obj_directory(target), target:id()}) );
    end

    if target.settings.strip then
        table.insert( flags, "-Wl,--strip-all" );
    end

    local library_directories = {
        " ",
        [[-L"%s"]] % target.settings.lib;
    };
    for _, directory in ipairs(android.library_directories(target.settings, target.architecture)) do
        table.insert( library_directories, [[-L"%s"]] % directory );
    end
    if target.settings.library_directories then
        for _, directory in ipairs(target.settings.library_directories) do
            table.insert( library_directories, [[-L"%s"]] % directory );
        end
    end
    
    local libraries = {
        " "
    };
    if target.libraries then
        for _, library in ipairs(target.libraries) do
            table.insert( libraries, "-l%s_%s_%s" % {library:id(), platform, variant} );
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
            local destination = "%s/lib%s.so" % { branch(target:get_filename()), library };
            if not exists(destination) then 
                print( "lib%s.so" % library );
                for _, directory in ipairs(android.library_directories(target.settings, target.architecture)) do
                    local source = "%s/lib%s.so" % { directory, library };
                    if exists(source) then
                        cp( source, destination );
                        break;
                    end
                end
            end
        end
    end

    local objects = {
        " "
    };
    for dependency in target:get_dependencies() do
        if dependency:prototype() == CxxPrototype then
            if dependency.precompiled_header ~= nil then
                table.insert( objects, obj_name(dependency.precompiled_header:id()) );
            end            
            for object in dependency:get_dependencies() do
                if object:prototype() == nil and object ~= dependency.precompiled_header then
                    table.insert( objects, obj_name(object:id()) );
                end
            end
        end
    end

    if #objects > 0 then
        local ldflags = table.concat( flags, " " );
        local lddirs = table.concat( library_directories, " " );        
        local ldobjects = table.concat( objects, " " );
        local ldlibs = table.concat( libraries, " " );

        print( leaf(target:get_filename()) );
        pushd( "%s/%s" % {obj_directory(target), target.architecture} );
        build.system( gxx, "arm-linux-androideabi-g++"..ldflags..lddirs..ldobjects..ldlibs );
        popd();
    end
end 

function android.clean_executable( target )
    rm( target:get_filename() );
    rmdir( obj_directory(target) );
end

function android.obj_directory( target )
    return "%s/%s_%s/%s" % { target.settings.obj, platform, variant, relative(target:get_working_directory():path(), root()) };
end

function android.cc_name( name )
    return "%s.c" % basename( name );
end

function android.cxx_name( name )
    return "%s.cpp" % basename( name );
end

function android.obj_name( name, architecture )
    return "%s.o" % basename( name );
end

function android.lib_name( name, architecture )
    return "lib%s_%s_%s_%s.a" % { name, architecture, platform, variant };
end

function android.dll_name( name, architecture )
    return "lib/%s/lib%s.so" % { directory_by_architecture[architecture], name };
end

function android.exe_name( name, architecture )
    return "%s_%s_%s_%s" % { name, architecture, platform, variant };
end

function android.module_name( name, architecture )
    return "%s_%s" % { name, architecture };
end
