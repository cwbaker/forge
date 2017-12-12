
msvc = {};

function msvc.configure( settings )
    local function registry( key )
        local values = {};
        local RegQueryScanner = Scanner {
            [ [[[ ]* ([A-Za-z0-9_]+) [ ]* ([A-Za-z0-9_]+) [ ]* ([A-Za-z0-9_\(\)\\\:\. ]+)]] ] = function( key, type, value )
                values[key] = value;
            end;
            
            [ [[.*]] ] = function()
            end;
        };
        local reg = "C:/Windows/system32/reg.exe";
        local arguments = ('reg query "%s"'):format( key );
        build.system( reg, arguments, RegQueryScanner );
        return values;
    end

    local function autodetect_visual_studio_directory()
        local visual_studio_directory = os.getenv( "VS120COMNTOOLS" );
        if visual_studio_directory then
            visual_studio_directory = string.gsub( visual_studio_directory, "\\Common7\\Tools\\", "" );
        end    
        return visual_studio_directory;
    end

    local function autodetect_windows_sdk_directory()
        local windows_sdk = registry( [[HKLM\SOFTWARE\Microsoft\Windows Kits\Installed Roots]] );
        if windows_sdk then 
            return windows_sdk.KitsRoot81 or windows_sdk.KitsRoot;
        end
    end

    if operating_system() == "windows" then
        local local_settings = build.local_settings;
        if not local_settings.msvc then
            local_settings.updated = true;
            local_settings.msvc = {
                visual_studio_directory = autodetect_visual_studio_directory() or "C:/Program Files (x86)/Microsoft Visual Studio 12.0";
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
            ('%s\\Common7\\IDE'):format( visual_studio_directory ),
            ('%s\\VC\\BIN'):format( visual_studio_directory ),
            ('%s\\Common7\\Tools'):format( visual_studio_directory ),
            ('%s\\VC\\VCPackages'):format( visual_studio_directory ),
        };
        
        local include = {
            ('%s\\VC\\ATLMFC\\INCLUDE'):format( visual_studio_directory ),
            ('%s\\VC\\INCLUDE'):format( visual_studio_directory ),
        };
        
        local lib = {
            ('%s\\VC\\ATLMFC\\LIB'):format( visual_studio_directory ),
            ('%s\\VC\\LIB'):format( visual_studio_directory )
        };

        local libpath = {
            ('%s\\VC\\ATLMFC\\LIB'):format( visual_studio_directory ),
            ('%s\\VC\\LIB'):format( visual_studio_directory )
        };

        local windows_sdk_directory = settings.msvc.windows_sdk_directory;
        if windows_sdk_directory then 
            table.insert( path, ('%s\\bin'):format(windows_sdk_directory) );
            table.insert( include, ('%s\\Include\\um'):format(windows_sdk_directory) );
            table.insert( include, ('%s\\Include\\shared'):format(windows_sdk_directory) );
            table.insert( include, ('%s\\Include\\winrt'):format(windows_sdk_directory) );
            table.insert( lib, ('%s\\Lib\\winv6.3\\um\\x86'):format(windows_sdk_directory) );
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
        '/DBUILD_OS_WINDOWS',
        ('/DBUILD_PLATFORM_%s'):format( upper(platform) ),
        ('/DBUILD_VARIANT_%s'):format( upper(variant) ),
        '/DBUILD_LIBRARY_SUFFIX="\\".lib\\""',
        ('/DBUILD_MODULE_%s'):format( upper(string.gsub(target.module:id(), "-", "_")) ),
        ('/DBUILD_LIBRARY_TYPE_%s'):format( upper(target.settings.library_type) )
    };

    if string.find(target.settings.runtime_library, "debug", 1, true) then
        table.insert( defines, "/D_DEBUG" );
        table.insert( defines, "/DDEBUG" );
    else 
        table.insert( defines, "/DNDEBUG" );
    end

    if target.settings.defines then
        for _, define in ipairs(target.settings.defines) do
            table.insert( defines, ("/D%s"):format(define) );
        end
    end    
    if target.defines then
        for _, define in ipairs(target.defines) do
            table.insert( defines, ("/D%s"):format(define) );
        end
    end

    local include_directories = {};
    if target.include_directories then
        for _, directory in ipairs(target.include_directories) do
            table.insert( include_directories, ('-I "%s"'):format(relative(directory)) );
        end
    end
    if target.settings.include_directories then
        for _, directory in ipairs(target.settings.include_directories) do
            table.insert( include_directories, ('-I "%s"'):format(directory) );
        end
    end

    local flags = {
        "/nologo",
        "/FS",
        "/FC",
        "/c"
    };

    local language = target.language or "c++";
    assert( language == "c" or language == "c++", "Only the 'c' and 'c++' languages are supported by Microsoft Visual C++" );

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
        table.insert( flags, ("/Zi /Fd%s"):format(native(pdb)) );
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

    local cl = ("%s/VC/bin/cl.exe"):format( target.settings.msvc.visual_studio_directory );
    local cppdirs = table.concat( include_directories, " " );

    table.insert( defines, ('-DBUILD_VERSION="\\"%s\\""'):format(version) );
    local cppdefines = table.concat( defines, " " );
    local ccflags = table.concat( flags, " " );

    local sources_by_directory = {};
    for dependency in target:get_dependencies() do
        if dependency:is_outdated() then
            if dependency:prototype() == nil then
                local directory = branch( dependency.source );
                local sources = sources_by_directory[directory];
                if not sources then 
                    sources = {};
                    sources_by_directory[directory] = sources;
                end
                table.insert( sources, dependency.source );
            end
        end    
    end

    for directory, sources in pairs(sources_by_directory) do
        if #sources > 0 then
            local output_directory = native( ("%s%s/%s/"):format(obj_directory(target), target.architecture, directory) );
            local ccsource = table.concat( sources, '" "' );
            build.system( cl, ('cl %s %s %s /Fo%s "%s"'):format(cppdirs, cppdefines, ccflags, output_directory, ccsource) );
        end
    end
