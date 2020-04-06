---
layout: page
title: Root Build Script
parent: Getting Started
nav_order: 2
---

- TOC
{:toc}

The root build script is the file named *forge.lua* in the project's root directory.  This root build script is the entry point into the build that defines which files are built, the dependencies between those files and source files, and the actions taken to build files that are outdated with respect to their source files.

A build is defined in two logical steps.  The first step creates one or more toolsets to encapsulate the tools and settings used in the build.  The second step uses those toolsets to create targets representing the files, dependencies, and actions in the build.

### Copying Files

Consider the following root build script for a build that copies the files `{bar,baz,foo}.in` to `output/{bar,baz,foo}.out`:

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

This is a plain Lua script.  The syntax may appear a little foreign if you aren't familiar with Lua's syntactic sugar that allows parentheses to be left out for function arguments with tables and string literals and "method" calls made with the colon syntax.

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

With syntax explained we can work through what this script actually means in the context of a build.  The logical split into two steps is quite clear with the first step being the creation of the toolset and the second step the calls to create targets and specify dependencies.

The toolset is created by requiring the `forge` module and then making a call on it passing the settings for apply to the toolset as the sole argument (recall that calls with a single table argument may omit parentheses).  The required module acts as a prototype for toolsets and calling it creates a toolset with behavior defined by that prototype.

The toolset is then used to define the dependency graph of files and actions in the build.  The call to `toolset.all()` makes the output files dependencies of the special "all" target so that they are built by default when `forge` is run from that directory.  The `toolset.Copy()` call creates targets that copy the source files into the output directory using pattern matching and replacement to generate the output filenames from the inputs.

### Simple C++ Project

A more interesting root build script is that for a simple C++ project build that compiles and links a static library and executable:

~~~lua
variant = variant or 'debug';

local toolset = require 'forge.cc' {
    identifier = 'cc_${platform}_${architecture}';
    platform = operating_system();
    architecture = 'x86_64';
    bin = root( ('%s/bin'):format(variant) );
    lib = root( ('%s/lib'):format(variant) );
    obj = root( ('%s/obj/cc_x86_64'):format(variant);
    include_directories = {
        root( 'src' );
    };
    library_directories = {
        root( ('%s/lib'):format(variant) );
    };
    assertions = variant ~= 'shipping';
    debug = variant ~= 'shipping';
};

toolset:all {
    'src/executable/all';
};

buildfile 'src/executable/executable.forge';
buildfile 'src/library/library.forge';
~~~

### Variant Builds

The immediately obvious difference is that the C++ project uses a variant build.  That is it defines several independent builds each with slightly different settings.

~~~lua
variant = variant or 'debug';
~~~

Typically the three supported variants are *debug*, *release*, and *shipping* but variants can be any useful combination of settings that you care to provide support for.

The `variant` variable must set before the `forge` module is required so that the file that Forge caches its build database to is stored within a directory named after the variant instead of the root directory.  This is important because the build database needs to be separate for each variant to work properly.

### Toolset

Toolset creation for the C++ project requires the C/C++ module `cc` that provides support for C/C++ compiler toolchains.  Settings are passed in a table although this time there are many more of them.

~~~lua
local cc = require 'forge.cc' {
    identifier = 'cc_${platform}_${architecture}';
    platform = operating_system();
    architecture = 'x86_64';
    bin = root( ('%s/bin'):format(variant) );
    lib = root( ('%s/lib'):format(variant) );
    obj = root( ('%s/obj/cc_x86_64'):format(variant);
    include_directories = {
        root( 'src' );
    };
    library_directories = {
        root( ('%s/lib'):format(variant) );
    };
    assertions = variant ~= 'shipping';
    debug = variant ~= 'shipping';
};
~~~

The `identifier` variable is interpolated and used to identify the toolset later when defining the dependency graph.  Other strings appearing in the settings are *not* interpolated implicitly.  See the section on buildfiles later as well as the `add_toolset()` and `toolsets()` functions in the reference section.

The `bin`, `lib`, and `obj` settings are directories specified per variant to keep variant compilations independent.  Keeping these files separate means there's no need to clean a previous build when building a different variant and switching variants won't cause a full rebuild.

The `bin`, `lib`, and `obj` settings have no special meaning in Forge.  They are passed through to be referenced again in the buildfiles that define the dependency graph.  Using other variables is as simple as adding them in *forge.lua* and referencing them from one or more buildfiles.

Other settings are used internally by the C/C++ module.  They provide control of features such as which architecture to build for, whether or not to generate debug information, which optimization level to use, etc.  Refer to the *cc* module reference for more details.

### Default Targets

Targets built when Forge is invoked from the root directory of the project are specified by making them dependencies of the special "all" target in the root directory through a call to `Toolset.all()` passing the paths to the targets to add as dependencies:

~~~lua
toolset:all {
    'src/executable/all';
};
~~~

### Buildfiles

The second step of the build definition is now missing from the root build script!  The files, dependencies, and actions are specified in separately loaded buildfiles that are covered in detail in the next section.

~~~lua
buildfile 'src/executable/executable.forge';
buildfile 'src/library/library.forge';
~~~

Buildfiles are still plain Lua scripts just like the root build script.  Moving the dependency graph definition into separate files allows the dependency graph to be composed from small scripts that then become reusable between different projects with different settings.
