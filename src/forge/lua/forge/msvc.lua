
local msvc = {};

function msvc.configure( toolset, msvc_settings )
    local function vswhere()
        local vswhere = 'C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe';
        if exists( vswhere ) then 
            local environment = {
                ProgramData = getenv( 'ProgramData' );
                SystemRoot = getenv( 'SystemRoot' );
            };
            local values = {};
            system( vswhere, 'vswhere -latest', environment, nil, function(line)
                local key, value = line:match( '([%w_]+): ([^\n\r]+)' );
                if key and value then 
                    values[key] = value;
                end
            end );
            if #values == 0 then 
                system( vswhere, 'vswhere -products Microsoft.VisualStudio.Product.BuildTools', environment, nil, function(line)
                    local key, value = line:match( '([%w_]+): ([^\n\r]+)' );
                    if key and value then 
                        values[key] = value;
                    end
                end );
            end
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
        system( reg, arguments, nil, nil, function(line)
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
            for filename in ls(('%s/bin'):format(windows_sdk.KitsRoot10)) do
                local version = leaf( filename ):match('10%.%d+%.%d+%.%d+');
                if version then
                    latest_version = version;
                end
            end
            return latest_version;
        end
    end

    local toolset_version = autodetect_toolset_version();
    return {
        toolset_version = toolset_version;
        visual_studio_directory = autodetect_visual_studio_directory( toolset_version );
        visual_cxx_directory = autodetect_visual_cxx_directory( toolset_version );
        windows_sdk_directory = autodetect_windows_sdk_directory();
        windows_sdk_version = autodetect_windows_sdk_version();
    };
end

function msvc.validate( toolset, msvc_settings )
    return 
        operating_system() == 'windows' and 
        msvc_settings.toolset_version ~= nil and 
        msvc_settings.visual_cxx_directory ~= nil
    ;
end

function msvc.initialize( toolset )
    if toolset:configure(msvc, 'msvc') then 
        local identifier = toolset.settings.identifier;
        if identifier then
            add_toolset( toolset:interpolate(identifier), toolset );
        end

        -- Make sure that the environment variable VS_UNICODE_OUTPUT is not set.
        --
        -- Visual Studio sets this to signal its tools to communicate back to 
        -- Visual Studio using named pipes rather than stdout so that unicode output 
        -- works better but this then prevents the build tool from intercepting
        -- and collating this output.
        --
        -- See http://blogs.msdn.com/freik/archive/2006/04/05/569025.aspx.
        -- putenv( "VS_UNICODE_OUTPUT", "" );

        local settings = toolset.settings;
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
            ['i386'] = {
                PATH = table.concat( path_i386, ';' );
                LIB = table.concat( lib_i386, ';' );
                LIBPATH = table.concat( lib_i386, ';' );
                INCLUDE = table.concat( include, ';' );
                SYSTEMROOT = os.getenv( 'SYSTEMROOT' );
                TMP = os.getenv( 'TMP' );
            };
            ['x86_64'] = {
                PATH = table.concat( path_x86_64, ';' );
                LIB = table.concat( lib_x86_64, ';' );
                LIBPATH = table.concat( lib_x86_64, ';' );
                INCLUDE = table.concat( include, ';' );
                SYSTEMROOT = os.getenv( 'SYSTEMROOT' );
                TMP = os.getenv( 'TMP' );
            };
        };

        local Cc = forge:TargetPrototype( 'Cc' );
        Cc.language = 'c';
        Cc.build = msvc.compile;
        toolset.Cc = forge:GroupElement( Cc, msvc.object_filename );

        local Cxx = forge:TargetPrototype( 'Cxx' );
        Cxx.language = 'c++';
        Cxx.build = msvc.compile;
        toolset.Cxx = forge:GroupElement( Cxx, msvc.object_filename );

        local StaticLibrary = forge:FilePrototype( 'StaticLibrary', msvc.static_library_filename );
        StaticLibrary.build = msvc.archive;
        toolset.StaticLibrary = StaticLibrary;

        local DynamicLibrary = forge:FilePrototype( 'DynamicLibrary', msvc.dynamic_library_filename );
        DynamicLibrary.build = msvc.link;
        toolset.DynamicLibrary = DynamicLibrary;

        local Executable = forge:FilePrototype( 'Executable', msvc.executable_filename );
        Executable.build = msvc.link;
        toolset.Executable = Executable;

        toolset:defaults( toolset.settings, {
            architecture = 'x86_64';
            assertions = true;
            debug = true;
            debuggable = true;
            exceptions = true;
            fast_floating_point = false;
            framework_directories = {};
            generate_dsym_bundle = false;
            generate_map_file = true;
            incremental_linking = true;
            link_time_code_generation = false;
            minimal_rebuild = true;
            objc_arc = true;
            objc_modules = true;
            optimization = false;
            pre_compiled_headers = true;
            preprocess = false;
            profiling = false;
            run_time_checks = true;
            runtime_library = 'static_debug';
            run_time_type_info = true;
            sse2 = true;
            stack_size = 1048576;
            standard = 'c++17';
            string_pooling = false;
            strip = false;
            subsystem = 'CONSOLE';
            verbose_linking = false;
            warning_level = 3;
            warnings_as_errors = true;
        } );

        return toolset;
    end