end;

function msvc.build_library( target )
    local flags = {
        "/nologo"
    };
    
    if target.settings.link_time_code_generation then
        table.insert( arflags, "/ltcg" );
    end
    
    pushd( ("%s%s"):format(obj_directory(target), target.architecture) );
    local objects = {};
    for dependency in target:get_dependencies() do
        local prototype = dependency:prototype();
        if prototype == Cc or prototype == Cxx then
            for object in dependency:get_dependencies() do
                if object:prototype() == nil then
                    table.insert( objects, relative(object:get_filename()) );
                end
            end
        end
    end
    
    if #objects > 0 then
        local msar = target.settings.msvc.visual_studio_directory.."/VC/bin/lib.exe";
        local arflags = table.concat( flags, " " );
        local arobjects = table.concat( objects, '" "' );
        print( leaf(target:get_filename()) );
        build.system( msar, ('lib %s /out:"%s" "%s"'):format(arflags, native(target:get_filename()), arobjects) );
    end
    popd();
end;

function msvc.clean_library( target )
    rm( target:get_filename() );
    rmdir( obj_directory(target) );    
end;

function msvc.build_executable( target )
    local flags = {
        "/nologo"
    };

    local intermediate_manifest = ('%s%s_intermediate.manifest'):format( obj_directory(target), target:id() );
    table.insert( flags, "/manifest" );
    table.insert( flags, ("/manifestfile:%s"):format(intermediate_manifest) );
    
    if target.settings.subsystem then
        table.insert( flags, ("/subsystem:%s"):format(target.settings.subsystem) );
    end

    table.insert( flags, ("/out:%s"):format(native(target:get_filename())) );
    if target:prototype() == DynamicLibrary then
        table.insert( flags, "/dll" );
        table.insert( flags, ("/implib:%s"):format(native(lib_name(("%s/%s"):format(target.settings.lib, target:id())))) );
    end
    
    if target.settings.verbose_linking then
        table.insert( flags, "/verbose" );
    end
    
    if target.settings.debug then
        table.insert( flags, "/debug" );
        table.insert( flags, ("/pdb:%s"):format(native(obj_directory(target), pdb_name(target:id()))) );
    end

    if target.settings.link_time_code_generation then
        table.insert( flags, "/ltcg" );
    end

    if target.settings.generate_map_file then
        table.insert( flags, ("/map:%s.map"):format(native(("%s%s"):format(obj_directory(target), target:id()))) );
    end

    if target.settings.optimization then
        table.insert( flags, "/opt:ref" );
        table.insert( flags, "/opt:icf" );
    end

    if target.settings.stack_size then
        table.insert( flags, ("/stack:%d"):format(target.settings.stack_size) );
    end

    local library_directories = {};
    if target.settings.library_directories then
        for _, directory in ipairs(target.settings.library_directories) do
            table.insert( library_directories, ('/libpath:"%s"'):format(directory) );
        end
    end
    
    local objects = {};
    local libraries = {};

    for dependency in target:get_dependencies() do
        local prototype = dependency:prototype();
        if prototype == Cc or prototype == Cxx then
            for object in dependency:get_dependencies() do
                if object:prototype() == nil then
                    table.insert( objects, obj_name(object:id()) );
                end
            end
        elseif prototype == StaticLibrary or prototype == DynamicLibrary then
            table.insert( libraries, ('%s.lib'):format(basename(dependency:get_filename())) );
        end
    end

    if target.settings.third_party_libraries then
        for _, library in ipairs(target.settings.third_party_libraries) do
            table.insert( libraries, ("%s.lib"):format(basename(library)) );
        end
    end
    if target.third_party_libraries then
        for _, library in ipairs(target.settings.third_party_libraries) do
            table.insert( libraries, ("%s.lib"):format(basename(library)) );
        end
    end
    if target.system_libraries then
        for _, library in ipairs(target.system_libraries) do
            table.insert( libraries, ("%s.lib"):format(basename(library)) );
        end
    end

    if #objects > 0 then
        local msld = target.settings.msvc.visual_studio_directory.."/VC/bin/link.exe";
        local msmt = target.settings.msvc.windows_sdk_directory.."/bin/x86/mt.exe";
        local msrc = target.settings.msvc.windows_sdk_directory.."/bin/x86/rc.exe";

        print( leaf(target:get_filename()) );
        pushd( ("%s%s"):format(obj_directory(target), target.architecture) );
        if target.settings.incremental_linking then
            local embedded_manifest = ("%s_embedded.manifest"):format( target:id() );
            local embedded_manifest_rc = ("%s_embedded_manifest.rc"):format( target:id() );
            local embedded_manifest_res = ("%s_embedded_manifest.res"):format( target:id() );

            if not exists(embedded_manifest_rc) then        
                local rc = io.open( absolute(embedded_manifest_rc), "wb" );
                assertf( rc, "Opening '%s' to write manifest failed", absolute(embedded_manifest_rc) );
                if target:prototype() == Executable then
                    rc:write( ('1 /* CREATEPROCESS_MANIFEST_RESOURCE_ID */ 24 /* RT_MANIFEST */ "%s_embedded.manifest"'):format(target:id()) );
                else
                    rc:write( ('2 /* CREATEPROCESS_MANIFEST_RESOURCE_ID */ 24 /* RT_MANIFEST */ "%s_embedded.manifest"'):format(target:id()) );
                end
                rc:close();
            end

            IgnoreOutputScanner = Scanner {
                [ [[.*]] ] = function()
                end;
            };

            local ldflags = table.concat( flags, ' ' );
            local lddirs = table.concat( library_directories, ' ' );
            local ldlibs = table.concat( libraries, ' ' );
            local ldobjects = table.concat( objects, '" "' );

            if exists(embedded_manifest) ~= true then
                build.system( msld, ('link %s %s "%s" %s'):format(ldflags, lddirs, ldobjects, ldlibs) );
                build.system( msmt, ('mt /nologo /out:"%s" /manifest "%s"'):format(embedded_manifest, intermediate_manifest) );
                build.system( msrc, ('rc /Fo"%s" "%s"'):format(embedded_manifest_res, embedded_manifest_rc), IgnoreOutputScanner );
            end

            table.insert( objects, embedded_manifest_res );
            table.insert( flags, "/incremental" );
            local ldflags = table.concat( flags, ' ' );
            local ldobjects = table.concat( objects, '" "' );

            build.system( msld, ('link %s %s "%s" %s'):format(ldflags, lddirs, ldobjects, ldlibs) );
            build.system( msmt, ('mt /nologo /out:"%s" /manifest %s'):format(embedded_manifest, intermediate_manifest) );
            build.system( msrc, ('rc /Fo"%s" %s'):format(embedded_manifest_res, embedded_manifest_rc), IgnoreOutputScanner );
            build.system( msld, ('link %s %s "%s" %s'):format(ldflags, lddirs, ldobjects, ldlibs) );
        else
            table.insert( flags, "/incremental:no" );

            local ldflags = table.concat( flags, ' ' );
            local lddirs = table.concat( library_directories, ' ' );
            local ldlibs = table.concat( libraries, ' ' );
            local ldobjects = table.concat( objects, '" "' );

            build.system( msld, ('link %s %s "%s" %s'):format(ldflags, lddirs, ldobjects, ldlibs) );
            sleep( 100 );
            build.system( msmt, ('mt /nologo -outputresource:"%s";#1 -manifest %s'):format(native(target:get_filename()), intermediate_manifest) );
        end
        popd();
    end
