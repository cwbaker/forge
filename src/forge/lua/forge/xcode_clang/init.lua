
local clang = require 'forge.cc.clang';

local xcode_clang = {};

function xcode_clang.object_filename( forge, identifier )
    return ('%s.o'):format( identifier );
end

function xcode_clang.static_library_filename( forge, identifier )
    local identifier = forge:absolute( forge:interpolate(identifier) );
    local filename = ('%s/lib%s.a'):format( forge:branch(identifier), forge:leaf(identifier) );
    return identifier, filename;
end

function xcode_clang.dynamic_library_filename( forge, identifier )
    local identifier = forge:absolute( forge:interpolate(identifier) );
    local filename = ('%s.dylib'):format( identifier );
    return identifier, filename;
end

function xcode_clang.executable_filename( forge, identifier )
    local identifier = forge:interpolate( identifier );
    local filename = identifier;
    return identifier, filename;
end

-- Compile C, C++, Objective-C, and Objective-C++.
function xcode_clang.compile( forge, target ) 
    local settings = forge.settings;

    local flags = {};
    xcode_clang.append_defines( forge, target, flags );
    xcode_clang.append_include_directories( forge, target, flags );
    xcode_clang.append_compile_flags( forge, target, flags );
    
    local sdkroot = settings.sdkroot;
    if sdkroot == 'macosx' then 
        local macos_deployment_target = settings.macos_deployment_target;
        if macos_deployment_target then 
            table.insert( flags, ('-mmacosx-version-min=%s'):format(macos_deployment_target) );
        end
    elseif sdkroot == 'iphoneos' then
        local iphoneos_deployment_target = settings.iphoneos_deployment_target;
        if iphoneos_deployment_target then 
            table.insert( flags, ('-miphoneos-version-min=%s'):format(iphoneos_deployment_target) );
        end
    end

    local ccflags = table.concat( flags, ' ' );
    local xcrun = settings.xcrun;
    local source = target:dependency();
    print( forge:leaf(source) );
    local dependencies = ('%s.d'):format( target );
    local output = target:filename();
    local input = forge:absolute( source );
    forge:system( 
        xcrun, 
        ('xcrun --sdk %s clang %s -MMD -MF "%s" -o "%s" "%s"'):format(sdkroot, ccflags, dependencies, output, input)
    );
    xcode_clang.parse_dependencies_file( forge, dependencies, target );
end

-- Archive objects into a static library. 
function xcode_clang.archive( forge, target )
    local flags = {
        '-static'
    };

    local settings = forge.settings;
    forge:pushd( settings.obj_directory(forge, target) );
    local objects =  {};
    for _, object in forge:walk_dependencies( target ) do
        local prototype = object:prototype();
        if prototype == forge.Cc or prototype == forge.Cxx or prototype == forge.ObjC or prototype == forge.ObjCxx then
            table.insert( objects, forge:relative(object) );
        end
    end
    
    if #objects > 0 then
        local arflags = table.concat( flags, ' ' );
        local arobjects = table.concat( objects, '" "' );
        local xcrun = settings.xcrun;
        forge:system( xcrun, ('xcrun --sdk macosx libtool %s -o "%s" "%s"'):format(arflags, forge:native(target), arobjects) );
    end
    forge:popd();
end

-- Link dynamic libraries and executables.
function xcode_clang.link( forge, target ) 
    local settings = forge.settings;

    local objects = {};
    local libraries = {};
    forge:pushd( settings.obj_directory(forge, target) );
    for _, dependency in forge:walk_dependencies(target) do
        local prototype = dependency:prototype();
        if prototype == forge.Cc or prototype == forge.Cxx or prototype == forge.ObjC or prototype == forge.ObjCxx then
            table.insert( objects, forge:relative(dependency) );
        elseif prototype == forge.StaticLibrary or prototype == forge.DynamicLibrary then
            table.insert( libraries, ('-l%s'):format(dependency:id()) );
        end
    end

    local flags = {};
    xcode_clang.append_link_flags( forge, target, flags );
    xcode_clang.append_library_directories( forge, target, flags );
    xcode_clang.append_link_libraries( forge, target, libraries );

    local sdkroot = settings.sdkroot;
    if sdkroot == 'macosx' then 
        local macos_deployment_target = settings.macos_deployment_target;
        if macos_deployment_target then 
            table.insert( flags, ('-mmacosx-version-min=%s'):format(macos_deployment_target) );
        end
    elseif sdkroot == 'iphoneos' then
        local iphoneos_deployment_target = settings.iphoneos_deployment_target;
        if iphoneos_deployment_target then 
            if settings.platform == 'ios' then 
                table.insert( flags, ('-mios-version-min=%s'):format(iphoneos_deployment_target) );
            elseif settings.platform == 'ios_simulator' then
                table.insert( flags, ('-mios-simulator-version-min=%s'):format(iphoneos_deployment_target) );
            end
        end
    end

    if #objects > 0 then
        local xcrun = settings.xcrun;
        local sdkroot = settings.sdkroot;
        local ldflags = table.concat( flags, ' ' );
        local ldobjects = table.concat( objects, '" "' );
        local ldlibs = table.concat( libraries, ' ' );
        forge:system( xcrun, ('xcrun --sdk %s clang++ %s "%s" %s'):format(sdkroot, ldflags, ldobjects, ldlibs) );
    end
    forge:popd();
