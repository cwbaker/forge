
function autodetect_mingw_directory()
    local mingw_directory = "C:/MinGW";
    return mingw_directory;
end

function mingw( settings )
    -- Make sure that the environment variable VS_UNICODE_OUTPUT is not set.  
    -- Visual Studio sets this to signal its tools to communicate back to 
    -- Visual Studio using named pipes rather than stdout so that unicode output 
    -- works better but this then prevents the build tool from intercepting
    -- and collating this output.
    -- See http://blogs.msdn.com/freik/archive/2006/04/05/569025.aspx.
    putenv( "VS_UNICODE_OUTPUT", "" );

    local mingw_directory = settings.mingw.mingw_directory;
    
    local path = {
        "%s/bin" % mingw_directory,
        getenv("PATH")
    };
    putenv( "PATH", table.concat(path, ";") );
    
    local include = {
        "%s/include" % mingw_directory,
        getenv("INCLUDE")
    };
    putenv( "INCLUDE", table.concat(include, ";") );
    
    local lib = {
        "%s/lib" % mingw_directory,
        getenv("LIB")
    };
    putenv( "LIB", table.concat(lib, ";") );
end

if platform == "mingw" then    
    function cc( target )
        local gcc = "%s/bin/gcc.exe" % target.settings.mingw.mingw_directory;

        local cppdefines = "";
        cppdefines = cppdefines.." -DBUILD_PLATFORM_"..upper(platform);
        cppdefines = cppdefines.." -DBUILD_VARIANT_"..upper(variant);
        cppdefines = cppdefines.." -DBUILD_LIBRARY_SUFFIX=\"\\\"_"..platform.."_"..variant..".lib\\\"\"";
        cppdefines = cppdefines.." -DBUILD_MODULE_"..upper(string.gsub(target.module:id(), "-", "_"))
        cppdefines = cppdefines.." -DBUILD_LIBRARY_TYPE_"..upper(target.settings.library_type);

        if target.settings.defines then
            for _, define in ipairs(target.settings.defines) do
                cppdefines = cppdefines.." -D"..define;
            end
        end
        
        if target.defines then
            for _, define in ipairs(target.defines) do
                cppdefines = cppdefines.." -D"..define;
            end
        end

        local cppdirs = "";
        if target.include_directories then
            for _, directory in ipairs(target.include_directories) do
                cppdirs = cppdirs.." -I\""..relative(directory).."\"";
            end
        end

        if target.settings.include_directories then
            for _, directory in ipairs(target.settings.include_directories) do
                cppdirs = cppdirs.." -I\""..directory.."\"";
            end
        end

        local ccflags = " -c -fpermissive -Wno-deprecated";
        
        if target.settings.compile_as_c then
            ccflags = ccflags.." -x c";
        else
            ccflags = ccflags.." -x c++";
        end
        
        if target.settings.runtime_library == "static" or target.settings.runtime_library == "static_debug" then
            ccflags = ccflags.." -static-libstdc++";
        end
        
        if target.settings.debug then
            ccflags = ccflags.." -g";
        end

        if target.settings.exceptions then
            ccflags = ccflags.." -fexceptions";
        end

        if target.settings.optimization then
            ccflags = ccflags.." -O2";
        end
        
        if target.settings.preprocess then
            ccflags = ccflags.." -E";
        end

        if target.settings.run_time_type_info then
            ccflags = ccflags.." -frtti";
        end

        if target.precompiled_header ~= nil then            
            if target.precompiled_header:is_outdated() then
                print( leaf(target.precompiled_header.source) );
                system( gcc, "gcc"..cppdirs..cppdefines..ccflags.." -o"..obj_directory(target)..obj_name(target.precompiled_header.source).." "..target.precompiled_header.source, GccScanner );
            end        
        end
        
        local GccScanner = Scanner {
            [ [[((?:[A-Z]\:)?[^\:]+)\:([0-9]+)\:[0-9]+\: ([^\:]+)\:(.*)]] ] = function( filename, line, class, message )
                print( "%s(%s): %s: %s" % {filename, line, class, message} );
            end;
        };

        cppdefines = cppdefines.." -DBUILD_VERSION=\"\\\""..version.."\\\"\"";
        for dependency in target:get_dependencies() do
            if dependency:is_outdated() and dependency ~= target.precompiled_header then
                if dependency:rule() == File then
                    print( leaf(dependency.source) );
                    system( gcc, "gcc"..cppdirs..cppdefines..ccflags.." -o"..obj_directory(target)..obj_name(dependency.source).." "..dependency.source, GccScanner );
                elseif dependency.results then
                    for _, result in ipairs(dependency.results) do
                        if result:is_outdated() then
                            print( leaf(result.source) );
                            system( gcc, "gcc"..cppdirs..cppdefines..ccflags.." -o"..obj_directory(target)..obj_name(result.source).." "..result.source, GccScanner );
                        end
                    end
                end
            end    
        end
    end

    function build_library( target )
        local ar = "%s/bin/ar.exe" % target.settings.mingw.mingw_directory;

        local arflags = " ";
        
        local objects = "";
        for dependency in target:get_dependencies() do
            if dependency:rule() == Cc then
                if dependency.precompiled_header ~= nil then
                    objects = objects.." "..obj_name( dependency.precompiled_header:id() );
                end
                
                for object in dependency:get_dependencies() do
                    if object:rule() == File and object ~= dependency.precompiled_header then
                        objects = objects.." "..obj_name( object:id() );
                    end
                end
            end
        end
        
        if objects ~= "" then
            print( lib_name(target:id()) );
            pushd( obj_directory(target) );
            system( ar, "ar"..arflags.." -rcs "..native(target:get_filename())..objects );
            popd();
        end
    end

    function clean_library( target )
        rm( "%s/%s" % {target.settings.lib, lib_name(target:id())} );
        rmdir( obj_directory(target) );    
    end

    function build_executable( target )
        local gxx = "%s/bin/g++.exe" % target.settings.mingw.mingw_directory;

        local ldlibs = " ";
        
        local lddirs = " -L \""..target.settings.lib.."\"";

        if target.settings.library_directories then
            for _, directory in ipairs(target.settings.library_directories) do
                lddirs = lddirs.." -L \""..directory.."\"";
            end
        end
        
        local ldflags = " ";

        local out = "";
        if target:rule() == Executable then
            out = native( target:get_filename() );
            ldflags = ldflags.." -o "..out;
        elseif target:rule() == Library then
            out = native( target.settings.bin.."/"..dll_name(target:id()) );
            ldflags = ldflags.." -o "..out;
            ldflags = ldflags.." -shared -Wl,--out-implib,"..native( target.settings.lib.."/"..lib_name(target:id()) );
        end
        
        if target.settings.verbose_linking then
            ldflags = ldflags.." -Wl,--verbose=31";
        end
        
        if target.settings.runtime_library == "static" or target.settings.runtime_library == "static_debug" then
            ldflags = ldflags.." -static-libstdc++";
        end
        
        if target.settings.debug then
            ldflags = ldflags.." -debug";
        end

        if target.settings.generate_map_file then
            ldflags = ldflags.." -Wl,-Map,"..native(obj_directory(target)..target:id()..".map");
        end

        if target.settings.stack_size then
            ldflags = ldflags.." -Wl,--stack,"..tostring(target.settings.stack_size);
        end
        
        if target.settings.strip then
            ldflags = ldflags.." -Wl,--strip-all";
        end

        local libraries = "";
        if target.libraries then
            for _, library in ipairs(target.libraries) do
                libraries = "%s -l%s_%s_%s" % { libraries, library:id(), platform, variant };
            end
        end
        if target.third_party_libraries then
            for _, library in ipairs(target.third_party_libraries) do
                libraries = "%s -l%s" % { libraries, library };
            end
        end

        local objects = "";
        for dependency in target:get_dependencies() do
            if dependency:rule() == Cc then
                if dependency.precompiled_header ~= nil then
                    objects = objects.." "..obj_name( dependency.precompiled_header:id() );
                end
                
                for object in dependency:get_dependencies() do
                    if object:rule() == File and object ~= dependency.precompiled_header then
                        objects = objects.." "..obj_name( object:id() );
                    end
                end
            end
        end

        if objects ~= "" then
            if target:rule() == Executable then
                print( exe_name(target:id()) );
            else
                print( dll_name(target:id()) );
            end

            pushd( obj_directory(target) );
            system( gxx, "g++"..ldflags..lddirs..objects..libraries..ldlibs );
            popd();
        end
    end 

    function clean_executable( target )
        if target:rule() == Executable then
            rm( target.settings.bin.."/"..exe_name(target:id()) );
            rm( target.settings.bin.."/"..ilk_name(target:id()) );
            rmdir( obj_directory(target) );
        else        
            rm( target.settings.bin.."/"..dll_name(target:id()) );
            rm( target.settings.bin.."/"..ilk_name(target:id()) );
            rm( target.settings.lib.."/"..lib_name(target:id()) );
            rm( target.settings.lib.."/"..exp_name(target:id()) );
            rmdir( obj_directory(target) );
        end        
    end

    function obj_directory( target )
        return target.settings.obj.."/"..platform.."_"..variant.."/"..relative( target:directory(), root() ).."/";
    end

    function cc_name( name )
        return basename( name )..".c";
    end

    function cxx_name( name )
        return basename( name )..".cpp";
    end

    function obj_name( name )
        return basename( name )..".o";
    end

    function lib_name( name )
        return "%s_%s_%s.lib" % { name, platform, variant };
    end

    function exp_name( name )
        return name.."_"..platform.."_"..variant..".exp";
    end

    function dll_name( name )
        return name.."_"..platform.."_"..variant..".dll";
    end

    function exe_name( name )
        return name.."_"..platform.."_"..variant..".exe";
    end

    function ilk_name( name )
        return name.."_"..platform.."_"..variant..".ilk";
    end
end