end

function msvc.object_filename( toolset, identifier )
    return ('%s.obj'):format( identifier );
end

function msvc.static_library_filename( toolset, identifier )
    local identifier = absolute( toolset:interpolate(identifier) );
    local filename = ('%s.lib'):format( identifier );
    return identifier, filename;
end

function msvc.dynamic_library_filename( toolset, identifier )
    local identifier = absolute( toolset:interpolate(identifier) );
    local filename = ('%s.dll'):format( identifier );
    return identifier, filename;
end

function msvc.executable_filename( toolset, identifier )
    local identifier = toolset:interpolate( identifier );
    local filename = ('%s.exe'):format( identifier );
    return identifier, filename;
end

-- Compile C and C++.
function msvc.compile( toolset, target ) 
    local settings = toolset.settings;    

    local flags = {};
    msvc.append_defines( toolset, target, flags );
    msvc.append_include_directories( toolset, target, flags );
    msvc.append_compile_flags( toolset, target, flags );

    local objects_by_source = {};
    local sources_by_directory = {};
    for _, object in target:dependencies() do
        if object:outdated() then
            local source = object:dependency();
            local directory = branch( source:filename() );
            local sources = sources_by_directory[directory];
            if not sources then 
                sources = {};
                sources_by_directory[directory] = sources;
            end
            local source = object:dependency();
            table.insert( sources, source:id() );
            objects_by_source[leaf(source:id())] = object;
            object:clear_implicit_dependencies();
        end    
    end

    for directory, sources in pairs(sources_by_directory) do
        if #sources > 0 then
            -- Make sure that the output directory has a trailing slash so
            -- that Visual C++ doesn't interpret it as a file when a single
            -- source file is compiled.
            local output_directory = native( ('%s/%s'):format(toolset:obj_directory(target), relative(directory)) );
            if output_directory:sub(-1) ~= '\\' then 
                output_directory = ('%s\\'):format( output_directory );
            end

            local ccflags = table.concat( flags, ' ' );
            local source = table.concat( sources, '" "' );
            local cl = msvc.visual_cxx_tool( toolset, 'cl.exe' );
            local environment = msvc.environments_by_architecture[settings.architecture];
            pushd( directory );
            system( 
                cl, 
                ('cl %s /Fo%s "%s"'):format(ccflags, output_directory, source), 
                environment, 
                nil,
                msvc.dependencies_filter(toolset, output_directory, absolute(directory))
            );
            popd();
        end
    end

    for _, object in pairs(objects_by_source) do
        object:set_built( true );
    end
end

