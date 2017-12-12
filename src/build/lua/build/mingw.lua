
mingw = {};

function mingw.configure( settings )
    function autodetect_mingw_directory()
        local mingw_directory = "C:/MinGW";
        return mingw_directory;
    end

    local local_settings = build.local_settings;
    if not local_settings.mingw then
        local_settings.updated = true;
        local_settings.mingw = {
            mingw_directory = autodetect_mingw_directory() or "C:/MinGW";
        };
    end
end

function mingw.initialize( settings )
    mingw.configure( settings );
    if platform == "mingw" then
        -- Make sure that the environment variable VS_UNICODE_OUTPUT is not set.  
        -- Visual Studio sets this to signal its tools to communicate back to 
        -- Visual Studio using named pipes rather than stdout so that unicode output 
        -- works better but this then prevents the build tool from intercepting
        -- and collating this output.
        -- See http://blogs.msdn.com/freik/archive/2006/04/05/569025.aspx.
        putenv( "VS_UNICODE_OUTPUT", "" );

        local mingw_directory = settings.mingw.mingw_directory;
        
        local path = {
            "%s/bin" % mingw_directory,
            getenv("PATH")
        };
        putenv( "PATH", table.concat(path, ";") );
        
        local include = {
            "%s/include" % mingw_directory,
            getenv("INCLUDE")
        };
        putenv( "INCLUDE", table.concat(include, ";") );
        
        local lib = {
            "%s/lib" % mingw_directory,
            getenv("LIB")
        };
        putenv( "LIB", table.concat(lib, ";") );

        cc = mingw.cc;
        build_library = mingw.build_library;
        clean_library = mingw.clean_library;
        build_executable = mingw.build_executable;
        clean_executable = mingw.clean_executable;
        obj_directory = mingw.obj_directory;
        cc_name = mingw.cc_name;
        cxx_name = mingw.cxx_name;
        pch_name = mingw.pch_name;
        pdb_name = mingw.pdb_name;
        obj_name = mingw.obj_name;
        lib_name = mingw.lib_name;
        exp_name = mingw.exp_name;
        dll_name = mingw.dll_name;
        exe_name = mingw.exe_name;
        ilk_name = mingw.ilk_name;
        module_name = mingw.module_name;
    end
end

function mingw.cc( target )
    local gcc = "%s/bin/gcc.exe" % target.settings.mingw.mingw_directory;

    local cppdefines = "";
    cppdefines = cppdefines.." -DBUILD_OS_"..upper(operating_system());
    cppdefines = cppdefines.." -DBUILD_PLATFORM_"..upper(platform);
    cppdefines = cppdefines.." -DBUILD_VARIANT_"..upper(variant);
    cppdefines = cppdefines.." -DBUILD_LIBRARY_SUFFIX=\"\\\"_"..platform.."_"..variant..".lib\\\"\"";
    cppdefines = cppdefines.." -DBUILD_MODULE_"..upper(string.gsub(target.module:id(), "-", "_"))
    cppdefines = cppdefines.." -DBUILD_LIBRARY_TYPE_"..upper(target.settings.library_type);

    if target.settings.defines then
        for _, define in ipairs(target.settings.defines) do
            cppdefines = cppdefines.." -D"..define;
        end
    end
    
    if target.defines then
        for _, define in ipairs(target.defines) do
            cppdefines = cppdefines.." -D"..define;
        end
    end

    local cppdirs = "";
    if target.include_directories then
        for _, directory in ipairs(target.include_directories) do
            cppdirs = cppdirs.." -I\""..relative(directory).."\"";
        end
    end

    if target.settings.include_directories then
        for _, directory in ipairs(target.settings.include_directories) do
            cppdirs = cppdirs.." -I\""..directory.."\"";
        end
    end

    local ccflags = " -c";

    if target.settings.compile_as_c then
        ccflags = ccflags.." -x c";
    else
        ccflags = ccflags.." -fpermissive -Wno-deprecated -x c++";
    end
    
    if target.settings.runtime_library == "static" or target.settings.runtime_library == "static_debug" then
        ccflags = ccflags.." -static-libstdc++";
    end
    
    if target.settings.debug then
        ccflags = ccflags.." -g";
    end

    if target.settings.optimization then
        ccflags = ccflags.." -O2";
    end
    
    if target.settings.preprocess then
        ccflags = ccflags.." -E";
    end

    if target.settings.exceptions and not target.settings.compile_as_c then
        ccflags = ccflags.." -fexceptions";
    end

    if target.settings.run_time_type_info and not target.settings.compile_as_c then
        ccflags = ccflags.." -frtti";
    end

    if target.precompiled_header ~= nil then            
        if target.precompiled_header:is_outdated() then
            print( leaf(target.precompiled_header.source) );
            build.system( gcc, "gcc"..cppdirs..cppdefines..ccflags.." -o"..obj_directory(target)..obj_name(target.precompiled_header.source).." "..target.precompiled_header.source, GccScanner );
        end        
    end
    
    local GccScanner = Scanner {
        [ [[((?:[A-Z]\:)?[^\:]+)\:([0-9]+)\:[0-9]+\: ([^\:]+)\:(.*)]] ] = function( filename, line, class, message )
            print( "%s(%s): %s: %s" % {filename, line, class, message} );
        end;
    };

    cppdefines = cppdefines.." -DBUILD_VERSION=\"\\\""..version.."\\\"\"";
    for dependency in target:get_dependencies() do
        if dependency:is_outdated() and dependency ~= target.precompiled_header then
            if dependency:prototype() == nil then
                print( leaf(dependency.source) );
                build.system( gcc, "gcc"..cppdirs..cppdefines..ccflags.." -o"..dependency:get_filename().." "..dependency.source, GccScanner );
            elseif dependency.results then
                for _, result in ipairs(dependency.results) do
                    if result:is_outdated() then
                        print( leaf(result.source) );
                        build.system( gcc, "gcc"..cppdirs..cppdefines..ccflags.." -o"..dependency:get_filename().." "..result.source, GccScanner );
                    end
                end
            end
        end    
    end
