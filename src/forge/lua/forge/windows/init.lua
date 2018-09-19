
windows = {};

function windows.initialize( settings )
    if forge:operating_system() == "windows" then
        for _, architecture in ipairs(settings.architectures) do 
            forge:add_default_build( ("cc_windows_%s"):format(architecture), forge:configure {
                obj = ("%s/cc_windows_%s"):format( settings.obj, architecture );
                obj_extension = '.obj';
                architecture = architecture;
                default_architecture = architecture;
                cc = windows.cc;
                build_library = windows.build_library;
                clean_library = windows.clean_library;
                build_executable = windows.build_executable;
                clean_executable = windows.clean_executable;
                obj_directory = windows.obj_directory;
                cc_name = windows.cc_name;
                cxx_name = windows.cxx_name;
                pch_name = windows.pch_name;
                pdb_name = windows.pdb_name;
                obj_name = windows.obj_name;
                lib_name = windows.lib_name;
                exp_name = windows.exp_name;
                dll_name = windows.dll_name;
                exe_name = windows.exe_name;
                ilk_name = windows.ilk_name;
            } );
        end

        local settings = forge.settings;
        local architecture = settings.default_architecture;
        settings.obj = forge:root( ('%s/cc_windows_%s'):format(settings.obj, architecture) );
        settings.obj_extension = '.obj';
        settings.platform = "windows";
        settings.architecture = architecture;
        settings.default_architecture = architecture;
        settings.cc = windows.cc;
        settings.build_library = windows.build_library;
        settings.clean_library = windows.clean_library;
        settings.build_executable = windows.build_executable;
        settings.clean_executable = windows.clean_executable;
        settings.obj_directory = windows.obj_directory;
        settings.cc_name = windows.cc_name;
        settings.cxx_name = windows.cxx_name;
        settings.pch_name = windows.pch_name;
        settings.pdb_name = windows.pdb_name;
        settings.obj_name = windows.obj_name;
        settings.lib_name = windows.lib_name;
        settings.exp_name = windows.exp_name;
        settings.dll_name = windows.dll_name;
        settings.exe_name = windows.exe_name;
        settings.ilk_name = windows.ilk_name;
    end
end

function windows.cc( target )
    local flags = {
        '/DBUILD_OS_WINDOWS';
    };
    msvc.append_defines( target, flags );
    msvc.append_version_defines( target, flags );
    msvc.append_include_directories( target, flags );
    msvc.append_compile_flags( target, flags );
    
    local objects_by_source = {};
    local sources_by_directory = {};
    for _, object in target:dependencies() do
        if object:outdated() then
            local source = object:dependency();
            local directory = forge:branch( source:filename() );
            local sources = sources_by_directory[directory];
            if not sources then 
                sources = {};
                sources_by_directory[directory] = sources;
            end
            local source = object:dependency();
            table.insert( sources, source:id() );
            objects_by_source[forge:leaf(source:id())] = object;
            object:clear_implicit_dependencies();
        end    
    end

    local settings = target.settings;

    for directory, sources in pairs(sources_by_directory) do
        if #sources > 0 then
            local settings = target.settings;
            local output_directory = forge:native( ("%s/%s"):format(settings.obj_directory(target), forge:relative(directory)) );

            -- Make sure that the output directory has a trailing slash so
            -- that Visual C++ doesn't interpret the output directory as a 
            -- file as seems to happen when a single source file is compiled.
            if output_directory:sub(-1) ~= '\\' then 
                output_directory = ('%s\\'):format( output_directory );
            end

            local ccflags = table.concat( flags, " " );
            local source = table.concat( sources, '" "' );
            local cl = msvc.visual_cxx_tool( target, "cl.exe" );
            local environment = msvc.environments_by_architecture[target.architecture];
            forge:pushd( directory );
            forge:system( 
                cl, 
                ('cl %s /Fo%s "%s"'):format(ccflags, output_directory, source), 
                environment, 
                nil,
                msvc.dependencies_filter(output_directory, forge:absolute(directory))
            );
            forge:popd();
        end
    end

    for _, object in pairs(objects_by_source) do
        object:set_built( true );
    end
end;

function windows.build_library( target )
    local flags = {
        "/nologo"
    };
    
    if target.settings.link_time_code_generation then
        table.insert( flags, "/ltcg" );
    end
    
    local settings = target.settings;
    forge:pushd( settings.obj_directory(target) );

    local objects = {};
    for _, dependency in target:dependencies() do
        local prototype = dependency:prototype();
        if prototype == forge.Cc or prototype == forge.Cxx then
            for _, object in dependency:dependencies() do
                table.insert( objects, forge:relative(object:filename()) );
            end
        end
    end
    
    if #objects > 0 then
        local arflags = table.concat( flags, " " );
        local arobjects = table.concat( objects, '" "' );
        local msar = msvc.visual_cxx_tool( target, "lib.exe" );
        local environment = msvc.environments_by_architecture[target.architecture];
        forge:system( msar, ('lib %s /out:"%s" "%s"'):format(arflags, forge:native(target:filename()), arobjects), environment );
    end
    forge:popd();
end;

function windows.clean_library( target )
    forge:rm( target:filename() );
    forge:rmdir( obj_directory(target) );
