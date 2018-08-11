
gcc = {};

gcc.flags_by_architecture = {
    armv5 = "-march=armv5te -mtune=xscale -mthumb";
    armv7 = "-march=armv7 -mtune=xscale -mthumb";
    armv8 = "-march=armv8-a -mtune=xscale -mthumb";
    x86_64 = "";
};

function gcc.append_defines( target, flags )
    local settings = target.settings;
    table.insert( flags, ('-DBUILD_PLATFORM_%s'):format(forge:upper(settings.platform)) );
    table.insert( flags, ('-DBUILD_VARIANT_%s'):format(forge:upper(variant)) );
    table.insert( flags, ('-DBUILD_LIBRARY_TYPE_%s'):format(forge:upper(settings.library_type)) );
    table.insert( flags, ('-DBUILD_BIN_DIRECTORY="\\"%s\\""'):format(settings.bin) );
    table.insert( flags, ('-DBUILD_MODULE_DIRECTORY="\\"%s\\""'):format(target:working_directory():path()) );

    if string.find(settings.runtime_library, "debug", 1, true) then
        table.insert( flags, "-D_DEBUG" );
        table.insert( flags, "-DDEBUG" );
    else 
        table.insert( flags, "-DNDEBUG" );
    end

    local defines = settings.defines;
    if defines then
        for _, define in ipairs(defines) do
            table.insert( flags, ("-D%s"):format(define) );
        end
    end
    
    local defines = target.defines;
    if defines then
        for _, define in ipairs(defines) do
            table.insert( flags, ("-D%s"):format(define) );
        end
    end
end

function gcc.append_version_defines( target, flags )
    table.insert( flags, ('-DBUILD_VERSION="\\"%s\\""'):format(version) );
end

function gcc.append_include_directories( target, flags )
    if target.include_directories then
        for _, directory in ipairs(target.include_directories) do
            table.insert( flags, ('-I "%s"'):format(forge:relative(directory)) );
        end
    end

    if target.settings.include_directories then
        for _, directory in ipairs(target.settings.include_directories) do
            table.insert( flags, ('-I "%s"'):format(directory) );
        end
    end
end

function gcc.append_compile_flags( target, flags )
    table.insert( flags, "-c" );
    table.insert( flags, gcc.flags_by_architecture[target.architecture] );
    table.insert( flags, "-fpic" );
    table.insert( flags, "-MMD" );
    
    local language = target.language or "c++";
    if language then
        table.insert( flags, ("-x %s"):format(language) );
        if string.find(language, "c++", 1, true) then
            table.insert( flags, "-std=c++11" );
            if target.settings.exceptions then
                table.insert( flags, "-fexceptions" );
            end
            if target.settings.run_time_type_info then
                table.insert( flags, "-frtti" );
            end
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

    if target.settings.warnings_as_errors then 
        table.insert( flags, "-Werror" );
    end

    local warning_level = target.settings.warning_level
    if warning_level == 0 then 
        table.insert( flags, "-w" );
    elseif warning_level == 1 then
        table.insert( flags, "-Wall" );
    elseif warning_level >= 2 then
        table.insert( flags, "-Wall -Wextra" );
    end
end

function gcc.append_library_directories( target, library_directories )
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
end

function gcc.append_link_flags( target, flags )
    table.insert( flags, gcc.flags_by_architecture[target.architecture] );
    table.insert( flags, "-std=c++11" );

    if target:prototype() == forge.DynamicLibrary then
        table.insert( flags, "-shared" );
    end
    
    if target.settings.verbose_linking then
        table.insert( flags, "-verbose" );
    end
    
    if target.settings.debug then
        table.insert( flags, "-g" );
    end

    -- The latest GCC with Android (or clang with iOS) doesn't recognize 
    -- '-Wl,map' to specify the path to output a mapfile.
    -- if target.settings.generate_map_file then
    --     table.insert( flags, ('-Wl,-Map,"%s"'):format(forge:native(("%s.map"):format(target:filename()))) );
    -- end

    if target.settings.strip and not target.settings.generate_dsym_bundle then
        table.insert( flags, "-Wl,--strip-all" );
    end

    if target.settings.exported_symbols_list then
        table.insert( flags, ('-exported_symbols_list "%s"'):format(forge:absolute(target.settings.exported_symbols_list)) );
    end

    table.insert( flags, ('-o "%s"'):format(forge:native(target:filename())) );
end

function gcc.append_link_libraries( target, libraries )
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
end

forge:register_module( gcc );