-- Archive objects into a static library. 
function msvc.archive( toolset, target ) 
    printf( leaf(target) );

    local settings = toolset.settings;

    local flags = {
        '/nologo'
    };
   
    if settings.link_time_code_generation then
        table.insert( flags, '/ltcg' );
    end
    
    pushd( toolset:obj_directory(target) );
    local objects = {};
    for _, dependency in target:dependencies() do
        local prototype = dependency:prototype();
        if prototype ~= toolset.Directory then
            for _, object in dependency:dependencies() do
                table.insert( objects, relative(object:filename()) );
            end
        end
    end
    
    if #objects > 0 then
        local arflags = table.concat( flags, ' ' );
        local arobjects = table.concat( objects, '" "' );
        local msar = msvc.visual_cxx_tool( toolset, 'lib.exe' );
        local environment = msvc.environments_by_architecture[settings.architecture];
        system( msar, ('lib %s /out:"%s" "%s"'):format(arflags, native(target:filename()), arobjects), environment );
    end
    popd();
end

-- Link dynamic libraries and executables.
function msvc.link( toolset, target ) 
    printf( leaf(target) );

    local objects = {};
    local libraries = {};
    local settings = toolset.settings;
    pushd( toolset:obj_directory(target) );
    for _, dependency in target:dependencies() do
        local prototype = dependency:prototype();
        if prototype == toolset.StaticLibrary or prototype == toolset.DynamicLibrary then
            table.insert( libraries, ('%s.lib'):format(basename(dependency:filename())) );
        elseif prototype ~= toolset.Directory then
            assertf( target.architecture == dependency.architecture, "Architectures for '%s' (%s) and '%s' (%s) don't match", target:path(), tostring(target.architecture), dependency:path(), tostring(dependency.architecture) );
            for _, object in dependency:dependencies() do
                if object:prototype() == nil then
                    table.insert( objects, relative(object:filename()) );
                end
            end
        end
    end

    local flags = {};
    msvc.append_link_flags( toolset, target, flags );
    msvc.append_library_directories( toolset, target, flags );
    msvc.append_link_libraries( toolset, target, libraries );

    if #objects > 0 then
        local msld = msvc.visual_cxx_tool( toolset, "link.exe" );
        local msmt = msvc.windows_sdk_tool( toolset, "mt.exe" );
        local msrc = msvc.windows_sdk_tool( toolset, "rc.exe" );
        local intermediate_manifest = ('%s/%s_intermediate.manifest'):format( toolset:obj_directory(target), target:id() );

        if settings.incremental_linking then
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

            local ignore_filter = function() end;
            local ldflags = table.concat( flags, ' ' );
            local ldlibs = table.concat( libraries, ' ' );
            local ldobjects = table.concat( objects, '" "' );
            local environment = msvc.environments_by_architecture[settings.architecture];

            if exists(embedded_manifest) ~= true then
                system( msld, ('link %s "%s" %s'):format(ldflags, ldobjects, ldlibs), environment );
                system( msmt, ('mt /nologo /out:"%s" /manifest "%s"'):format(embedded_manifest, intermediate_manifest), environment );
                system( msrc, ('rc /Fo"%s" "%s"'):format(embedded_manifest_res, embedded_manifest_rc), environment, nil, ignore_filter );
            end

            table.insert( objects, embedded_manifest_res );
            table.insert( flags, "/incremental" );
            local ldflags = table.concat( flags, ' ' );
            local ldobjects = table.concat( objects, '" "' );

            system( msld, ('link %s "%s" %s'):format(ldflags, ldobjects, ldlibs), environment );
            system( msmt, ('mt /nologo /out:"%s" /manifest %s'):format(embedded_manifest, intermediate_manifest), environment );
            system( msrc, ('rc /Fo"%s" %s'):format(embedded_manifest_res, embedded_manifest_rc), environment, nil, ignore_filter );
            system( msld, ('link %s "%s" %s'):format(ldflags, ldobjects, ldlibs), environment );
        else
            table.insert( flags, "/incremental:no" );

            local ldflags = table.concat( flags, ' ' );
            local ldlibs = table.concat( libraries, ' ' );
            local ldobjects = table.concat( objects, '" "' );
            local environment = msvc.environments_by_architecture[settings.architecture];

            system( msld, ('link %s "%s" %s'):format(ldflags, ldobjects, ldlibs), environment );
            sleep( 100 );
            system( msmt, ('mt /nologo -outputresource:"%s";#1 -manifest %s'):format(native(target:filename()), intermediate_manifest), environment );
        end
    end
    popd();
