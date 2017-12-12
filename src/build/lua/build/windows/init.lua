
windows = {};

function windows.initialize( settings )
    if build.platform_matches("windows") then
        cc = windows.cc;
        build_library = windows.build_library;
        clean_library = windows.clean_library;
        build_executable = windows.build_executable;
        clean_executable = windows.clean_executable;
        lipo_executable = windows.lipo_executable;
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
        module_name = windows.module_name;
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
    
    local sources_by_directory = {};
    for dependency in target:dependencies() do
        if dependency:outdated() then
            local directory = branch( dependency.source );
            local sources = sources_by_directory[directory];
            if not sources then 
                sources = {};
                sources_by_directory[directory] = sources;
            end
            table.insert( sources, dependency.source );
            dependency:clear_implicit_dependencies();
        end    
    end

    for directory, sources in pairs(sources_by_directory) do
        if #sources > 0 then
            local output_directory = native( ("%s%s/%s/"):format(obj_directory(target), target.architecture, directory) );
            local ccflags = table.concat( flags, " " );
            local source = table.concat( sources, '" "' );
            local cl = ("%s/VC/bin/cl.exe"):format( target.settings.msvc.visual_studio_directory );
            local environment = msvc.environment;
            build.system( cl, ('cl %s /Fo%s "%s"'):format(ccflags, output_directory, source), environment, msvc.process_dependencies_filter(output_directory, absolute(directory)) );
        end
    end
end;

function windows.build_library( target )
    local flags = {
        "/nologo"
    };
    
    if target.settings.link_time_code_generation then
        table.insert( flags, "/ltcg" );
    end
    
    pushd( ("%s%s"):format(obj_directory(target), target.architecture) );
    local objects = {};
    for dependency in target:dependencies() do
        local prototype = dependency:prototype();
        if prototype == build.Cc or prototype == build.Cxx then
            for object in dependency:dependencies() do
                table.insert( objects, relative(object:filename()) );
            end
        end
    end
    
    if #objects > 0 then
        local arflags = table.concat( flags, " " );
        local arobjects = table.concat( objects, '" "' );
        local msar = ("%s/VC/bin/lib.exe"):format( target.settings.msvc.visual_studio_directory );
        local environment = msvc.environment;
        print( leaf(target:filename()) );
        build.system( msar, ('lib %s /out:"%s" "%s"'):format(arflags, native(target:filename()), arobjects), environment );
    end
    popd();
end;

function windows.clean_library( target )
    rm( target:filename() );
    rmdir( obj_directory(target) );
end;

function windows.build_executable( target )
    local flags = {};
    msvc.append_link_flags( target, flags );
    msvc.append_library_directories( target, flags );

    local objects = {};
    local libraries = {};

    for dependency in target:dependencies() do
        local prototype = dependency:prototype();
        if prototype == build.Cc or prototype == build.Cxx then
            for object in dependency:dependencies() do
                table.insert( objects, obj_name(object:id()) );
            end
        elseif prototype == build.StaticLibrary or prototype == build.DynamicLibrary then
            table.insert( libraries, ('%s.lib'):format(basename(dependency:filename())) );
        end
    end

    msvc.append_link_libraries( target, libraries );

    if #objects > 0 then
        local msld = ("%s/VC/bin/link.exe"):format( target.settings.msvc.visual_studio_directory );
        local msmt = ("%s/bin/x86/mt.exe"):format( target.settings.msvc.windows_sdk_directory );
        local msrc = ("%s/bin/x86/rc.exe"):format( target.settings.msvc.windows_sdk_directory );
        local intermediate_manifest = ('%s%s_intermediate.manifest'):format( obj_directory(target), target:id() );

        print( leaf(target:filename()) );
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

            local ignore_filter = function() end;
            local ldflags = table.concat( flags, ' ' );
            local ldlibs = table.concat( libraries, ' ' );
            local ldobjects = table.concat( objects, '" "' );
            local environment = msvc.environment;

            if exists(embedded_manifest) ~= true then
                build.system( msld, ('link %s "%s" %s'):format(ldflags, ldobjects, ldlibs), environment );
                build.system( msmt, ('mt /nologo /out:"%s" /manifest "%s"'):format(embedded_manifest, intermediate_manifest), environment );
                build.system( msrc, ('rc /Fo"%s" "%s"'):format(embedded_manifest_res, embedded_manifest_rc), environment, ignore_filter );
            end

            table.insert( objects, embedded_manifest_res );
            table.insert( flags, "/incremental" );
            local ldflags = table.concat( flags, ' ' );
            local ldobjects = table.concat( objects, '" "' );

            build.system( msld, ('link %s "%s" %s'):format(ldflags, ldobjects, ldlibs), environment );
            build.system( msmt, ('mt /nologo /out:"%s" /manifest %s'):format(embedded_manifest, intermediate_manifest), environment );
            build.system( msrc, ('rc /Fo"%s" %s'):format(embedded_manifest_res, embedded_manifest_rc), environment, ignore_filter );
            build.system( msld, ('link %s "%s" %s'):format(ldflags, ldobjects, ldlibs), environment );
        else
            table.insert( flags, "/incremental:no" );

            local ldflags = table.concat( flags, ' ' );
            local ldlibs = table.concat( libraries, ' ' );
            local ldobjects = table.concat( objects, '" "' );
            local environment = msvc.environment;

            build.system( msld, ('link %s "%s" %s'):format(ldflags, ldobjects, ldlibs), environment );
            sleep( 100 );
            build.system( msmt, ('mt /nologo -outputresource:"%s";#1 -manifest %s'):format(native(target:filename()), intermediate_manifest), environment );
        end
        popd();
    end
end;

function windows.clean_executable( target )
    local filename = target:filename();
    rm( filename );
    rm( ("%s/%s.ilk"):format(branch(filename), basename(filename)) );
    rmdir( obj_directory(target) );
end

function windows.lipo_executable( target )
end

function windows.obj_directory( target )
    return ("%s/%s/"):format( target.settings.obj, relative(target:working_directory():path(), root()) );
end

function windows.cc_name( name )
    return ("%s.c"):format( basename(name) );
end

function windows.cxx_name( name )
    return ("%s.cpp"):format( basename(name) );
end

function windows.pch_name( name )
    return ("%s.pch"):format( basename(name) );
end

function windows.pdb_name( name )
    return ("%s.pdb"):format( basename(name) );
end

function windows.obj_name( name )
    return ("%s.obj"):format( basename(name) );
end

function windows.lib_name( name, architecture )
    return ("%s_%s.lib"):format( name, architecture );
end

function windows.exp_name( name, architecture )
    return ("%s_%s.exp"):format( name, architecture );
end

function windows.dll_name( name, architecture )
    return ("%s_%s.dll"):format( name, architecture );
end

function windows.exe_name( name, architecture )
    return ("%s_%s.exe"):format( name, architecture );
end

function windows.ilk_name( name, architecture )
    return ("%s_%s.ilk"):format( name, architecture );
end

function windows.module_name( name, architecture )
    return name;
end

build.register_module( windows );
