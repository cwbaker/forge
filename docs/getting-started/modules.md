---
layout: page
title: Modules
parent: Getting Started
nav_order: 5
---

- TOC
{:toc}

Modules provide support for different tools used during the build.  C and C++ compilation is provided by the *cc* module, building Android packages is provided by the experimental *android* module, building for the Apple Store on macOS and iOS by the experimental *xcode* module, and so on.

Modules are loaded using Lua's built-in `require()` to search for and load the module.  Any value returned by the module is returned from the `require()` call.

Forge expects its modules to return a table that containing the functions `configure()`, `validate()`, and `initialize()`.  The module table should also be callable as a short form for loading and initalizing both the main Forge module and the loaded module and returning a toolset that access to the tools provided by the module.

An abridged copy of the *gcc* module that provides support for compiling C and C++ with GCC:

~~~lua
local gcc = {};

function gcc.configure( toolset, gcc_settings )
    local paths = os.getenv( 'PATH' );
    return {
        gcc = toolset:which( gcc_settings.gcc or os.getenv('CC') or 'gcc', paths );
        gxx = toolset:which( gcc_settings.gxx or os.getenv('CXX') or 'g++', paths );
        ar = toolset:which( gcc_settings.ar or os.getenv('AR') or 'ar', paths );
        environment = gcc_settings.environment or {
            PATH = '/usr/bin';
        };
    };
end

function gcc.validate( toolset, gcc_settings )
    return 
        exists( gcc_settings.gcc ) and 
        exists( gcc_settings.gxx ) and 
        exists( gcc_settings.ar )
    ;
end

function gcc.initialize( toolset )
    if toolset:configure(gcc, 'gcc') then
        local identifier = toolset.settings.identifier;
        if identifier then
            add_toolset( toolset:interpolate(identifier), toolset );
        end

        local Cc = forge:FilePrototype( 'Cc' );
        Cc.language = 'c';
        Cc.build = gcc.compile;
        toolset.Cc = forge:PatternElement( Cc, gcc.object_filename );

        local Cxx = forge:FilePrototype( 'Cxx' );
        Cxx.language = 'c++';
        Cxx.build = gcc.compile;
        toolset.Cxx = forge:PatternElement( Cxx, gcc.object_filename );

        local StaticLibrary = forge:FilePrototype( 'StaticLibrary', gcc.static_library_filename );
        StaticLibrary.build = gcc.archive;
        toolset.StaticLibrary = StaticLibrary;

        local DynamicLibrary = forge:FilePrototype( 'DynamicLibrary', gcc.dynamic_library_filename );
        DynamicLibrary.build = gcc.link;
        toolset.DynamicLibrary = DynamicLibrary;

        local Executable = forge:FilePrototype( 'Executable', gcc.executable_filename );
        Executable.build = gcc.link;
        toolset.Executable = Executable;

        toolset:defaults( toolset.settings, {
            architecture = 'x86_64';
            assertions = true;
            debug = true;
            -- ... other settings here skipped for clarity ...
            warning_level = 3;
            warnings_as_errors = true;
        } );

        return toolset;
    end
end

setmetatable( gcc, {
    __call = function( gcc, settings )
        local forge = require( 'forge' ):clone( settings );
        gcc.initialize( forge );
        return forge;
    end
} );

return gcc;
~~~

### Callable Modules

~~~lua
setmetatable( gcc, {
    __call = function( gcc, settings )
        local forge = require( 'forge' ):clone( settings );
        gcc.initialize( forge );
        return forge;
    end
} );
~~~

When appropriate the returned module table should be callable.  

Calling the table returned from the module should create a toolset initialized to use the tools configure the tools provided by that module.  The single, optional parameter to this call is a table containing settings that override any defaults provided by the module.

Simple builds require modules and treat them as callable objects to initialize a single default toolset for building.

More complex builds cans require modules and initialize them explicitly to register multiple toolsets to build for multiple platforms and/or architectures.

### Configure

The `configure()` function is called when when a module needs to detect per-machine settings (e.g. the location of the tool executables).  For example the *msvc* module will look for Microsoft Visual C++ compilers to be installed by executing the *vswhere* tool and parsing its output.

The `gcc.configure()` function searches for paths to GCC tools from settings provided in the root build script, environment variables, or default names. The `Toolset.which()` function finds the first file with a matching name in the list of paths provided.

~~~lua
function gcc.configure( toolset, gcc_settings )
    local paths = os.getenv( 'PATH' );
    return {
        gcc = toolset:which( gcc_settings.gcc or os.getenv('CC') or 'gcc', paths );
        gxx = toolset:which( gcc_settings.gxx or os.getenv('CXX') or 'g++', paths );
        ar = toolset:which( gcc_settings.ar or os.getenv('AR') or 'ar', paths );
        environment = gcc_settings.environment or {
            PATH = '/usr/bin';
        };
    };
end
~~~

Configuration results are written to the *local_settings.lua* file in the project root directory.  This file should be ignored and left out of version control.  It contains the per-machine per-project settings of any modules used in the build.

The `configure()` function can be passed settings from the root build script, *forge.lua*, to override or provide specific values instead of accepting the defaults provided by a module.  See each module's specific documentation for which settings this applies to and how.

Configuration can be hand-edited in *local_settings.lua* after it has been generated to override any defaults selected by modules.

### Validate

The `validate()` function is called for every build.  Each toolset is validated to make sure that the tools that are expected are still available in the directories that were configured in the configuration step.

~~~lua
function gcc.validate( toolset, gcc_settings )
    return 
        exists( gcc_settings.gcc ) and 
        exists( gcc_settings.gxx ) and 
        exists( gcc_settings.ar )
    ;
end
~~~

Modules that fail validation typically don't register any toolsets with Forge and thus anything built with them silently fails.  This behaviour can be overridden by asserting that particular toolsets are available or that particular module validation is successful by calling `validate()` explicitly and that it returns true.

### Initialize

~~~lua
function gcc.initialize( toolset )
    if toolset:configure(gcc, 'gcc') then
        local identifier = toolset.settings.identifier;
        if identifier then
            add_toolset( toolset:interpolate(identifier), toolset );
        end

        local Cc = forge:FilePrototype( 'Cc' );
        Cc.language = 'c';
        Cc.build = gcc.compile;
        toolset.Cc = forge:PatternElement( Cc, gcc.object_filename );

        local Cxx = forge:FilePrototype( 'Cxx' );
        Cxx.language = 'c++';
        Cxx.build = gcc.compile;
        toolset.Cxx = forge:PatternElement( Cxx, gcc.object_filename );

        local StaticLibrary = forge:FilePrototype( 'StaticLibrary', gcc.static_library_filename );
        StaticLibrary.build = gcc.archive;
        toolset.StaticLibrary = StaticLibrary;

        local DynamicLibrary = forge:FilePrototype( 'DynamicLibrary', gcc.dynamic_library_filename );
        DynamicLibrary.build = gcc.link;
        toolset.DynamicLibrary = DynamicLibrary;

        local Executable = forge:FilePrototype( 'Executable', gcc.executable_filename );
        Executable.build = gcc.link;
        toolset.Executable = Executable;

        toolset:defaults( toolset.settings, {
            architecture = 'x86_64';
            assertions = true;
            debug = true;
            -- ... other settings here skipped for clarity ...
            warning_level = 3;
            warnings_as_errors = true;
        } );

        return toolset;
    end
end
~~~

Initialization runs at least once per module per build and possibly more than once per module in the case of a modules that register multiple toolsets, e.g. to support building multiple architectures.
