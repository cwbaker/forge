
msvc = {};

function msvc.configure( settings )
    local function registry( key )
        local values = {};
        local RegQueryScanner = Scanner {
            [ [[[ ]* ([A-Za-z0-9_]+) [ ]* ([A-Za-z0-9_]+) [ ]* ([A-Za-z0-9_\\\:\. ]+)]] ] = function( key, type, value )
                values[key] = value;
            end;
            
            [ [[.*]] ] = function()
            end;
        };
        local reg = "C:/Windows/system32/reg.exe";
        local arguments = [[reg query "%s"]] % key;
        build.system( reg, arguments, RegQueryScanner );
        return values;
    end

    local function autodetect_visual_studio_directory()
        local visual_studio_directory = os.getenv( "VS100COMNTOOLS" ) or os.getenv( "VS120COMNTOOLS" );
        if visual_studio_directory then
            visual_studio_directory = string.gsub( visual_studio_directory, "\\Common7\\Tools\\", "" );
        end    
        return visual_studio_directory;
    end

    local function autodetect_windows_sdk_directory()
        local windows_sdk = registry( [[HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows]] ) or "C:\\Program Files (x86)\\Windows Kits\\8.1";
        return windows_sdk.CurrentInstallFolder;
    end

    if operating_system() == "windows" then
        local local_settings = build.local_settings;
        if not local_settings.msvc then
            local_settings.updated = true;
            local_settings.msvc = {
                visual_studio_directory = autodetect_visual_studio_directory() or "C:/Program Files/Microsoft Visual Studio 12.0";
                windows_sdk_directory = autodetect_windows_sdk_directory();
            };
        end
    end
end;

function msvc.initialize( settings )
    msvc.configure( settings );
    if platform == "msvc" or platform == "" then
        -- Make sure that the environment variable VS_UNICODE_OUTPUT is not set.  
        -- Visual Studio sets this to signal its tools to communicate back to 
        -- Visual Studio using named pipes rather than stdout so that unicode output 
        -- works better but this then prevents the build tool from intercepting
        -- and collating this output.
        -- See http://blogs.msdn.com/freik/archive/2006/04/05/569025.aspx.
        putenv( "VS_UNICODE_OUTPUT", "" );

        local visual_studio_directory = settings.msvc.visual_studio_directory;
        
        local path = {
            getenv( "PATH" ),
            [[%s\Common7\IDE]] % visual_studio_directory,
            [[%s\VC\BIN]] % visual_studio_directory,
            [[%s\Common7\Tools]] % visual_studio_directory,
            [[%s\VC\VCPackages]] % visual_studio_directory
        };
        
        local include = {
            [[%s\VC\ATLMFC\INCLUDE]] % visual_studio_directory,
            [[%s\VC\INCLUDE]] % visual_studio_directory
        };
        
        local lib = {
            [[%s\VC\ATLMFC\LIB]] % visual_studio_directory,
            [[%s\VC\LIB]] % visual_studio_directory
        };

        local libpath = {
            [[%s\VC\ATLMFC\LIB]] % visual_studio_directory,
            [[%s\VC\LIB]] % visual_studio_directory
        };

        local windows_sdk_directory = settings.msvc.windows_sdk_directory;
        if windows_sdk_directory then 
            table.insert( path, [[%s\bin]] % windows_sdk_directory );
            table.insert( include, [[%s\Include\um]] % windows_sdk_directory );
            table.insert( include, [[%s\Include\shared]] % windows_sdk_directory );
            table.insert( include, [[%s\Include\winrt]] % windows_sdk_directory );
            table.insert( lib, [[%s\Lib\winv6.3\um\x86]] % windows_sdk_directory );
        end

        putenv( "PATH", table.concat(path, ";") );
        putenv( "INCLUDE", table.concat(include, ";") );
        putenv( "LIB", table.concat(lib, ";") );
        putenv( "LIBPATH", table.concat(libpath, ";") );

        cc = msvc.cc;
        build_library = msvc.build_library;
        clean_library = msvc.clean_library;
        build_executable = msvc.build_executable;
        clean_executable = msvc.clean_executable;
        lipo_executable = msvc.lipo_executable;
        obj_directory = msvc.obj_directory;
        cc_name = msvc.cc_name;
        cxx_name = msvc.cxx_name;
        pch_name = msvc.pch_name;
        pdb_name = msvc.pdb_name;
        obj_name = msvc.obj_name;
        lib_name = msvc.lib_name;
        exp_name = msvc.exp_name;
        dll_name = msvc.dll_name;
        exe_name = msvc.exe_name;
        ilk_name = msvc.ilk_name;
        module_name = msvc.module_name;
    end
