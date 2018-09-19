
macos = {};

function macos.configure( settings )
    local function autodetect_macos_sdk_version()
        local sdk_version = "";
        local sdk_build_version = "";

        local xcodebuild = "/usr/bin/xcodebuild";
        local arguments = "xcodebuild -sdk macosx -version";
        local result = forge:execute( xcodebuild, arguments, nil, nil, function(line)
            local key, value = line:match( "(%w+): ([^\n]+)" );
            if key and value then 
                if key == "ProductBuildVersion" then 
                    sdk_build_version = value;
                elseif key == "SDKVersion" then
                    sdk_version = value;
                end
            end
        end );
        assert( result == 0, "Running xcodebuild to extract SDK name and version failed" );

        return sdk_version, sdk_build_version;
    end

    local function autodetect_xcode_version()
        local xcode_version = "";
        local xcode_build_version = "";

        local xcodebuild = "/usr/bin/xcodebuild";
        local arguments = "xcodebuild -version";
        local result = forge:execute( xcodebuild, arguments, nil, nil, function(line)
            local major, minor = line:match( "Xcode (%d+)%.(%d+)" );
            if major and minor then 
                xcode_version = ("%02d%02d"):format( tonumber(major), tonumber(minor) );
            end

            local build_version = line:match( "Build version (%w+)" )
            if build_version then
                xcode_build_version = build_version;
            end
        end );
        assert( result == 0, "Running xcodebuild to extract Xcode version failed" );
        
        return xcode_version, xcode_build_version;
    end

    local function autodetect_macos_version()
        local os_version = "";

        local sw_vers = "/usr/bin/sw_vers";
        local arguments = "sw_vers -buildVersion";
        local result = forge:execute( sw_vers, arguments, nil, nil, function(line)
            local version = line:match( "%w+" );
            if version then 
                os_version = version;
            end
        end );
        assert( result == 0, "Running sw_vers to extract operating system version failed" );

        return os_version;
    end

    if forge:operating_system() == "macos" then
        local local_settings = forge.local_settings;
        if not local_settings.macos then
            local_settings.updated = true;
            local_settings.macos = {
                xcrun = "/usr/bin/xcrun";
                sdk_version = sdk_version;
                sdk_build_version = sdk_build_version;
                xcode_version = xcode_version;
                xcode_build_version = xcode_build_version;
                os_version = os_version;
            };
        end
    end
end

function macos.initialize( settings )
    if forge:operating_system() == "macos" then
        for _, architecture in ipairs(settings.architectures) do 
            forge:add_default_build( ("cc_macos_%s"):format(architecture), forge:configure {
                obj = ("%s/cc_macos_%s"):format( settings.obj, architecture );
                platform = "macos";
                sdkroot = 'macosx';
                xcrun = settings.macos.xcrun;
                architecture = architecture;
                default_architecture = architecture;
                cc = macos.cc;
                objc = macos.objc;
                build_library = macos.build_library;
                clean_library = macos.clean_library;
                build_executable = macos.build_executable;
                clean_executable = macos.clean_executable;
                lipo_executable = macos.lipo_executable;
                obj_directory = macos.obj_directory;
                cc_name = macos.cc_name;
                cxx_name = macos.cxx_name;
                obj_name = macos.obj_name;
                lib_name = macos.lib_name;
                dll_name = macos.dll_name;
                exe_name = macos.exe_name;
            } );
        end

        local settings = forge.settings;
        local architecture = settings.default_architecture;
        settings.obj = forge:root( ("%s/cc_macos_%s"):format(settings.obj, architecture) );
        settings.platform = "macos";
        settings.architecture = architecture;
        settings.default_architecture = architecture;
        settings.cc = macos.cc;
        settings.objc = macos.objc;
        settings.build_library = macos.build_library;
        settings.clean_library = macos.clean_library;
        settings.build_executable = macos.build_executable;
        settings.clean_executable = macos.clean_executable;
        settings.lipo_executable = macos.lipo_executable;
        settings.obj_directory = macos.obj_directory;
        settings.cc_name = macos.cc_name;
        settings.cxx_name = macos.cxx_name;
        settings.obj_name = macos.obj_name;
        settings.lib_name = macos.lib_name;
        settings.dll_name = macos.dll_name;
        settings.exe_name = macos.exe_name;
    end
end

