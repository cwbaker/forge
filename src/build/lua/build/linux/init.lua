
linux = {};

function linux.configure( settings )
    local local_settings = build.local_settings;

    if not local_settings.linux then
        local_settings.updated = true;
        local_settings.linux = {
            architectures = { "x86_64" };
            gcc = "/usr/bin/gcc";
            gxx = "/usr/bin/g++";
            ar = "/usr/bin/ar";
        };
    end
end

function linux.initialize( settings )
    local path = {
        "/usr/bin",
        "/bin"
    };
    linux.environment = {
        PATH = table.concat( path, ":" );
    };

    for _, architecture in ipairs(settings.linux.architectures) do 
        build:default_build( ("cc_linux_%s"):format(architecture), build:configure {
            obj = ("%s/cc_linux_%s"):format( settings.obj, architecture );
            platform = "linux";
            architecture = architecture;
            default_architecture = architecture;
            cc = linux.cc;
            build_library = linux.build_library;
            clean_library = linux.clean_library;
            build_executable = linux.build_executable;
            clean_executable = linux.clean_executable;
            obj_directory = linux.obj_directory;
            cc_name = linux.cc_name;
            cxx_name = linux.cxx_name;
            pch_name = linux.pch_name;
            pdb_name = linux.pdb_name;
            obj_name = linux.obj_name;
            lib_name = linux.lib_name;
            exp_name = linux.exp_name;
            dll_name = linux.dll_name;
            exe_name = linux.exe_name;        
            ilk_name = linux.ilk_name;
        } );
    end

    local settings = build.settings;
    local architecture = settings.default_architecture;
    settings.obj = build:root( ("%s/cc_linux_%s"):format(settings.obj, architecture) );
    settings.platform = "linux";
    settings.architecture = architecture;
    settings.default_architecture = architecture;
    settings.cc = linux.cc;
    settings.objc = linux.objc;
    settings.build_library = linux.build_library;
    settings.clean_library = linux.clean_library;
    settings.build_executable = linux.build_executable;
    settings.clean_executable = linux.clean_executable;
    settings.lipo_executable = linux.lipo_executable;
    settings.obj_directory = linux.obj_directory;
    settings.cc_name = linux.cc_name;
    settings.cxx_name = linux.cxx_name;
    settings.obj_name = linux.obj_name;
    settings.lib_name = linux.lib_name;
    settings.dll_name = linux.dll_name;
    settings.exe_name = linux.exe_name;    
end

function linux.cc( target )

    local flags = {
        "-DBUILD_OS_LINUX";
    };

    gcc.append_defines( target, flags );
    gcc.append_version_defines( target, flags );
    gcc.append_include_directories( target, flags );
    gcc.append_compile_flags( target, flags );

    local ccflags = table.concat( flags, " " );
    local gcc_ = target.settings.linux.gcc;
    
    for _, object in target:dependencies() do
        if object:outdated() then
            object:set_built( false );
            local source = object:dependency();
            print( build:leaf(source:id()) );
            local output = object:filename();
            local input = build:relative( source:filename() );
            build:system( 
                gcc_, 
                ('gcc %s -o "%s" "%s"'):format(ccflags, output, input), 
                linux.environment,
                build:dependencies_filter(object)
            );
            object:set_built( true );
        end
    end
end

function linux.build_library( target )
    local flags = {
        "-rcs"
    };
    
    local settings = target.settings;
    build:pushd( ("%s/%s_%s"):format(settings.obj_directory(target), settings.platform, settings.architecture) );
    local objects = {};
    for _, compile in target:dependencies() do
        local prototype = compile:prototype();
        if prototype == build.Cc or prototype == build.Cxx then
            for _, object in compile:dependencies() do
                table.insert( objects, build:relative(object:filename()) )
            end
        end
    end
    
    if #objects > 0 then
        local arflags = table.concat( flags, " " );
        local arobjects = table.concat( objects, '" "' );
        local ar = target.settings.linux.ar;
        build:system( ar, ('ar %s "%s" "%s"'):format(arflags, build:native(target:filename()), arobjects), linux.environment );
    end
    build:popd();
end

function linux.clean_library( target )
    build:rm( target );
    local settings = target.settings;
    build:rmdir( settings.obj_directory(target) );
end

function linux.build_executable( target )
    local flags = { 
        -- ("-Wl,-soname,%s"):format( build:leaf(target:filename()) ),
        -- "-shared",
        -- "-no-canonical-prefixes",
        -- "-Wl,--no-undefined",
        -- "-Wl,-z,noexecstack",
        -- "-Wl,-z,relro",
        -- "-Wl,-z,now",
        -- ('-o "%s"'):format( build:native(target:filename()) )
    };

    gcc.append_link_flags( target, flags );
    gcc.append_library_directories( target, flags );

    local objects = {};
    local libraries = {};

    local settings = target.settings;
    build:pushd( ("%s/%s_%s"):format(settings.obj_directory(target), settings.platform, settings.architecture) );
    for _, dependency in target:dependencies() do
        local prototype = dependency:prototype();
        if prototype == build.Cc or prototype == build.Cxx then
            for _, object in dependency:dependencies() do
                if object:prototype() == nil then
                    table.insert( objects, build:relative(object:filename()) );
                end
            end
        elseif prototype == build.StaticLibrary or prototype == build.DynamicLibrary then
            if dependency.whole_archive then
                table.insert( libraries, ("-Wl,--whole-archive") );
            end
            table.insert( libraries, ("-l%s"):format(dependency:id()) );
            if dependency.whole_archive then
                table.insert( libraries, ("-Wl,--no-whole-archive") );
            end
        end
    end

    gcc.append_link_libraries( target, libraries );

    if #objects > 0 then
        local ldflags = table.concat( flags, " " );
        local ldobjects = table.concat( objects, '" "' );
        local ldlibs = table.concat( libraries, " " );
        local gxx = settings.linux.gxx;
        build:system( gxx, ('g++ %s "%s" %s'):format(ldflags, ldobjects, ldlibs), linux.environment );
    end
    build:popd();
end 

function linux.clean_executable( target )
    build:rm( target );
    local settings = target.settings;
    build:rmdir( settings.obj_directory(target) );
end

function linux.obj_directory( target )
    return ("%s/%s"):format( target.settings.obj, build:relative(target:working_directory():path(), build:root()) );
end

function linux.cc_name( name )
    return ("%s.c"):format( build:basename(name) );
end

function linux.cxx_name( name )
    return ("%s.cpp"):format( build:basename(name) );
end

function linux.obj_name( name, architecture )
    return ("%s.o"):format( build:basename(name) );
end

function linux.lib_name( name )
    return ("lib%s.a"):format( name );
end

function linux.dll_name( name )
    return ("lib%s.so"):format( name );
end

function linux.exe_name( name )
    return ("%s"):format( name );
end

build:register_module( linux );