end;

function msvc.cc( target )
    local defines = {
        [[/DBUILD_OS_WINDOWS]],
        [[/DBUILD_PLATFORM_%s]] % upper( platform ),
        [[/DBUILD_VARIANT_%s]] % upper( variant ),
        [[/DBUILD_LIBRARY_SUFFIX="\"_%s_%s.lib\""]] % { platform, variant },
        [[/DBUILD_MODULE_%s]] % upper( string.gsub(target.module:id(), "-", "_") ),
        [[/DBUILD_LIBRARY_TYPE_%s]] % upper( target.settings.library_type )
    };
    if target.settings.debug then 
        table.insert( defines, "/D_DEBUG" );
        table.insert( defines, "/DDEBUG" );
    else 
        table.insert( defines, "/DNDEBUG" );
    end
    if target.settings.defines then
        for _, define in ipairs(target.settings.defines) do
            table.insert( defines, "/D%s" % define );
        end
    end    
    if target.defines then
        for _, define in ipairs(target.defines) do
            table.insert( defines, "/D%s" % define );
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

    local flags = {
        "/nologo",
        "/FS",
        "/FC",
        "/c"
    };

    local language = target.language or "c++";
    assert( language == "c" or language == "c++", "Only 'c' and 'c++' languages are supported by Microsoft Visual C++" );

    if language == "c" then 
        table.insert( flags, "/TC" );
    elseif language == "c++" then
        table.insert( flags, "/TP" );
        if target.settings.exceptions then
            table.insert( flags, "/EHsc" );
        end
        if target.settings.run_time_type_info then
            table.insert( flags, "/GR" );
        end
    end

    if target.settings.runtime_library == "static" then
        table.insert( flags, "/MT" );
    elseif target.settings.runtime_library == "static_debug" then
        table.insert( flags, "/MTd" );
    elseif target.settings.runtime_library == "dynamic" then
        table.insert( flags, "/MD" );
    elseif target.settings.runtime_library == "dynamic_debug" then
        table.insert( flags, "/MDd" );
    end
    
    if target.settings.debug then
        local pdb = obj_directory(target)..pdb_name( target.module:id() );
        table.insert( flags, "/Zi /Fd%s" % native(pdb) );
    end

    if target.settings.link_time_code_generation then
        table.insert( flags, "/GL" );
    end

    if target.settings.minimal_rebuild then
        table.insert( flags, "/Gm" );
    end

    if target.settings.optimization then
        table.insert( flags, "/GF /O2 /Ot /Oi /GS-" );
    end

    if target.settings.preprocess then
        table.insert( flags, "/P /C" );
    end

    if target.settings.run_time_checks then
        table.insert( flags, "/RTC1" );
    end

    local mscc = "%s/VC/bin/cl.exe" % target.settings.msvc.visual_studio_directory;
    local cppdirs = table.concat( include_directories, " " );

    if target.precompiled_header ~= nil then            
        local cppdefines = table.concat( defines, " " );
        local ccflags = table.concat( flags, " " );

        if target.precompiled_header:is_outdated() then
            local pch = native( "%s%s" % {obj_directory(target), pch_name(target.precompiled_header:id())} );
            local header = native( target.precompiled_header.header );
            local obj = native( "%s%s" % {obj_directory(target), target.precompiled_header.source} );
            print( leaf(target.precompiled_header.source) );
            build.system( mscc, "cl %s %s %s /Fp%s /Yc%s /Fo%s" % {cppdefines, cppdirs, ccflags, pch, header, obj} );
        end

        table.insert( flags, "/Fp%s" % native("%s%s" % obj_directory(target), pch_name(target.precompiled_header:id())) );
        table.insert( flags, "/Yu%s" % target.precompiled_header.header );
    end
    
    table.insert( defines, [[-DBUILD_VERSION="\"%s\""]] % version );
    local cppdefines = table.concat( defines, " " );
    local ccflags = table.concat( flags, " " );

    local sources = {};
    for dependency in target:get_dependencies() do
        if dependency:is_outdated() and dependency ~= target.precompiled_header then            
            if dependency:prototype() == nil then
                table.insert( sources, dependency.source );
            elseif dependency.results then
                for _, result in ipairs(dependency.results) do
                    if result:is_outdated() then
                        table.insert( sources, result.source );
                    end
                end
            end
        end    
    end

    if #sources > 0 then
        local output_directory = native( "%s%s/" % {obj_directory(target), target.architecture} );
        local ccsource = table.concat( sources, " " );
        build.system( mscc, "cl %s %s %s /Fo%s %s" % {cppdirs, cppdefines, ccflags, output_directory, ccsource} );
    end
