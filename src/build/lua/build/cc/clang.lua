
clang = {};

function clang.append_defines( target, defines )
    table.insert( defines, ('-DBUILD_PLATFORM_%s'):format(upper(platform)) );
    table.insert( defines, ('-DBUILD_VARIANT_%s'):format(upper(variant)) );
    table.insert( defines, ('-DBUILD_LIBRARY_TYPE_%s'):format(upper(target.settings.library_type)) );
    table.insert( defines, ('-DBUILD_BIN_DIRECTORY="\\"%s\\""'):format(target.settings.bin) );
    table.insert( defines, ('-DBUILD_MODULE_DIRECTORY="\\"%s\\""'):format(target:working_directory():path()) );
    table.insert( defines, ('-DBUILD_VERSION="\\"%s\\""'):format(version) );

    if string.find(target.settings.runtime_library, "debug", 1, true) then
        table.insert( defines, "-D_DEBUG" );
        table.insert( defines, "-DDEBUG" );
    else 
        table.insert( defines, "-DNDEBUG" );
    end

    if target.settings.defines then
        for _, define in ipairs(target.settings.defines) do
            table.insert( defines, ("-D%s"):format(define) );
        end
    end
    
    if target.defines then
        for _, define in ipairs(target.defines) do
            table.insert( defines, ("-D%s"):format(define) );
        end
    end
end

function clang.append_include_directories( target, include_directories )
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

    if target.framework_directories then 
        for _, directory in ipairs(target.framework_directories) do
            table.insert( include_directories, ('-F "%s"'):format(directory) );
        end
    end

    if target.settings.framework_directories then 
        for _, directory in ipairs(target.settings.framework_directories) do
            table.insert( include_directories, ('-F "%s"'):format(directory) );
        end
    end
end

function clang.append_compile_flags( target, flags )
    table.insert( flags, "-c" );
    table.insert( flags, ("-arch %s"):format(target.architecture) );
    table.insert( flags, "-fasm-blocks" );
    
    local language = target.language or "c++";
    if language then
        table.insert( flags, ("-x %s"):format(language) );
        if string.find(language, "c++", 1, true) then
            table.insert( flags, "-std=c++11" );
            table.insert( flags, "-stdlib=libc++" );
            table.insert( flags, "-Wno-deprecated" );
            if target.settings.exceptions then
                table.insert( flags, "-fexceptions" );
            end
            if target.settings.run_time_type_info then
                table.insert( flags, "-frtti" );
            end
        end

        if string.find(language, "objective", 1, true) then
            table.insert( flags, "-fobjc-abi-version=2" );
            table.insert( flags, "-fobjc-legacy-dispatch" );
            table.insert( flags, '"-DIBOutlet=__attribute__((iboutlet))"' );
            table.insert( flags, '"-DIBOutletCollection(ClassName)=__attribute__((iboutletcollection(ClassName)))"' );
            table.insert( flags, '"-DIBAction=void)__attribute__((ibaction)"' );
        end
    end
        
    if target.settings.debug then
        table.insert( flags, "-g3" );
    end

    if target.settings.optimization then
        table.insert( flags, "-O3" );
        table.insert( flags, "-Ofast" );
    end
    
    if target.settings.preprocess then
        table.insert( flags, "-E" );
    end

    if target.settings.runtime_checks then
        table.insert( flags, "-fstack-protector" );
    else
        table.insert( flags, "-fno-stack-protector" );
    end
end

function clang.append_library_directories( target, library_directories )
    if target.library_directories then
        for _, directory in ipairs(target.library_directories) do
            table.insert( library_directories, ('-L "%s"'):format(directory) );
        end
    end
    
    if target.settings.library_directories then
        for _, directory in ipairs(target.settings.library_directories) do
            table.insert( library_directories, ('-L "%s"'):format(directory) );
        end
    end
    
    if target.framework_directories then 
        for _, directory in ipairs(target.framework_directories) do
            table.insert( library_directories, ('-F "%s"'):format(directory) );
        end
    end
    
    if target.settings.framework_directories then 
        for _, directory in ipairs(target.settings.framework_directories) do
            table.insert( library_directories, ('-F "%s"'):format(directory) );
        end
    end
end

function clang.append_link_flags( target, flags )
    table.insert( flags, ("-arch %s"):format(target.architecture) );
    table.insert( flags, "-std=c++11" );
    table.insert( flags, "-stdlib=libc++" );

    if target:prototype() == DynamicLibrary then
        table.insert( flags, "-shared" );
        table.insert( flags, ("-Wl,--out-implib,%s"):format(native(("%s/%s"):format(target.settings.lib, lib_name(target:id())))) );
    end
    
    if target.settings.verbose_linking then
        table.insert( flags, "-Wl,--verbose=31" );
    end
    
    if target.settings.generate_map_file then
        table.insert( flags, ('-Wl,-map,"%s"'):format(native(("%s/%s.map"):format(obj_directory(target), target:id()))) );
    end

    if target.settings.strip and not target.settings.generate_dsym_bundle then
        table.insert( flags, "-Wl,-dead_strip" );
    end

    if target.settings.exported_symbols_list then
        table.insert( flags, ('-exported_symbols_list "%s"'):format(absolute(target.settings.exported_symbols_list)) );
    end

    table.insert( flags, ('-o "%s"'):format(native(target:filename())) );
end

function clang.append_link_libraries( target, libraries )
    if target.settings.third_party_libraries then
        for _, library in ipairs(target.settings.third_party_libraries) do
            table.insert( libraries, ("-l%s"):format(library) );
        end
    end

    if target.third_party_libraries then
        for _, library in ipairs(target.third_party_libraries) do
            table.insert( libraries, ("-l%s"):format(library) );
        end
    end

    if target.system_libraries then
        for _, library in ipairs(target.system_libraries) do 
            table.insert( libraries, ("-l%s"):format(library) );
        end
    end

    if target.frameworks then
        for _, framework in ipairs(target.frameworks) do
            table.insert( libraries, ('-framework "%s"'):format(framework) );
        end
    end

    if target.settings.frameworks then 
        for _, framework in ipairs(target.settings.frameworks) do
            table.insert( libraries, ('-framework "%s"'):format(framework) );
        end
    end
end