end

function msvc.visual_cxx_tool( toolset, tool )
    local settings = toolset.settings;
    local msvc = settings.msvc;
    if msvc.toolset_version >= 15 then 
        if settings.architecture == 'x86_64' then
            return ('%s\\bin\\Hostx64\\x64\\%s'):format( msvc.visual_cxx_directory, tool );
        else
            return ('%s\\bin\\Hostx64\\x86\\%s'):format( msvc.visual_cxx_directory, tool );
        end
    else
        if settings.architecture == 'x86_64' then
            return ('%s/VC/bin/amd64/%s'):format( msvc.visual_studio_directory, tool );
        else
            return ('%s/VC/bin/%s'):format( msvc.visual_studio_directory, tool );
        end
    end
end

function msvc.windows_sdk_tool( toolset, tool )
    local settings = toolset.settings;
    local directory = settings.msvc.windows_sdk_directory;
    local version = settings.msvc.windows_sdk_version;
    if settings.architecture == "x86_64" then
        return ("%s\\bin\\%s\\x64\\%s"):format( directory, version, tool );
    else
        return ("%s\\bin\\%s\\x86\\%s"):format( directory, version, tool );
    end
end

function msvc.append_defines( toolset, target, flags )
    local settings = toolset.settings;

    if not settings.assertions then
        table.insert( flags, '/DNDEBUG' );
    end

    if settings.defines then
        for _, define in ipairs(settings.defines) do
            table.insert( flags, ('/D%s'):format(define) );
        end
    end

    if target.defines then
        for _, define in ipairs(target.defines) do
            table.insert( flags, ('/D%s'):format(define) );
        end
    end
end

function msvc.append_include_directories( toolset, target, flags )
    local settings = toolset.settings;

    if target.include_directories then
        for _, directory in ipairs(target.include_directories) do
            table.insert( flags, ('/I "%s"'):format(relative(directory)) );
        end
    end

    if settings.include_directories then
        for _, directory in ipairs(settings.include_directories) do
            table.insert( flags, ('/I "%s"'):format(directory) );
        end
    end
end

function msvc.append_compile_flags( toolset, target, flags )
    local settings = toolset.settings;

    table.insert( flags, '/nologo' );
    table.insert( flags, '/FS' );
    table.insert( flags, '/FC' );
    table.insert( flags, '/c' );
    table.insert( flags, '/showIncludes' );

    local language = target.language or 'c++';
    if language == 'c' then 
        table.insert( flags, '/TC' );
    elseif language == 'c++' then
        table.insert( flags, '/TP' );
        if settings.exceptions then
            table.insert( flags, '/EHsc' );
        end
        if settings.run_time_type_info then
            table.insert( flags, '/GR' );
        end
    else
        assert( false, 'Only C and C++ are supported by Microsoft Visual C++' );
    end

    local standard = settings.standard;
    if standard then 
        table.insert( flags, ('/std:%s'):format(standard) );
    end

    if settings.runtime_library == 'static' then
        table.insert( flags, '/MT' );
    elseif settings.runtime_library == 'static_debug' then
        table.insert( flags, '/MTd' );
    elseif settings.runtime_library == 'dynamic' then
        table.insert( flags, '/MD' );
    elseif settings.runtime_library == 'dynamic_debug' then
        table.insert( flags, '/MDd' );
    end
    
    if settings.debug then
        local pdb = ('%s/%s.pdb'):format(toolset:obj_directory(target), target:working_directory():id() );
        table.insert( flags, ('/Zi /Fd%s'):format(native(pdb)) );
    end

    if settings.link_time_code_generation then
        table.insert( flags, '/GL' );
    end

    if settings.optimization then
        table.insert( flags, '/GF /O2 /Ot /Oi /Ox /Oy /GS- /favor:blend' );
    end

    if settings.preprocess then
        table.insert( flags, '/P /C' );
    end

    if settings.run_time_checks then
        table.insert( flags, '/RTC1' );
    end

    if settings.warnings_as_errors then 
        table.insert( flags, '/WX' );
    end

    local warning_level = settings.warning_level
    if warning_level == 0 then 
        table.insert( flags, '/w' );
    elseif warning_level == 1 then
        table.insert( flags, '/W1' );
    elseif warning_level == 2 then
        table.insert( flags, '/W2' );
    elseif warning_level == 3 then
        table.insert( flags, '/W3' );
    elseif warning_level >= 4 then
        table.insert( flags, '/W4' );
    end