end;

function msvc.build_library( target )
    local msar = target.settings.msvc.visual_studio_directory.."/VC/bin/lib.exe";

    local arflags = " /nologo";
    
    if target.settings.link_time_code_generation then
        arflags = arflags.." /ltcg";
    end
    
    local objects = "";
    for dependency in target:get_dependencies() do
        if dependency:prototype() == CcPrototype then
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
        pushd( "%s%s" % {obj_directory(target), target.architecture} );
        build.system( msar, "lib"..arflags.." /out:"..native(target:get_filename())..objects );
        popd();
    end
end;

function msvc.clean_library( target )
    rm( target:get_filename() );
    rmdir( obj_directory(target) );    
end;

function msvc.build_executable( target )
    local msld = target.settings.msvc.visual_studio_directory.."/VC/bin/link.exe";
    local msmt = target.settings.msvc.windows_sdk_directory.."/bin/x86/mt.exe";
    local msrc = target.settings.msvc.windows_sdk_directory.."/bin/x86/rc.exe";

    local ldlibs = "";

    local lddirs = "";
    if target.settings.library_directories then
        for _, directory in ipairs(target.settings.library_directories) do
            lddirs = lddirs.." /libpath:\""..directory.."\"";
        end
    end
    
    local ldflags = " /nologo";

    local intermediate_manifest = obj_directory(target)..target:id().."_intermediate.manifest";
    ldflags = ldflags.." /manifest /manifestfile:"..intermediate_manifest;
    
    if target.settings.subsystem then
        ldflags = ldflags.." /subsystem:"..target.settings.subsystem;
    end

    ldflags = ldflags.." /out:"..native( target:get_filename() );
    if target:prototype() == DynamicLibraryPrototype then
        ldflags = ldflags.." /dll /implib:"..native( lib_name("%s/%s" % {target.settings.lib, target:id()}) );
    end
    
    if target.settings.verbose_linking then
        ldflags = ldflags.." /verbose";
    end
    
    if target.settings.debug then
        ldflags = ldflags.." /debug /pdb:"..native( obj_directory(target)..pdb_name(target:id()) );
    end

    if target.settings.link_time_code_generation then
        ldflags = ldflags.." /ltcg";
    end

    if target.settings.generate_map_file then
        ldflags = ldflags.." /map:"..native(obj_directory(target)..target:id()..".map");
    end

    if target.settings.optimization then
        ldflags = ldflags.." /opt:ref /opt:icf";
    end

    if target.settings.stack_size then
        ldflags = ldflags.." /stack:"..tostring(target.settings.stack_size);
    end

    local libraries = "";
    if target.libraries then
        for _, library in ipairs(target.libraries) do
            libraries = "%s %s.lib" % { libraries, basename(library:get_filename()) };
        end
    end
    if target.third_party_libraries then
        for _, library in ipairs(target.third_party_libraries) do
            libraries = "%s %s.lib" % { libraries, basename(library) };
        end
    end
    if target.system_libraries then
        for _, library in ipairs(target.system_libraries) do
            libraries = "%s %s.lib" % { libraries, basename(library) };
        end
    end

    local objects = "";
    for dependency in target:get_dependencies() do
        if dependency:prototype() == CcPrototype then
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
        pushd( "%s%s" % {obj_directory(target), target.architecture} );
        if target.settings.incremental_linking then
            local embedded_manifest = target:id().."_embedded.manifest";
            local embedded_manifest_rc = target:id().."_embedded_manifest.rc";
            local embedded_manifest_res = target:id().."_embedded_manifest.res";

            if exists(embedded_manifest_rc) ~= true then        
                local rc = io.open( absolute(embedded_manifest_rc), "wb" );
                assert( rc, string.format("Opening '%s' to write manifest failed", absolute(embedded_manifest_rc)) );
                if target:prototype() == ExecutablePrototype then
                    rc:write( "1 /* CREATEPROCESS_MANIFEST_RESOURCE_ID */ 24 /* RT_MANIFEST */ \""..target:id().."_embedded.manifest\"" );
                else
                    rc:write( "2 /* CREATEPROCESS_MANIFEST_RESOURCE_ID */ 24 /* RT_MANIFEST */ \""..target:id().."_embedded.manifest\"" );
                end
                rc:close();
            end

            IgnoreOutputScanner = Scanner {
                [ [[.*]] ] = function()
                end;
            };

            if exists(embedded_manifest) ~= true then        
                build.system( msld, "link"..ldflags..lddirs..objects..libraries..ldlibs );
                build.system( msmt, "mt /nologo /out:\""..embedded_manifest.."\" /manifest "..intermediate_manifest );
                build.system( msrc, "rc /Fo\""..embedded_manifest_res.."\" "..embedded_manifest_rc, IgnoreOutputScanner );
            end

            objects = objects.." "..embedded_manifest_res;
            ldflags = ldflags.." /incremental";
            
            build.system( msld, "link"..ldflags..lddirs..objects..libraries..ldlibs );
            build.system( msmt, "mt /nologo /out:\""..embedded_manifest.."\" /manifest "..intermediate_manifest );
            build.system( msrc, "rc /Fo\""..embedded_manifest_res.."\" "..embedded_manifest_rc, IgnoreOutputScanner );
            build.system( msld, "link"..ldflags..lddirs..objects..libraries..ldlibs );
        else
            ldflags = ldflags.." /incremental:no";
            build.system( msld, "link"..ldflags..lddirs..objects..libraries..ldlibs );
            sleep( 100 );
            build.system( msmt, "mt /nologo -outputresource:"..native(target:get_filename())..";#1 -manifest "..intermediate_manifest );
        end
        popd();
    end