function macos.cc( target )
    local flags = {
        '-DBUILD_OS_MACOS'
    };
    clang.append_defines( target, flags );
    clang.append_include_directories( target, flags );
    clang.append_compile_flags( target, flags );
    
    local macos_deployment_target = target.settings.macos_deployment_target;
    if macos_deployment_target then 
        table.insert( flags, ("-mmacosx-version-min=%s"):format(macos_deployment_target) );
    end

    local ccflags = table.concat( flags, " " );
    local xcrun = target.settings.macos.xcrun;

    for _, object in target:dependencies() do
        if object:outdated() then
            object:set_built( false );
            local source = object:dependency();
            print( forge:leaf(source:id()) );
            local dependencies = ("%s.d"):format( object:filename() );
            local output = object:filename();
            local input = forge:absolute( source:filename() );
            forge:system( 
                xcrun, 
                ('xcrun --sdk macosx clang %s -MMD -MF "%s" -o "%s" "%s"'):format(ccflags, dependencies, output, input)
            );
            clang.parse_dependencies_file( dependencies, object );
            object:set_built( true );
        end
    end
end

function macos.build_library( target )
    local flags = {
        "-static"
    };

    local settings = target.settings;
    forge:pushd( settings.obj_directory(target) );
    local objects =  {};
    for _, compile in target:dependencies() do
        local prototype = compile:prototype();
        if prototype == forge.Cc or prototype == forge.Cxx or prototype == forge.ObjC or prototype == forge.ObjCxx then
            for _, object in compile:dependencies() do
                table.insert( objects, forge:relative(object:filename()) );
            end
        end
    end
    
    if #objects > 0 then
        local arflags = table.concat( flags, " " );
        local arobjects = table.concat( objects, '" "' );
        local xcrun = target.settings.macos.xcrun;
        forge:system( xcrun, ('xcrun --sdk macosx libtool %s -o "%s" "%s"'):format(arflags, forge:native(target:filename()), arobjects) );
    end
    forge:popd();
end

function macos.clean_library( target )
    forge:rm( target:filename() );
end

function macos.build_executable( target )
    local flags = {};
    clang.append_link_flags( target, flags );

    local settings = target.settings;
    local macos_deployment_target = settings.macos_deployment_target;
    if macos_deployment_target then 
        table.insert( flags, ("-mmacosx-version-min=%s"):format(macos_deployment_target) );
    end

    local rpaths = target.rpaths;
    if rpaths then 
        for _, rpath in ipairs(rpaths) do 
            table.insert( flags, ('-rpath "%s"'):format(rpath) );
        end
    end

    clang.append_library_directories( target, flags );

    local objects = {};
    local libraries = {};

    local settings = target.settings;
    forge:pushd( settings.obj_directory(target) );
    for _, dependency in target:dependencies() do
        local prototype = dependency:prototype();
        if prototype == forge.Cc or prototype == forge.Cxx or prototype == forge.ObjC or prototype == forge.ObjCxx then
            assertf( target.architecture == dependency.architecture, "Architectures for '%s' (%s) and '%s' (%s) don't match", target:path(), tostring(target.architecture), dependency:path(), tostring(dependency.architecture) );
            for _, object in dependency:dependencies() do
                if object:prototype() == nil then
                    table.insert( objects, forge:relative(object:filename()) );
                end
            end
        elseif prototype == forge.StaticLibrary or prototype == forge.DynamicLibrary then
            table.insert( libraries, ("-l%s"):format(dependency:id()) );
        end
    end

    clang.append_link_libraries( target, libraries );

    if #objects > 0 then
        local ldflags = table.concat( flags, " " );
        local ldobjects = table.concat( objects, '" "' );
        local ldlibs = table.concat( libraries, " " );
        local xcrun = settings.macos.xcrun;
        forge:system( xcrun, ('xcrun --sdk macosx clang++ %s "%s" %s'):format(ldflags, ldobjects, ldlibs) );
    end
    forge:popd();
end

function macos.clean_executable( target )
    forge:rm( target:filename() );
end

function macos.lipo_executable( target )
    local executables = {};
    for _, executable in target:dependencies() do 
        table.insert( executables, executable:filename() );
    end
    executables = table.concat( executables, [[" "]] );
    local xcrun = target.settings.macos.xcrun;
    forge:system( xcrun, ('xcrun --sdk macosx lipo -create -output "%s" "%s"'):format(target:filename(), executables) );
end

function macos.obj_directory( target )
    local relative_path = forge:relative( target:working_directory():path(), forge:root() );
    return forge:absolute( relative_path, target.settings.obj );
end

function macos.cc_name( name )
    return ("%s.c"):format( forge:basename(name) );
end

function macos.cxx_name( name )
    return ("%s.cpp"):format( forge:basename(name) );
end

function macos.obj_name( name )
    return ("%s.o"):format( name );
end

function macos.lib_name( name )
    return ("lib%s.a"):format( name );
end

function macos.dll_name( name )
    return ("%s.dylib"):format( name );
end

function macos.exe_name( name, architecture )
    return name;
end

forge:register_module( macos );
