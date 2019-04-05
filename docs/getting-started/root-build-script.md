---
layout: page
title: Root Build Script
parent: Getting Started
nav_order: 2
---

- TOC
{:toc}

The file named *forge.lua* in the project's root directory is the root build script.  The root build script is the entry point into the build system that defines and runs the build by configuring toolsets and then using them to create the targets and dependencies that express the files to be built.

An example root build script that copies files from one directory to another changing their extension from *.in* to *.out*:

~~~lua
local toolset = require 'forge' {
    output = root( 'output' );
};

toolset:all {
    toolset:Copy '${output}/%1.out' {
        'bar.in';
        'baz.in';
        'foo.in';
    };    
};
~~~

The root build script is a plain Lua script.  The syntax may appear a little foreign if you aren't familiar with Lua's syntactic sugar for function arguments with tables and string literals and "method" calls.

From the Lua Manual - [3.4.10 Function Calls](https://www.lua.org/manual/5.3/manual.html#3.4.10):

> A call v:name(args) is syntactic sugar for v.name(v,args), except that v is evaluated only once.

> A call of the form f{fields} is syntactic sugar for f({fields}); that is, the argument list is a single new table. A call of the form f'string' (or f"string" or f[[string]]) is syntactic sugar for f('string'); that is, the argument list is a single literal string.

Adding the omitted parentheses and parameters makes the function calls and their arguments a little clearer:

~~~lua
local toolset = require( 'forge' )( {
    output = root( 'output' );
} );

toolset.all( toolset, {
    toolset.Copy( toolset, '${output}/%1.out' )( {
        'bar.in';
        'baz.in';
        'foo.in';
    } );
} );
~~~

The main Forge module is required and called to create a toolset.  The toolset encapsulates settings and a set of tools that are used to define the dependency graph in the build.

The toolset is then used to define the dependency graph of targets in the build.  The `toolset:Copy` call returns targets that copy input files to output files.  The `toolset:all` call adds those targets to the "all" target that is built by default when Forge is invoked.

There's nothing special about the variable or identifier `output` in this example.  Forge itself has no knowledge of or special handling of it.  Any variables provided as settings for a toolset are available to be substituted into target identifiers using the common `${}` syntax.

### Typical C/C++ Project

The root build script for a C++ project is more interesting.  The following is the root build script used to configure the build for the Forge project.  Each section of the root build script is repeated and described in the sections that follow:

~~~lua
-- Reference Lua build scripts from within the source tree rather than 
-- relative to the build executable so that they can be edited in place during
-- development and are versioned along with the code that they are building.
package.path = root('src/forge/lua/?.lua')..';'..root('src/forge/lua/?/init.lua');

variant = variant or 'debug';

local cc = require 'forge.cc' {
    identifier = 'cc_${platform}_${architecture}';
    platform = operating_system();
    bin = root( ('%s/bin'):format(variant) );
    lib = root( ('%s/lib'):format(variant) );
    obj = root( ('%s/obj/cc_%s_x86_64'):format(variant, operating_system()) );
    include_directories = {
        root( 'src' );
        root( 'src/boost' );
        root( 'src/lua/src' );
        root( 'src/unittest-cpp' );
    };
    library_directories = {
        root( ('%s/lib'):format(variant) ),
    };
    defines = {
        ('BUILD_VARIANT_%s'):format( upper(variant) );
    };
    xcode = {
        xcodeproj = root( 'forge.xcodeproj' );
    };

    architecture = 'x86_64';
    assertions = variant ~= 'shipping';
    debug = variant ~= 'shipping';
    debuggable = variant ~= 'shipping';
    exceptions = true;
    fast_floating_point = variant ~= 'debug';
    incremental_linking = variant == 'debug';
    link_time_code_generation = variant == 'shipping';
    minimal_rebuild = variant == 'debug';
    optimization = variant ~= 'debug';
    run_time_checks = variant == 'debug';
    runtime_library = variant == 'debug' and 'static_debug' or 'static_release';
    run_time_type_info = true;
    stack_size = 1048576;
    standard = 'c++11';
    string_pooling = variant == 'shipping';
    strip = false;
    warning_level = 3;
    warnings_as_errors = true;
};

-- Bump the C++ standard to c++14 when building on Windows as that is the 
-- closest standard supported by Microsoft Visual C++.
local settings = cc.settings;
if settings.platform == 'windows' then
    settings.standard = 'c++14';
end

buildfile 'src/assert/assert.forge';
buildfile 'src/boost/boost.forge';
buildfile 'src/cmdline/cmdline.forge';
buildfile 'src/error/error.forge';
buildfile 'src/forge/forge.forge';
buildfile 'src/lua/lua.forge';
buildfile 'src/luaxx/luaxx.forge';
buildfile 'src/process/process.forge';
buildfile 'src/unittest-cpp/unittest-cpp.forge';

cc:all {
    'src/forge/forge/all';
    'src/forge/forge_hooks/all';
    'src/forge/forge_test/all';
};

function install()
    prefix = prefix and root( prefix ) or home( 'forge' );
    local failures = default();
    if failures == 0 then 
        cc:cpdir( '${prefix}/bin', '${bin}' );
        cc:cpdir( '${prefix}/lua', 'src/forge/lua' );
    end
end
~~~

### Package Path

The first block of code sets Lua's package path used to search for the modules referenced in any `require()` call.  Most root build scripts won't need to do this as the default path makes the main Forge module and C/C++ modules available by default.

~~~lua
package.path = root('src/forge/lua/?.lua')..';'..root('src/forge/lua/?/init.lua');
~~~

In the Forge project the package path is set to load the standard Forge build scripts from within the source tree rather than using the default path of relative to the Forge executable or using the standard Lua package path specified by the `LUA_PATH` environment variable.

A more typical usage is to add search paths to search in addition to those loaded by default.  This allows modules to be loaded that define custom, per-project build toolsets used for custom tools provided by an individual project.

### Default Values

Default values are provided for variables that are optionally set on the command line using a Lua idiom of assigning a value to itself or a default value.

~~~lua
variant = variant or 'debug';
~~~

In Lua the `or` operator returns its first argument if it is not false; otherwise it will return its second argument.  Unassigned variables evaluate to `nil` which is interpreted as false.  Thus the effect here is to preserve any value that may have been set on the command line or provide a sensible default otherwise.

### Modules

The C/C++ module is required and called to create a toolset for building C/C++ source files into executables and libraries.

~~~lua
local cc = require 'forge.cc' {
    identifier = 'cc_${platform}_${architecture}';
    platform = operating_system();
    -- ...
~~~

The `identifier` variable set here is special in that it will be interpolated implicitly and used to identify the toolset later when defining the dependency graph.  Most other strings appearing in settings are *not* interpolated implicitly.

### Settings

Most of other settings are used internally by the C/C++ module and its prototypes.  They specify typical settings for a C/C++ toolchain such as the architecture to build for, generating debug information, optimization level, etc.

The `bin`, `lib`, and `obj` settings are directories specified per variant to keep variant compilations independent.  There's no need to clean a previous build to build a different variant and switching variants won't cause a full rebuild.

These `bin`, `lib`, and `obj` settings are another example of settings that are ignored by Forge and are passed through to be referenced again in the buildfiles that define the dependency graph.  Making use of other variables is as simple as adding them in *forge.lua* and referencing them from one or more buildfiles.

~~~lua
-- ...
bin = root( ('%s/bin'):format(variant) );
lib = root( ('%s/lib'):format(variant) );
obj = root( ('%s/obj/cc_%s_x86_64'):format(variant, operating_system()) );
include_directories = {
    root( 'src' );
    root( 'src/boost' );
    root( 'src/lua/src' );
    root( 'src/unittest-cpp' );
};
library_directories = {
    root( ('%s/lib'):format(variant) ),
};
defines = {
    ('BUILD_VARIANT_%s'):format( upper(variant) );
};
xcode = {
    xcodeproj = root( 'forge.xcodeproj' );
};

architecture = 'x86_64';
assertions = variant ~= 'shipping';
debug = variant ~= 'shipping';
debuggable = variant ~= 'shipping';
-- ...
~~~

### Per-Platform Settings

Settings are able to be overridden globally for specific situations by updating the `settings` table of the toolset after it has been returned.

Changing settings of the toolset directly affects all targets that are then created with that toolset.  Toolsets can also be inherited and cloned to isolate changes to within buildfiles or to provide separate toolsets that duplicate most settings but differentiate others (e.g. machine architecture being the prime example).

The following snippet bumps the C++ standard required from C++11 to C++14 when building for Windows with Microsoft Visual C++ (this is the earliest standard supported by Visual C++):

~~~lua
-- Bump the C++ standard to c++14 when building on Windows as that is the 
-- closest standard supported by Microsoft Visual C++.
local settings = cc.settings;
if settings.platform == 'windows' then
    settings.standard = 'c++14';
end
~~~

### Buildfiles

The dependency graph is defined in a modular and reusable way by loading in one or more buildfiles.  Each buildfile defines a small piece of the dependency graph that can be reused again by different projects with different configuration provided through each project's root build script.

~~~lua
buildfile 'src/assert/assert.forge';
buildfile 'src/boost/boost.forge';
buildfile 'src/cmdline/cmdline.forge';
buildfile 'src/error/error.forge';
buildfile 'src/forge/forge.forge';
buildfile 'src/lua/lua.forge';
buildfile 'src/luaxx/luaxx.forge';
buildfile 'src/process/process.forge';
buildfile 'src/unittest-cpp/unittest-cpp.forge';
~~~

### Default Targets

Targets built when Forge is invoked from the root directory of the project are specified by making them dependencies of the special "all" target in the root directory through a call to `Toolset.all()` passing the paths to the targets to add as dependencies:

~~~lua
cc:all {
    'src/forge/forge/all';
    'src/forge/forge_hooks/all';
    'src/forge/forge_test/all';
};
~~~

### Custom Commands

The `install()` function specifies a new, custom build command by defining a global function.  Passing `install` on the command line will invoke this command to build and install Forge.  The variable `prefix` can be passed on the command line to override the default install location in the user's home directory.

~~~lua
function install()
    prefix = prefix and root( prefix ) or home( 'forge' );
    local failures = build();
    if failures == 0 then 
        cc:cpdir( '${prefix}/bin', '${bin}' );
        cc:cpdir( '${prefix}/lua', 'src/forge/lua' );
    end
end
~~~
