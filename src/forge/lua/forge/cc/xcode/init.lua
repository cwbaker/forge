
local clang = require 'forge.cc.clang';
local xcodeproj = require 'forge.cc.xcode.xcodeproj';

local xcode = {};

function xcode.configure( forge )
    local function autodetect_sdk_version( sdk )
        local sdk_version = '';
        local sdk_build_version = '';
        local xcodebuild = '/usr/bin/xcodebuild';
        local arguments = ('xcodebuild -sdk %s -version'):format( sdk );
        local result = forge:execute( xcodebuild, arguments, nil, nil, function(line)
            local key, value = line:match( '(%w+): ([^\n]+)' );
            if key and value then 
                if key == 'ProductBuildVersion' then 
                    sdk_build_version = value;
                elseif key == 'SDKVersion' then
                    sdk_version = value;
                end
            end
        end );
        assert( result == 0, 'Running xcodebuild to extract SDK name and version failed' );
        return sdk_version, sdk_build_version;
    end

    local function autodetect_xcode_version()
        local xcode_version = '';
        local xcode_build_version = '';
        local xcodebuild = '/usr/bin/xcodebuild';
        local arguments = 'xcodebuild -version';
        local result = forge:execute( xcodebuild, arguments, nil, nil, function(line)
            local major, minor = line:match( 'Xcode (%d+)%.(%d+)' );
            if major and minor then 
                xcode_version = ('%02d%02d'):format( tonumber(major), tonumber(minor) );
            end

            local build_version = line:match( 'Build version (%w+)' )
            if build_version then
                xcode_build_version = build_version;
            end
        end );
        assert( result == 0, 'Running xcodebuild to extract Xcode version failed' );
        return xcode_version, xcode_build_version;
    end

    local function autodetect_macos_version()
        local os_version = '';
        local sw_vers = '/usr/bin/sw_vers';
        local arguments = 'sw_vers -buildVersion';
        local result = forge:execute( sw_vers, arguments, nil, nil, function(line)
            local version = line:match( '%w+' );
            if version then 
                os_version = version;
            end
        end );
        assert( result == 0, 'Running sw_vers to extract operating system version failed' );
        return os_version;
    end

    local settings = forge.settings;
    local local_settings = forge.local_settings;
    if forge:operating_system() == 'macos' then
        if not local_settings.xcode then
            local macosx_sdk_version, macosx_sdk_build_version = autodetect_sdk_version( 'macosx' );
            local iphoneos_sdk_version, iphoneos_sdk_build_version = autodetect_sdk_version( 'iphoneos' );
            local xcode_version, xcode_build_version = autodetect_xcode_version();
            local os_version = autodetect_macos_version();
            local_settings.updated = true;
            local_settings.xcode = {
                xcrun = "/usr/bin/xcrun";
                codesign = "/usr/bin/codesign";
                plutil = "/usr/bin/plutil";
                signing_identity = "iPhone Developer";
                sdk_version = {
                    iphoneos = iphoneos_sdk_version;
                    macosx = macosx_sdk_version;
                };
                sdk_build_version = {
                    iphoneos = iphoneos_sdk_build_version;
                    macosx = macosx_sdk_build_version;
                };
                xcode_version = xcode_version;
                xcode_build_version = xcode_build_version;
                os_version = os_version;
            };
        end
        return true;
    end
end

