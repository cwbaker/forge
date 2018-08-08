
msvc = {};

function msvc.configure( settings )
    local function vswhere()
        local vswhere = 'C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe';
        if build:exists( vswhere ) then 
            local environment = {
                ProgramData = build:getenv( 'ProgramData' );
                SystemRoot = build:getenv( 'SystemRoot' );
            };
            local values = {};
            build:system( vswhere, 'vswhere -latest', environment, nil, function(line)
                local key, value = line:match( '([%w_]+): ([^\n\r]+)' );
                if key and value then 
                    values[key] = value;
                end
            end );
            return values;
        end
    end

    local function vc_default_version( installation_path )
        local filename = ('%s/VC/Auxiliary/Build/Microsoft.VCToolsVersion.default.txt'):format( installation_path );
        local file = assert( io.open(filename, 'rb') );
        local version = file:read( '*all' );
        file:close();
        return version:match('([^\n\r]+)');
    end

    local function registry( key )
        local values = {};
        local reg = "C:/Windows/system32/reg.exe";
        local arguments = ('reg query "%s"'):format( key );
        build:system( reg, arguments, nil, nil, function(line)
            local REG_QUERY_PATTERN = "%s* ([%w_]+) %s* ([%w_]+) %s* ([^\n\r]+)";
            local key, type_, value = line:match( REG_QUERY_PATTERN );
            if key and type_ and value then 
                values[key] = value;
            end
        end );
        return values;
    end

    local function autodetect_toolset_version()
        local visual_studio_directory;
        local values = vswhere();
        if values then 
            return 15;
        end
        for version = 14, 10, -1 do 
            local visual_studio_directory = os.getenv( ('VS%d0COMNTOOLS'):format(version) );
            if visual_studio_directory then 
                return version;
            end
        end
    end

    local function autodetect_visual_studio_directory( toolset_version )
        if toolset_version >= 15 then 
            local visual_studio_directory;
            local values = vswhere();
            if values then 
                return values.installationPath;
            end
        end
        local visual_studio_directory = os.getenv( ("VS%d0COMNTOOLS"):format(toolset_version) );
        if visual_studio_directory then
            visual_studio_directory = string.gsub( visual_studio_directory, "\\Common7\\Tools\\", "" );
            return visual_studio_directory;
        end
    end

    local function autodetect_visual_cxx_directory( toolset_version )
        if toolset_version >= 15 then 
            local values = vswhere();
            if values then 
                local visual_studio_directory = values.installationPath;
                local vc_version = vc_default_version( visual_studio_directory );
                if visual_studio_directory and vc_version then
                    return ('%s\\VC\\Tools\\MSVC\\%s'):format( visual_studio_directory, vc_version );
                end
            end
        end
        local visual_studio_directory = os.getenv( ("VS%d0COMNTOOLS"):format(toolset_version) );
        if visual_studio_directory then
            visual_studio_directory = string.gsub( visual_studio_directory, "\\Common7\\Tools\\", "" );
            return ('%s\\VC'):format( visual_studio_directory );
        end
    end

    local function autodetect_windows_sdk_directory()
        local windows_sdk = registry( [[HKLM\SOFTWARE\Microsoft\Windows Kits\Installed Roots]] );
        if windows_sdk.KitsRoot10 then 
            return windows_sdk.KitsRoot10;
        end
    end

    local function autodetect_windows_sdk_version()
        local windows_sdk = registry( [[HKLM\SOFTWARE\Microsoft\Windows Kits\Installed Roots]] );
        if windows_sdk.KitsRoot10 then 
            local latest_version;
            for filename in build:ls(('%s/bin'):format(windows_sdk.KitsRoot10)) do
                local version = build:leaf( filename ):match('10%.%d+%.%d+%.%d+');
                if version then
                    latest_version = version;
                end
            end
            return latest_version;
        end
    end

    if build:operating_system() == 'windows' then
        local local_settings = build.local_settings;
        if not local_settings.msvc then
            local toolset_version = autodetect_toolset_version();
            local_settings.updated = true;
            local_settings.msvc = {
                toolset_version = toolset_version;
                visual_studio_directory = autodetect_visual_studio_directory( toolset_version );
                visual_cxx_directory = autodetect_visual_cxx_directory( toolset_version );
                windows_sdk_directory = autodetect_windows_sdk_directory();
                windows_sdk_version = autodetect_windows_sdk_version();
            };
        end
    end
