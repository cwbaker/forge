
macosx = {};

function macosx.configure( settings )
    local local_settings = build.local_settings;
    if not local_settings.macosx then
        local_settings.updated = true;
        local_settings.macosx = {
            xcrun = "/usr/bin/xcrun";
        };
    end
end

function macosx.initialize( settings )
    macosx.configure( settings );

    if platform == "macosx" then
        cc = macosx.cc;
        objc = macosx.objc;
        build_library = macosx.build_library;
        clean_library = macosx.clean_library;
        build_executable = macosx.build_executable;
        clean_executable = macosx.clean_executable;
        lipo_executable = macosx.lipo_executable;
        obj_directory = macosx.obj_directory;
        cc_name = macosx.cc_name;
        cxx_name = macosx.cxx_name;
        obj_name = macosx.obj_name;
        lib_name = macosx.lib_name;
        dll_name = macosx.dll_name;
        exe_name = macosx.exe_name;
        module_name = macosx.module_name;
    end
end

function macosx.cc( target )
    local flags = {
        '-DBUILD_OS_MACOSX'
    };
    clang.append_defines( target, flags );
    table.insert( flags, "-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk" );

    clang.append_include_directories( target, flags );
    clang.append_compile_flags( target, flags );
    
    local ccflags = table.concat( flags, " " );
    local xcrun = target.settings.macosx.xcrun;

    for dependency in target:get_dependencies() do
        if dependency:is_outdated() then
            print( leaf(dependency.source) );
            build.system( xcrun, ('xcrun --sdk macosx clang %s -o "%s" "%s"'):format(ccflags, dependency:get_filename(), absolute(dependency.source)) );
        end    
    end
end

function macosx.build_library( target )
    local flags = {
        "-static"
    };

    pushd( ("%s/%s"):format(obj_directory(target), target.architecture) );
    local objects =  {};
    for compile in target:get_dependencies() do
        local prototype = compile:prototype();
        if prototype == Cc or prototype == Cxx or prototype == ObjC or prototype == ObjCxx then
            for object in compile:get_dependencies() do
                table.insert( objects, relative(object:get_filename()) );
            end
        end
    end
    
    if #objects > 0 then
        print( leaf(target:get_filename()) );
        local arflags = table.concat( flags, " " );
        local arobjects = table.concat( objects, '" "' );
        local xcrun = target.settings.macosx.xcrun;
        build.system( xcrun, ('xcrun --sdk macosx libtool %s -o "%s" "%s"'):format(arflags, native(target:get_filename()), arobjects) );
    end
    popd();
end

function macosx.clean_library( target )
    rm( target:get_filename() );
    rmdir( obj_directory(target) );
end

function macosx.build_executable( target )
    local flags = {
        "-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk";
    };
    clang.append_link_flags( target, flags );
    clang.append_library_directories( target, flags );

    local objects = {};
    local libraries = {};

    pushd( ("%s/%s"):format(obj_directory(target), target.architecture) );
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

    clang.append_link_libraries( target, libraries );

    if #objects > 0 then
        local ldflags = table.concat( flags, " " );
        local ldobjects = table.concat( objects, '" "' );
        local ldlibs = table.concat( libraries, " " );
        local xcrun = target.settings.macosx.xcrun;

        print( leaf(target:get_filename()) );
        build.system( xcrun, ('xcrun --sdk macosx clang++ %s "%s" %s'):format(ldflags, ldobjects, ldlibs) );
    end
    popd();
end

function macosx.clean_executable( target )
    rm( target:get_filename() );
    rmdir( obj_directory(target) );
end

function macosx.lipo_executable( target )
    local executables = {};
    for executable in target:get_dependencies() do 
        local prototype = executable:prototype();
        if prototype == Executable or prototype == DynamicLibrary then
            table.insert( executable, executable:get_filename() );
        end
    end
    executables = table.concat( executables, '" "' );
    print( leaf(target:get_filename()) );
    local xcrun = target.settings.macosx.xcrun;
    build.system( xcrun, ('xcrun lipo -create "%s" -output "%s"'):format(executables, target:get_filename()) );
end

function macosx.obj_directory( target )
    return ("%s/%s_%s/%s"):format( target.settings.obj, platform, variant, relative(target:get_working_directory():path(), root()) );
end

function macosx.cc_name( name )
    return ("%s.c"):format( basename(name) );
end

function macosx.cxx_name( name )
    return ("%s.cpp"):format( basename(name) );
end

function macosx.obj_name( name, architecture )
    return ("%s.o"):format( basename(name) );
end

function macosx.lib_name( name, architecture )
    return ("lib%s_%s.a"):format( name, architecture );
end

function macosx.dll_name( name )
    return ("%s.dylib"):format( name );
end

function macosx.exe_name( name, architecture )
    return ("%s_%s"):format( name, architecture );
end

function macosx.module_name( name, architecture )
    return ("%s_%s"):format( name, architecture );
end