end

function windows.build_executable( target )
    local flags = {};
    msvc.append_link_flags( target, flags );
    msvc.append_library_directories( target, flags );

    local objects = {};
    local libraries = {};

    local settings = target.settings;
    forge:pushd( settings.obj_directory(target) );

    for _, dependency in target:dependencies() do
        local prototype = dependency:prototype();
        if prototype == forge.Cc or prototype == forge.Cxx then
            assertf( target.architecture == dependency.architecture, "Architectures for '%s' (%s) and '%s' (%s) don't match", target:path(), tostring(target.architecture), dependency:path(), tostring(dependency.architecture) );
            for _, object in dependency:dependencies() do
                if object:prototype() == nil then
                    table.insert( objects, forge:relative(object:filename()) );
                end
            end
        elseif prototype == forge.StaticLibrary or prototype == forge.DynamicLibrary then
            table.insert( libraries, ('%s.lib'):format(forge:basename(dependency:filename())) );
        end
    end

    msvc.append_link_libraries( target, libraries );

    if #objects > 0 then
        local msld = msvc.visual_cxx_tool( target, "link.exe" );
        local msmt = msvc.windows_sdk_tool( target, "mt.exe" );
        local msrc = msvc.windows_sdk_tool( target, "rc.exe" );
        local intermediate_manifest = ('%s/%s_intermediate.manifest'):format( settings.obj_directory(target), target:id() );

        if target.settings.incremental_linking then
            local embedded_manifest = ("%s_embedded.manifest"):format( target:id() );
            local embedded_manifest_rc = ("%s_embedded_manifest.rc"):format( target:id() );
            local embedded_manifest_res = ("%s_embedded_manifest.res"):format( target:id() );

            if not forge:exists(embedded_manifest_rc) then        
                local rc = io.open( forge:absolute(embedded_manifest_rc), "wb" );
                assertf( rc, "Opening '%s' to write manifest failed", forge:absolute(embedded_manifest_rc) );
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
            local environment = msvc.environments_by_architecture[target.architecture];

            if forge:exists(embedded_manifest) ~= true then
                forge:system( msld, ('link %s "%s" %s'):format(ldflags, ldobjects, ldlibs), environment );
                forge:system( msmt, ('mt /nologo /out:"%s" /manifest "%s"'):format(embedded_manifest, intermediate_manifest), environment );
                forge:system( msrc, ('rc /Fo"%s" "%s"'):format(embedded_manifest_res, embedded_manifest_rc), environment, nil, ignore_filter );
            end

            table.insert( objects, embedded_manifest_res );
            table.insert( flags, "/incremental" );
            local ldflags = table.concat( flags, ' ' );
            local ldobjects = table.concat( objects, '" "' );

            forge:system( msld, ('link %s "%s" %s'):format(ldflags, ldobjects, ldlibs), environment );
            forge:system( msmt, ('mt /nologo /out:"%s" /manifest %s'):format(embedded_manifest, intermediate_manifest), environment );
            forge:system( msrc, ('rc /Fo"%s" %s'):format(embedded_manifest_res, embedded_manifest_rc), environment, nil, ignore_filter );
            forge:system( msld, ('link %s "%s" %s'):format(ldflags, ldobjects, ldlibs), environment );
        else
            table.insert( flags, "/incremental:no" );

            local ldflags = table.concat( flags, ' ' );
            local ldlibs = table.concat( libraries, ' ' );
            local ldobjects = table.concat( objects, '" "' );
            local environment = msvc.environments_by_architecture[target.architecture];

            forge:system( msld, ('link %s "%s" %s'):format(ldflags, ldobjects, ldlibs), environment );
            forge:sleep( 100 );
            forge:system( msmt, ('mt /nologo -outputresource:"%s";#1 -manifest %s'):format(forge:native(target:filename()), intermediate_manifest), environment );
        end
    end
    forge:popd();
end

function windows.clean_executable( target )
    local filename = target:filename();
    forge:rm( filename );
    forge:rm( ("%s/%s.ilk"):format(forge:branch(filename), forge:basename(filename)) );
    forge:rmdir( obj_directory(target) );
end

function windows.lipo_executable( target )
end

function windows.obj_directory( target )
    local relative_path = forge:relative( target:working_directory():path(), forge:root() );
    return forge:absolute( relative_path, target.settings.obj );
end

function windows.cc_name( name )
    return ("%s.c"):format( forge:basename(name) );
end

function windows.cxx_name( name )
    return ("%s.cpp"):format( forge:basename(name) );
end

function windows.pch_name( name )
    return ("%s.pch"):format( forge:basename(name) );
end

function windows.pdb_name( name )
    return ("%s.pdb"):format( forge:basename(name) );
end

function windows.obj_name( name )
    return ("%s.obj"):format( name );
end

function windows.lib_name( name )
    return ("%s.lib"):format( name, architecture );
end

function windows.exp_name( name )
    return ("%s.exp"):format( name );
end

function windows.dll_name( name )
    return ("%s.dll"):format( name );
end

function windows.exe_name( name )
    return ("%s.exe"):format( name );
end

function windows.ilk_name( name )
    return ("%s.ilk"):format( name );
end

forge:register_module( windows );