end

function msvc.initialize( settings )
    if build:platform_matches("windows") then
        -- Make sure that the environment variable VS_UNICODE_OUTPUT is not set.
        --
        -- Visual Studio sets this to signal its tools to communicate back to 
        -- Visual Studio using named pipes rather than stdout so that unicode output 
        -- works better but this then prevents the build tool from intercepting
        -- and collating this output.
        --
        -- See http://blogs.msdn.com/freik/archive/2006/04/05/569025.aspx.
        -- putenv( "VS_UNICODE_OUTPUT", "" );

        local toolset_version = settings.msvc.toolset_version;
        local visual_studio_directory = settings.msvc.visual_studio_directory;
        local visual_cxx_directory = settings.msvc.visual_cxx_directory;

        local path_i386, path_x86_64, lib_i386, lib_x86_64, include;

        if toolset_version >= 15 then 
            path_i386 = {
                ('%s\\bin\\Hostx64\\x86'):format( visual_cxx_directory ),
                ('%s\\Common7\\IDE'):format( visual_studio_directory ),
                ('%s\\Common7\\Tools'):format( visual_studio_directory ),
            };

            path_x86_64 = {
                ('%s\\bin\\Hostx64\\x64'):format( visual_cxx_directory ),
                ('%s\\Common7\\IDE'):format( visual_studio_directory ),
                ('%s\\Common7\\Tools'):format( visual_studio_directory ),
            };
            
            lib_i386 = {
                ('%s\\atlmfc\\lib\\x86'):format( visual_cxx_directory ),
                ('%s\\lib\\x86'):format( visual_cxx_directory )
            };

            lib_x86_64 = {
                ('%s\\VC\\atlmfc\\lib\\x64'):format( visual_cxx_directory ),
                ('%s\\lib\\x64'):format( visual_cxx_directory )
            };

            include = {
                ('%s\\atlmfc\\include'):format( visual_cxx_directory ),
                ('%s\\include'):format( visual_cxx_directory ),
            };
        else
            path_i386 = {
                ('%s\\VC\\bin\\amd64_x86'):format( visual_studio_directory ),
                ('%s\\Common7\\IDE'):format( visual_studio_directory ),
                ('%s\\Common7\\Tools'):format( visual_studio_directory ),
                ('%s\\VC\\vcpackages'):format( visual_studio_directory ),
            };

            path_x86_64 = {
                ('%s\\VC\\bin\\amd64'):format( visual_studio_directory ),
                ('%s\\Common7\\IDE'):format( visual_studio_directory ),
                ('%s\\Common7\\Tools'):format( visual_studio_directory ),
                ('%s\\VC\\vcpackages'):format( visual_studio_directory ),
            };
            
            lib_i386 = {
                ('%s\\VC\\atlmfc\\lib'):format( visual_studio_directory ),
                ('%s\\VC\\lib'):format( visual_studio_directory )
            };

            lib_x86_64 = {
                ('%s\\VC\\atlmfc\\lib\\amd64'):format( visual_studio_directory ),
                ('%s\\VC\\lib\\amd64'):format( visual_studio_directory )
            };

            include = {
                ('%s\\VC\\atlmfc\\include'):format( visual_studio_directory ),
                ('%s\\VC\\include'):format( visual_studio_directory ),
            };
        end
        
        local sdk_directory = settings.msvc.windows_sdk_directory;
        local sdk_version = settings.msvc.windows_sdk_version;
        if sdk_directory then 
            table.insert( include, ('%s\\Include\\%s\\ucrt'):format(sdk_directory, sdk_version) );
            table.insert( include, ('%s\\Include\\%s\\um'):format(sdk_directory, sdk_version) );
            table.insert( include, ('%s\\Include\\%s\\shared'):format(sdk_directory, sdk_version) );
            table.insert( include, ('%s\\Include\\%s\\winrt'):format(sdk_directory, sdk_version) );
            table.insert( lib_i386, ('%s\\Lib\\%s\\um\\x86'):format(sdk_directory, sdk_version) );
            table.insert( lib_i386, ('%s\\Lib\\%s\\ucrt\\x86'):format(sdk_directory, sdk_version) );
            table.insert( lib_x86_64, ('%s\\Lib\\%s\\um\\x64'):format(sdk_directory, sdk_version) );
            table.insert( lib_x86_64, ('%s\\Lib\\%s\\ucrt\\x64'):format(sdk_directory, sdk_version) );
        end

        msvc.environments_by_architecture = {
            ["i386"] = {
                PATH = table.concat( path_i386, ";" );
                LIB = table.concat( lib_i386, ";" );
                LIBPATH = table.concat( lib_i386, ";" );
                INCLUDE = table.concat( include, ";" );
                SYSTEMROOT = os.getenv( "SYSTEMROOT" );
                TMP = os.getenv( "TMP" );
            };
            ["x86_64"] = {
                PATH = table.concat( path_x86_64, ";" );
                LIB = table.concat( lib_x86_64, ";" );
                LIBPATH = table.concat( lib_x86_64, ";" );
                INCLUDE = table.concat( include, ";" );
                SYSTEMROOT = os.getenv( "SYSTEMROOT" );
                TMP = os.getenv( "TMP" );
            };
        };
    end