function xcode.initialize( forge )
    if xcode.configure(forge, forge.local_settings) then 
        local identifier = forge.settings.identifier;
        if identifier then
            forge:add_build( forge:interpolate(identifier), forge );
        end

        local settings = forge.settings;
        if settings.platform == 'ios' then
            forge.App = require 'forge.cc.xcode.App';
        end
        forge.AssetCatalog = require 'forge.cc.xcode.AssetCatalog';
        forge.Plist = require 'forge.cc.xcode.Plist';
        forge.Lipo = require 'forge.cc.xcode.Lipo';
        forge.Xib = require 'forge.cc.xcode.Xib';

        local Cc = forge:PatternPrototype( 'Cc', xcode.object_filename );
        Cc.language = 'c';
        Cc.build = xcode.compile;
        forge.Cc = Cc;

        local Cxx = forge:PatternPrototype( 'Cxx', xcode.object_filename );
        Cxx.language = 'c++';
        Cxx.build = xcode.compile;
        forge.Cxx = Cxx;

        local ObjC = forge:PatternPrototype( 'ObjC', xcode.object_filename );
        ObjC.language = 'objective-c';
        ObjC.build = xcode.compile;
        forge.ObjC = ObjC;

        local ObjCxx = forge:PatternPrototype( 'ObjCxx', xcode.object_filename );
        ObjCxx.language = 'objective-c++';
        ObjCxx.build = xcode.compile;
        forge.ObjCxx = ObjCxx;

        local StaticLibrary = forge:FilePrototype( 'StaticLibrary', xcode.static_library_filename );
        StaticLibrary.build = xcode.archive;
        forge.StaticLibrary = StaticLibrary;

        local DynamicLibrary = forge:FilePrototype( 'DynamicLibrary', xcode.dynamic_library_filename );
        DynamicLibrary.build = xcode.link;
        forge.DynamicLibrary = DynamicLibrary;

        local Executable = forge:FilePrototype( 'Executable', xcode.executable_filename );
        Executable.build = xcode.link;
        forge.Executable = Executable;

        forge:defaults( forge.settings, {
            architecture = 'x86_64';
            assertions = true;
            compile_as_c = false;
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
            subsystem = "CONSOLE";
            verbose_linking = false;
            warning_level = 3;
            warnings_as_errors = true;
        } );

        if settings.platform == 'macos' then
            settings.sdkroot = 'macosx';
        elseif settings.platform == 'ios' then 
            settings.sdkroot = 'iphoneos';
        end

        return xcode;
    end
end

function xcode.object_filename( forge, identifier )
    return ('%s.o'):format( identifier );
end

function xcode.static_library_filename( forge, identifier )
    local identifier = forge:absolute( forge:interpolate(identifier) );
    local filename = ('%s/lib%s.a'):format( forge:branch(identifier), forge:leaf(identifier) );
    return identifier, filename;
end

function xcode.dynamic_library_filename( forge, identifier )
    local identifier = forge:absolute( forge:interpolate(identifier) );
    local filename = ('%s.dylib'):format( identifier );
    return identifier, filename;
end

function xcode.executable_filename( forge, identifier )
    local identifier = forge:interpolate( identifier );
    local filename = identifier;
    return identifier, filename;
end

-- Compile C, C++, Objective-C, and Objective-C++.
function xcode.compile( forge, target ) 
    local settings = forge.settings;

    local flags = {};
    xcode.append_defines( forge, target, flags );
    xcode.append_include_directories( forge, target, flags );
    xcode.append_compile_flags( forge, target, flags );
    xcode.append_deployment_target_flags( forge, target, flags );

    local sdkroot = settings.sdkroot;
    local ccflags = table.concat( flags, ' ' );
    local xcrun = settings.xcode.xcrun;
    local source = target:dependency();
    print( forge:leaf(source) );
    local dependencies = ('%s.d'):format( target );
    local output = target:filename();
    local input = forge:absolute( source );
    forge:system( 
        xcrun, 
        ('xcrun --sdk %s clang %s -MMD -MF "%s" -o "%s" "%s"'):format(sdkroot, ccflags, dependencies, output, input)
    );
    clang.parse_dependencies_file( forge, dependencies, target );
end

-- Archive objects into a static library. 
function xcode.archive( forge, target )
    local flags = {
        '-static'
    };

    local settings = forge.settings;
    forge:pushd( forge:obj_directory(target) );
    local objects =  {};
    for _, object in forge:walk_dependencies( target ) do
        local prototype = object:prototype();
        if prototype == forge.Cc or prototype == forge.Cxx or prototype == forge.ObjC or prototype == forge.ObjCxx then
            table.insert( objects, forge:relative(object) );
        end
    end
    
    if #objects > 0 then
        local sdkroot = forge.settings.sdkroot;
        local arflags = table.concat( flags, ' ' );
        local arobjects = table.concat( objects, '" "' );
        local xcrun = settings.xcode.xcrun;
        printf( '%s', forge:leaf(target) );
        forge:system( xcrun, ('xcrun --sdk %s libtool %s -o "%s" "%s"'):format(sdkroot, arflags, forge:native(target), arobjects) );
    end
    forge:popd();
end