end;

function msvc.clean_executable( target )
    local filename = target:get_filename();
    rm( filename );
    rm( "%s/%s.ilk" % {branch(filename), basename(filename)} );
    rmdir( obj_directory(target) );
end

function msvc.lipo_executable( target )
end

function msvc.obj_directory( target )
    return "%s/%s/" % { target.settings.obj, relative(target:get_working_directory():path(), root()) };
end

function msvc.cc_name( name )
    return "%s.c" % basename( name );
end

function msvc.cxx_name( name )
    return "%s.cpp" % basename( name );
end

function msvc.pch_name( name )
    return "%s.pch" % basename( name );
end

function msvc.pdb_name( name )
    return "%s.pdb" % basename( name );
end

function msvc.obj_name( name )
    return "%s.obj" % basename( name );
end

function msvc.lib_name( name )
    return "%s_%s_%s.lib" % { name, platform, variant };
end

function msvc.exp_name( name )
    return "%s_%s_%s.exp" % { name, platform, variant };
end

function msvc.dll_name( name )
    return "%s_%s_%s.dll" % { name, platform, variant };
end

function msvc.exe_name( name )
    return "%s_%s_%s.exe" % { name, platform, variant };
end

function msvc.ilk_name( name )
    return "%s_%s_%s.ilk" % { name, platform, variant };
end

function msvc.module_name( name, architecture )
    return name;
end