end;

function msvc.visual_cxx_tool( target, tool )
    local msvc = target.settings.msvc;
    if msvc.toolset_version >= 15 then 
        if target.architecture == 'x86_64' then
            return ('%s\\bin\\Hostx64\\x64\\%s'):format( msvc.visual_cxx_directory, tool );
        else
            return ('%s\\bin\\Hostx64\\x86\\%s'):format( msvc.visual_cxx_directory, tool );
        end
    else
        if target.architecture == 'x86_64' then
            return ('%s/VC/bin/amd64/%s'):format( target.settings.msvc.visual_studio_directory, tool );
        else
            return ('%s/VC/bin/%s'):format( target.settings.msvc.visual_studio_directory, tool );
        end
    end
end

function msvc.windows_sdk_tool( target, tool )
    local settings = target.settings;
    local directory = settings.msvc.windows_sdk_directory;
    local version = settings.msvc.windows_sdk_version;
    if target.architecture == "x86_64" then
        return ("%s\\bin\\%s\\x64\\%s"):format( directory, version, tool );
    else
        return ("%s\\bin\\%s\\x86\\%s"):format( directory, version, tool );
    end
end

function msvc.append_defines( target, flags )
    table.insert( flags, ('/DBUILD_PLATFORM_%s'):format(build:upper(platform)) );
    table.insert( flags, ('/DBUILD_VARIANT_%s'):format(build:upper(variant)) );
    table.insert( flags, ('/DBUILD_LIBRARY_SUFFIX="\\"_%s.lib\\""'):format(target.architecture) );
    table.insert( flags, ('/DBUILD_LIBRARY_TYPE_%s'):format(build:upper(target.settings.library_type)) );
    table.insert( flags, '/D_CRT_SECURE_NO_WARNINGS' );

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
            table.insert( flags, ('/I "%s"'):format(build:relative(directory)) );
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
    table.insert( flags, "/showIncludes" );

    local settings = target.settings;

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
        local pdb = ("%s%s.pdb"):format(settings.obj_directory(target), target:working_directory():id() );
        table.insert( flags, ("/Zi /Fd%s"):format(build:native(pdb)) );
    end

    if target.settings.link_time_code_generation then
        table.insert( flags, "/GL" );
    end

    if target.settings.minimal_rebuild then
        table.insert( flags, "/Gm" );
    end

    if target.settings.optimization then
        table.insert( flags, "/GF /O2 /Ot /Oi /Ox /Oy /GS- /favor:blend" );
    end

    if target.settings.preprocess then
        table.insert( flags, "/P /C" );
    end

    if target.settings.run_time_checks then
        table.insert( flags, "/RTC1" );
    end

    if target.settings.warnings_as_errors then 
        table.insert( flags, "/WX" );
    end

    local warning_level = target.settings.warning_level
    if warning_level == 0 then 
        table.insert( flags, "/w" );
    elseif warning_level == 1 then
        table.insert( flags, "/W1" );
    elseif warning_level == 2 then
        table.insert( flags, "/W2" );
    elseif warning_level == 3 then
        table.insert( flags, "/W3" );
    elseif warning_level >= 4 then
        table.insert( flags, "/W4" );
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
    local settings = target.settings;

    table.insert( flags, "/nologo" );

    local intermediate_manifest = ('%s/%s_intermediate.manifest'):format( settings.obj_directory(target), target:id() );
    table.insert( flags, "/manifest" );
    table.insert( flags, ("/manifestfile:%s"):format(intermediate_manifest) );
    
    if target.settings.subsystem then
        table.insert( flags, ("/subsystem:%s"):format(target.settings.subsystem) );
    end

    table.insert( flags, ("/out:%s"):format(build:native(target:filename())) );
    if target:prototype() == build.DynamicLibrary then
        table.insert( flags, "/dll" );
        table.insert( flags, ("/implib:%s"):format(build:native(("%s/%s.lib"):format(target.settings.lib, target:id()))) );
    end
    
    if target.settings.verbose_linking then
        table.insert( flags, "/verbose" );
    end
    
    if target.settings.debug then
        table.insert( flags, "/debug" );
        local pdb = ('%s/%s.pdb'):format( settings.obj_directory(target), target:id() );
        table.insert( flags, ("/pdb:%s"):format(build:native(pdb)) );
    end

    if target.settings.link_time_code_generation then
        table.insert( flags, "/ltcg" );
    end

    if target.settings.generate_map_file then
        local map = ('%s/%s.map'):format( settings.obj_directory(target), target:id() );
        table.insert( flags, ("/map:%s"):format(build:native(map)) );
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
            table.insert( flags, ("%s.lib"):format(build:basename(library)) );
        end
    end
    if target.third_party_libraries then
        for _, library in ipairs(target.settings.third_party_libraries) do
            table.insert( flags, ("%s.lib"):format(build:basename(library)) );
        end
    end
    if target.system_libraries then
        for _, library in ipairs(target.system_libraries) do
            table.insert( flags, ("%s.lib"):format(build:basename(library)) );
        end
    end