end;

function msvc.clean_executable( target )
    local filename = target:get_filename();
    rm( filename );
    rm( ("%s/%s.ilk"):format(branch(filename), basename(filename)) );
    rmdir( obj_directory(target) );
end

function msvc.lipo_executable( target )
end

function msvc.obj_directory( target )
    return ("%s/%s/"):format( target.settings.obj, relative(target:get_working_directory():path(), root()) );
end

function msvc.cc_name( name )
    return ("%s.c"):format( basename(name) );
end

function msvc.cxx_name( name )
    return ("%s.cpp"):format( basename(name) );
end

function msvc.pch_name( name )
    return ("%s.pch"):format( basename(name) );
end

function msvc.pdb_name( name )
    return ("%s.pdb"):format( basename(name) );
end

function msvc.obj_name( name )
    return ("%s.obj"):format( basename(name) );
end

function msvc.lib_name( name )
    return ("%s.lib"):format( name );
end

function msvc.exp_name( name )
    return ("%s.exp"):format( name );
end

function msvc.dll_name( name )
    return ("%s.dll"):format( name );
end

function msvc.exe_name( name )
    return ("%s.exe"):format( name );
end

function msvc.ilk_name( name )
    return ("%s.ilk"):format( name );
end

function msvc.module_name( name, architecture )
    return name;
end