end

function msvc.append_library_directories( toolset, target, flags )
    local settings = toolset.settings;

    if target.library_directories then
        for _, directory in ipairs(target.library_directories) do
            table.insert( flags, ('/libpath:"%s"'):format(directory) );
        end
    end    

    if settings.library_directories then
        for _, directory in ipairs(settings.library_directories) do
            table.insert( flags, ('/libpath:"%s"'):format(directory) );
        end
    end    
end

function msvc.append_link_flags( toolset, target, flags )
    local settings = toolset.settings;

    table.insert( flags, "/nologo" );

    local intermediate_manifest = ('%s/%s_intermediate.manifest'):format( toolset:obj_directory(target), target:id() );
    table.insert( flags, '/manifest' );
    table.insert( flags, ('/manifestfile:%s'):format(intermediate_manifest) );
    
    if settings.subsystem then
        table.insert( flags, ('/subsystem:%s'):format(settings.subsystem) );
    end

    table.insert( flags, ('/out:%s'):format(native(target:filename())) );
    if target:prototype() == toolset.DynamicLibrary then
        table.insert( flags, '/dll' );
        table.insert( flags, ('/implib:%s'):format(native(('%s/%s.lib'):format(settings.lib, target:id()))) );
    end
    
    if settings.verbose_linking then
        table.insert( flags, '/verbose' );
    end
    
    if settings.debug then
        table.insert( flags, '/debug' );
        local pdb = ('%s/%s.pdb'):format( toolset:obj_directory(target), target:id() );
        table.insert( flags, ('/pdb:%s'):format(native(pdb)) );
    end

    if settings.link_time_code_generation then
        table.insert( flags, '/ltcg' );
    end

    if settings.generate_map_file then
        local map = ('%s/%s.map'):format( toolset:obj_directory(target), target:id() );
        table.insert( flags, ('/map:%s'):format(native(map)) );
    end

    if settings.optimization then
        table.insert( flags, '/opt:ref' );
        table.insert( flags, '/opt:icf' );
    end

    if settings.stack_size then
        table.insert( flags, ('/stack:%d'):format(settings.stack_size) );
    end
end

function msvc.append_link_libraries( toolset, target, flags )
    local settings = toolset.settings;

    if settings.libraries then
        for _, library in ipairs(settings.libraries) do
            table.insert( flags, ('%s.lib'):format(basename(library)) );
        end
    end

    if target.libraries then
        for _, library in ipairs(target.libraries) do
            table.insert( flags, ('%s.lib'):format(basename(library)) );
        end
    end
end

function msvc.dependencies_filter( toolset, output_directory, source_directory )
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

            local relative_path = relative( path, root() );
            local within_source_tree = is_relative( relative_path ) and relative_path:find( '..', 1, true ) == nil;
            if within_source_tree then
                local header = toolset:SourceFile( path );
                object:add_implicit_dependency( header );
            end
            current_directory = branch( path );
        else
            local SOURCE_FILE_PATTERN = "^[^%.]*%.?[^\n\r]*[\n\r]*$";
            local start, finish = line:find( SOURCE_FILE_PATTERN );
            if start and finish then 
                local obj_name = function( name ) return ("%s.obj"):format( basename(name) ); end;
                object = toolset:File( ("%s/%s"):format(output_directory, obj_name(line)) );
                object:clear_implicit_dependencies();
            end
            printf( '%s', line );
        end
    end
    return dependencies_filter;
end

setmetatable( msvc, {
    __call = function( msvc, settings )
        local toolset = require( 'forge' ):clone( settings );
        return msvc.initialize( toolset );
    end
} );

return msvc;
