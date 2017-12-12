
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
    if platform == "windows" or platform == "" then
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
    end
end;

function msvc.append_defines( target, flags )
    table.insert( flags, ('/DBUILD_PLATFORM_%s'):format(upper(platform)) );
    table.insert( flags, ('/DBUILD_VARIANT_%s'):format(upper(variant)) );
    table.insert( flags, ('/DBUILD_LIBRARY_SUFFIX="\\"_%s.lib\\""'):format(target.architecture) );
    table.insert( flags, ('/DBUILD_MODULE_%s'):format(upper(string.gsub(target.module:id(), "-", "_"))) );
    table.insert( flags, ('/DBUILD_LIBRARY_TYPE_%s'):format(upper(target.settings.library_type)) );

    if string.find(target.settings.runtime_library, "debug", 1, true) then
        table.insert( flags, "/D_DEBUG" );
        table.insert( flags, "/DDEBUG" );
    else 
        table.insert( flags, "/DNDEBUG" );
    end

    if target.settings.defines then
        for _, define in ipairs(target.settings.defines) do
            table.insert( flags, ("/D%s"):format(define) );
        end
    end

    if target.defines then
        for _, define in ipairs(target.defines) do
            table.insert( flags, ("/D%s"):format(define) );
        end
    end
end

function msvc.append_version_defines( target, flags )
    table.insert( flags, ('/DBUILD_VERSION="\\"%s\\""'):format(version) );
end

function msvc.append_include_directories( target, flags )
    if target.include_directories then
        for _, directory in ipairs(target.include_directories) do
            table.insert( flags, ('/I "%s"'):format(relative(directory)) );
        end
    end

    if target.settings.include_directories then
        for _, directory in ipairs(target.settings.include_directories) do
            table.insert( flags, ('/I "%s"'):format(directory) );
        end
    end
end

function msvc.append_compile_flags( target, flags )
    table.insert( flags, "/nologo" );
    table.insert( flags, "/FS" );
    table.insert( flags, "/FC" );
    table.insert( flags, "/c" );

    local language = target.language or "c++";
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
    else
        assert( false, "Only the 'c' and 'c++' languages are supported by Microsoft Visual C++" );
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
        local pdb = ("%s%s"):format(obj_directory(target), pdb_name(target.module:id()) );
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
end

function msvc.append_library_directories( target, flags )
    if target.library_directories then
        for _, directory in ipairs(target.library_directories) do
            table.insert( flags, ('/libpath:"%s"'):format(directory) );
        end
    end    

    if target.settings.library_directories then
        for _, directory in ipairs(target.settings.library_directories) do
            table.insert( flags, ('/libpath:"%s"'):format(directory) );
        end
    end    
end

function msvc.append_link_flags( target, flags )
    table.insert( flags, "/nologo" );

    local intermediate_manifest = ('%s%s_intermediate.manifest'):format( obj_directory(target), target:id() );
    table.insert( flags, "/manifest" );
    table.insert( flags, ("/manifestfile:%s"):format(intermediate_manifest) );
    
    if target.settings.subsystem then
        table.insert( flags, ("/subsystem:%s"):format(target.settings.subsystem) );
    end

    table.insert( flags, ("/out:%s"):format(native(target:filename())) );
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
end

function msvc.append_link_libraries( target, flags )
    if target.settings.third_party_libraries then
        for _, library in ipairs(target.settings.third_party_libraries) do
            table.insert( flags, ("%s.lib"):format(basename(library)) );
        end
    end
    if target.third_party_libraries then
        for _, library in ipairs(target.settings.third_party_libraries) do
            table.insert( flags, ("%s.lib"):format(basename(library)) );
        end
    end
    if target.system_libraries then
        for _, library in ipairs(target.system_libraries) do
            table.insert( flags, ("%s.lib"):format(basename(library)) );
        end
    end
end