end

function msvc.dependencies_filter( output_directory, source_directory )
    local object;
    local current_directory = source_directory;
    local directories = { source_directory };

    -- Strip the backslash delimited prefix from _include_path_ and return the
    -- remaining portion.  This remaining portion is the correct relative path to
    -- a header.
    local function relative_include_path( include_path )
        local position = 1;
        local start, finish = include_path:find( "\\", position, false );
        while start do 
            position = finish + 1;
            start, finish = include_path:find( "\\", position, false );
        end
        return include_path:sub( position );
    end

    -- Match lines that indicate source files and header files in Microsoft
    -- Visual C++ output to gather dependencies for source file compilation.
    local function dependencies_filter( line )
        local SHOW_INCLUDES_PATTERN = "^Note: including file:(%s*)([^\n\r]*)[\n\r]*$";
        local indent, path = line:match( SHOW_INCLUDES_PATTERN );
        if indent and path then
            local indent = #indent;
            if indent < #directories then
                while indent < #directories do 
                    table.remove( directories );
                end
            end
            if indent > #directories then 
                table.insert( directories, current_directory );
            end

            local LOWER_CASE_DRIVE_PATTERN = "^%l:";
            local lower_case_path = path:find( LOWER_CASE_DRIVE_PATTERN );
            if lower_case_path then 
                local directory = directories[#directories];
                path = ("%s/%s"):format( directory, relative_include_path(path) );
            end

            local relative_path = build:relative( path, build:root() );
            local within_source_tree = build:is_relative( relative_path ) and relative_path:find( '..', 1, true ) == nil;
            if within_source_tree then
                local header = build:SourceFile( path );
                object:add_implicit_dependency( header );
            end
            current_directory = build:branch( path );
        else
            local SOURCE_FILE_PATTERN = "^[^%.]*%.?[^\n\r]*[\n\r]*$";
            local start, finish = line:find( SOURCE_FILE_PATTERN );
            if start and finish then 
                local obj_name = function( name ) return ("%s.obj"):format( build:basename(name) ); end;
                object = build:File( ("%s/%s"):format(output_directory, obj_name(line)) );
                object:clear_implicit_dependencies();
            end
            printf( '%s', line );
        end
    end
    return dependencies_filter;
end

build:register_module( msvc );