-- Link dynamic libraries and executables.
function xcode.link( forge, target ) 
    local settings = forge.settings;

    local objects = {};
    local libraries = {};
    forge:pushd( forge:obj_directory(target) );
    for _, dependency in forge:walk_dependencies(target) do
        local prototype = dependency:prototype();
        if prototype == forge.Cc or prototype == forge.Cxx or prototype == forge.ObjC or prototype == forge.ObjCxx then
            table.insert( objects, forge:relative(dependency) );
        elseif prototype == forge.StaticLibrary or prototype == forge.DynamicLibrary then
            table.insert( libraries, ('-l%s'):format(dependency:id()) );
        end
    end

    local flags = {};
    xcode.append_link_flags( forge, target, flags );
    xcode.append_library_directories( forge, target, flags );
    xcode.append_link_libraries( forge, target, libraries );
    xcode.append_deployment_target_flags( forge, target, flags );

    if #objects > 0 then
        local xcrun = settings.xcode.xcrun;
        local sdkroot = settings.sdkroot;
        local ldflags = table.concat( flags, ' ' );
        local ldobjects = table.concat( objects, '" "' );
        local ldlibs = table.concat( libraries, ' ' );
        printf( '%s', forge:leaf(target) );
        forge:system( xcrun, ('xcrun --sdk %s clang++ %s "%s" %s'):format(sdkroot, ldflags, ldobjects, ldlibs) );
    end
    forge:popd();
end

-- Register the clang C/C++ toolset in *forge*.
function xcode.register( forge )
end

function xcode.append_defines( forge, target, flags )
	clang.append_defines( forge, target, flags );
end

function xcode.append_include_directories( forge, target, flags )
	clang.append_include_directories( forge, target, flags );

    if target.framework_directories then 
        for _, directory in ipairs(target.framework_directories) do
            table.insert( flags, ('-F "%s"'):format(directory) );
        end
    end

    local settings = forge.settings;
    if settings.framework_directories then 
        for _, directory in ipairs(settings.framework_directories) do
            table.insert( flags, ('-F "%s"'):format(directory) );
        end
    end
end

function xcode.append_compile_flags( forge, target, flags )
	clang.append_compile_flags( forge, target, flags );
end

function xcode.append_library_directories( forge, target, library_directories )
    clang.append_library_directories( forge, target, library_directories );
    
    if target.framework_directories then 
        for _, directory in ipairs(target.framework_directories) do
            table.insert( library_directories, ('-F "%s"'):format(directory) );
        end
    end
    
    local settings = forge.settings;
    if settings.framework_directories then 
        for _, directory in ipairs(settings.framework_directories) do
            table.insert( library_directories, ('-F "%s"'):format(directory) );
        end
    end
end

function xcode.append_deployment_target_flags( forge, target, flags )
    local settings = forge.settings;
    local sdkroot = settings.sdkroot;
    if sdkroot == 'macosx' then 
        local macos_deployment_target = settings.macos_deployment_target;
        if macos_deployment_target then 
            table.insert( flags, ('-mmacosx-version-min=%s'):format(macos_deployment_target) );
        end
    elseif sdkroot == 'iphoneos' then
        local ios_deployment_target = settings.ios_deployment_target;
        if ios_deployment_target then 
            table.insert( flags, ('-miphoneos-version-min=%s'):format(ios_deployment_target) );
        end
    end
end

function xcode.append_link_flags( forge, target, flags )
    clang.append_link_flags( forge, target, flags );

    local rpaths = target.rpaths;
    if rpaths then 
        for _, rpath in ipairs(rpaths) do 
            table.insert( flags, ('-rpath "%s"'):format(rpath) );
        end
    end
end

function xcode.append_link_libraries( forge, target, libraries )
	clang.append_link_libraries( forge, target, libraries );

    local settings = forge.settings;
    if settings.frameworks then 
        for _, framework in ipairs(settings.frameworks) do
            table.insert( libraries, ('-framework "%s"'):format(framework) );
        end
    end

    if target.frameworks then
        for _, framework in ipairs(target.frameworks) do
            table.insert( libraries, ('-framework "%s"'):format(framework) );
        end
    end
end

setmetatable( xcode, {
    __call = function( xcode, settings )
        local forge = require( 'forge' ):clone( settings );
        xcode.initialize( forge );
        return forge;
    end
} );

return xcode;