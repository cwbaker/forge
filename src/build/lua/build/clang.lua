
clang = {};

function clang.configure( settings )
    local local_settings = build.local_settings;
    if not local_settings.clang then
        local_settings.updated = true;
        local_settings.clang = {
            xcrun = "/usr/bin/xcrun";
        };
    end
end;

function clang.initialize( settings )
    clang.configure( settings );

    if platform == "clang" then
        cc = clang.cc;
        objc = clang.objc;
        build_library = clang.build_library;
        clean_library = clang.clean_library;
        build_executable = clang.build_executable;
        clean_executable = clang.clean_executable;
        lipo_executable = clang.lipo_executable;
        obj_directory = clang.obj_directory;
        cc_name = clang.cc_name;
        cxx_name = clang.cxx_name;
        obj_name = clang.obj_name;
        lib_name = clang.lib_name;
        dll_name = clang.dll_name;
        exe_name = clang.exe_name;
        module_name = clang.module_name;
    end
end;

function clang.cc( target )
    local defines = {
        " ",
        [[-DBUILD_OS_MACOSX]],
        [[-DBUILD_PLATFORM_%s]] % upper( platform ),
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
        " ",
        "-c",
        "-arch %s" % target.architecture,
        "-fasm-blocks",
        "-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk";
    };
    
    local language = target.language or "c++";
    if language then
        table.insert( flags, "-x %s" % language );

        if string.find(language, "c++", 1, true) then
            table.insert( flags, "-std=c++11" );
            table.insert( flags, "-stdlib=libc++" );
            table.insert( flags, "-Wno-deprecated" );
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
        
    if target.settings.debug then
        table.insert( flags, "-g3" );
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

    local compiler = "clang";
    local cppdefines = table.concat( defines, " " );
    local cppdirs = table.concat( include_directories, " " );
    local ccflags = table.concat( flags, " " );

    if target.precompiled_header ~= nil then            
        if target.precompiled_header:is_outdated() then
            print( leaf(target.precompiled_header.source) );
            local xcrun = target.settings.clang.xcrun;
            build.system( xcrun, "xcrun %s %s %s %s -o %s %s" % {compiler, cppdirs, cppdefines, ccflags, target.precompiled_header:get_filename(), target.precompiled_header.source} );
        end        
    end
    
    cppdefines = cppdefines.." -DBUILD_VERSION=\"\\\""..version.."\\\"\"";
    for dependency in target:get_dependencies() do
        if dependency:is_outdated() and dependency ~= target.precompiled_header then
            if dependency:prototype() == nil then
                print( leaf(dependency.source) );
                local xcrun = target.settings.clang.xcrun;
                build.system( xcrun, "xcrun %s %s %s %s -o %s %s" % {compiler, cppdirs, cppdefines, ccflags, dependency:get_filename(), absolute(dependency.source)} );
            elseif dependency.results then
                for _, result in ipairs(dependency.results) do
                    if result:is_outdated() then
                        print( leaf(result.source) );
                        local xcrun = target.settings.clang.xcrun;
                        build.system( xcrun, "xcrun %s %s %s %s -o %s %s" % {compiler, cppdirs, cppdefines, ccflags, result:get_filename(), absolute(result.source)} );
                    end
                end
            end
        end    
    end
end;

function clang.build_library( target )
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
        local arflags = table.concat( flags, " " );
        local arobjects = table.concat( objects, " " );

        print( leaf(target:get_filename()) );
        pushd( "%s/%s" % {obj_directory(target), target.architecture} );
        local xcrun = target.settings.clang.xcrun;
        build.system( xcrun, [[xcrun libtool %s -o %s %s]] % {arflags, native(target:get_filename()), arobjects} );
        popd();
    end
end;

function clang.clean_library( target )
    rm( target:get_filename() );
    rmdir( obj_directory(target) );
end;

function clang.build_executable( target )
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
        "-stdlib=libc++",
        "-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk";
    };

    if target:prototype() == ArchivePrototype then
        table.insert( flags, "-shared" );
        table.insert( flags, "-Wl,--out-implib,%s" % native("%s/%s" % {target.settings.lib, lib_name(target:id())}) );
    end
    
    if target.settings.verbose_linking then
        table.insert( flags, "-Wl,--verbose=31" );
    end
    
    if target.settings.strip then
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
    if target.settings.third_party_libraries then
        for _, library in ipairs(target.settings.third_party_libraries) do
            table.insert( libraries, "-l%s" % library );
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
            table.insert( libraries, [[-framework "%s"]] % framework );
        end
    end
    if target.settings.frameworks then 
        for _, framework in ipairs(target.settings.frameworks) do
            table.insert( libraries, [[-framework "%s"]] % framework );
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
        local ldflags = table.concat( flags, " " );
        local lddirs = table.concat( library_directories, " " );        
        local ldobjects = table.concat( objects, " " );
        local ldlibs = table.concat( libraries, " " );

        print( leaf(target:get_filename()) );
        pushd( "%s/%s" % {obj_directory(target), target.architecture} );
        local xcrun = target.settings.clang.xcrun;
        build.system( xcrun, "xcrun clang++ %s %s %s %s" % {ldflags, lddirs, ldobjects, ldlibs} );
        popd();
    end
end;

function clang.clean_executable( target )
    rm( target:get_filename() );
    rmdir( obj_directory(target) );
end;

function clang.lipo_executable( target )
    local executables = { 
        " "
    };
    for executable in target:get_dependencies() do 
        if executable:prototype() == LinkPrototype then
            table.insert( executable, executable:get_filename() );
        end
    end
    executables = table.concat( executables, " " );
    print( leaf(target:get_filename()) );
    local xcrun = target.settings.clang.xcrun;
    build.system( xcrun, [[xcrun lipo -create %s -output %s]] % {executables, target:get_filename()} );
end

function clang.obj_directory( target )
    return "%s/%s_%s/%s" % { target.settings.obj, platform, variant, relative(target:get_working_directory():path(), root()) };
end;

function clang.cc_name( name )
    return "%s.c" % basename( name );
end;

function clang.cxx_name( name )
    return "%s.cpp" % basename( name );
end;

function clang.obj_name( name, architecture )
    return "%s.o" % basename( name );
end;

function clang.lib_name( name, architecture )
    return "lib%s_%s_%s.a" % { name, architecture, variant };
end;

function clang.dll_name( name )
    return "%s.dylib" % { name };
end;

function clang.exe_name( name, architecture )
    return "%s_%s" % { name, architecture };
end;

function clang.module_name( name, architecture )
    return "%s_%s" % { name, architecture };
end