end

function mingw.build_library( target )
    local ar = "%s/bin/ar.exe" % target.settings.mingw.mingw_directory;

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

function mingw.clean_library( target )
    rm( target:get_filename() );
    rmdir( obj_directory(target) );
end

function mingw.build_executable( target )
    local gxx = "%s/bin/g++.exe" % target.settings.mingw.mingw_directory;

    local ldlibs = " ";
    
    local lddirs = " -L \""..target.settings.lib.."\"";

    if target.settings.library_directories then
        for _, directory in ipairs(target.settings.library_directories) do
            lddirs = lddirs.." -L \""..directory.."\"";
        end
    end
    
    local ldflags = " ";

    ldflags = ldflags.." -o "..native( target:get_filename() );
    if target:prototype() == DynamicLibraryPrototype then
        ldflags = ldflags.." -shared -Wl,--out-implib,"..native( "%s/%s.lib" % {target.settings.lib, basename(target:id())} );
    end
    
    if target.settings.verbose_linking then
        ldflags = ldflags.." -Wl,--verbose=31";
    end
    
    if target.settings.runtime_library == "static" or target.settings.runtime_library == "static_debug" then
        ldflags = ldflags.." -static-libstdc++";
    end
    
    if target.settings.debug then
        ldflags = ldflags.." -debug";
    end

    if target.settings.generate_map_file then
        ldflags = ldflags.." -Wl,-Map,"..native(obj_directory(target)..target:id()..".map");
    end

    if target.settings.stack_size then
        ldflags = ldflags.." -Wl,--stack,"..tostring(target.settings.stack_size);
    end
    
    if target.settings.strip then
        ldflags = ldflags.." -Wl,--strip-all";
    end

    local libraries = "";
    if target.libraries then
        for _, library in ipairs(target.libraries) do
            libraries = "%s -l%s" % { libraries, basename(library:get_filename()) };
        end
    end
    if target.third_party_libraries then
        for _, library in ipairs(target.third_party_libraries) do
            libraries = "%s -l%s" % { libraries, library };
        end
    end

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
        build.system( gxx, "g++"..ldflags..lddirs..objects..libraries..ldlibs );
        popd();
    end
end 

function mingw.clean_executable( target )
    rm( target:get_filename() );
    rmdir( obj_directory(target) );
end

function mingw.obj_directory( target )
    return "%s/%s_%s/%s/" % { target.settings.obj, platform, variant, relative(target:get_working_directory():path(), root()) };
end

function mingw.cc_name( name )
    return "%s.c" % basename( name );
end

function mingw.cxx_name( name )
    return "%s.cpp" % basename( name );
end

function mingw.obj_name( name )
    return "%s.o" % basename( name );
end

function mingw.lib_name( name )
    return "%s_%s_%s.lib" % { name, platform, variant };
end

function mingw.exp_name( name )
    return "%s_%s_%s.exp" % { name, platform, variant };
end

function mingw.dll_name( name )
    return "%s_%s_%s.dll" % { name, platform, variant };
end

function mingw.exe_name( name )
    return "%s_%s_%s.exe" % { name, platform, variant };
end

function mingw.ilk_name( name )
    return "%s_%s_%s.ilk" % { name, platform, variant };
end

function mingw.module_name( name, architecture )
    return name;
end