end

function xcode_clang.lipo( forge, target )
    local executables = {};
    for _, executable in target:dependencies() do 
        table.insert( executables, executable:filename() );
    end
    executables = table.concat( executables, '" "' );
    local settings = forge.settings;
    local xcrun = settings.xcrun;
    local sdkroot = settings.sdkroot;
    forge:system( xcrun, ('xcrun --sdk %s lipo -create -output "%s" "%s"'):format(sdkroot, target:filename(), executables) );
end

local Cc = forge:PatternPrototype( 'Cc', xcode_clang.object_filename );
Cc.language = 'c';
Cc.build = xcode_clang.compile;
xcode_clang.Cc = Cc;

local Cxx = forge:PatternPrototype( 'Cxx', xcode_clang.object_filename );
Cxx.language = 'c++';
Cxx.build = xcode_clang.compile;
xcode_clang.Cxx = Cxx;

local ObjC = forge:PatternPrototype( 'ObjC', xcode_clang.object_filename );
ObjC.language = 'objective-c';
ObjC.build = xcode_clang.compile;
xcode_clang.ObjC = ObjC;

local ObjCxx = forge:PatternPrototype( 'ObjCxx', xcode_clang.object_filename );
ObjCxx.language = 'objective-c++';
ObjCxx.build = xcode_clang.compile;
xcode_clang.ObjCxx = ObjCxx;

local StaticLibrary = forge:FilePrototype( 'StaticLibrary', xcode_clang.static_library_filename );
StaticLibrary.build = xcode_clang.archive;
xcode_clang.StaticLibrary = StaticLibrary;

local DynamicLibrary = forge:FilePrototype( 'DynamicLibrary', xcode_clang.dynamic_library_filename );
DynamicLibrary.build = xcode_clang.link;
xcode_clang.DynamicLibrary = DynamicLibrary;

local Executable = forge:FilePrototype( 'Executable', xcode_clang.executable_filename );
Executable.build = xcode_clang.link;
xcode_clang.Executable = Executable;

local Lipo = forge:FilePrototype( 'Lipo' );
Lipo.build = xcode_clang.lipo;
xcode_clang.Lipo = Lipo;

-- Register the clang C/C++ toolset in *forge*.
function xcode_clang.register( forge )
    forge.Cc = xcode_clang.Cc;
    forge.Cxx = xcode_clang.Cxx;
    forge.ObjC = xcode_clang.ObjC;
    forge.ObjCxx = xcode_clang.ObjCxx;
    forge.StaticLibrary = xcode_clang.StaticLibrary;
    forge.DynamicLibrary = xcode_clang.DynamicLibrary;
    forge.Executable = xcode_clang.Executable;
    forge.Lipo = xcode_clang.Lipo;
end

function xcode_clang.append_defines( forge, target, flags )
	clang.append_defines( forge, target, flags );
end

function xcode_clang.append_include_directories( forge, target, flags )
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

function xcode_clang.append_compile_flags( forge, target, flags )
	clang.append_compile_flags( forge, target, flags );
end

function xcode_clang.append_library_directories( forge, target, library_directories )
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

function xcode_clang.append_link_flags( forge, target, flags )
    clang.append_link_flags( forge, target, flags );

    local rpaths = target.rpaths;
    if rpaths then 
        for _, rpath in ipairs(rpaths) do 
            table.insert( flags, ('-rpath "%s"'):format(rpath) );
        end
    end
end

function xcode_clang.append_link_libraries( forge, target, libraries )
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

require "forge.xcode_clang.AssetCatalog";
require "forge.xcode_clang.Plist";
require "forge.xcode_clang.Lipo";
require "forge.xcode_clang.Xib";

forge:register_module( xcode_clang );
return xcode_clang;