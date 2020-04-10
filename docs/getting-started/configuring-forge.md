---
layout: page
title: Configuring Forge
parent: Getting Started
nav_order: 2
---

- TOC
{:toc}

When invoked Forge searches from the current directory up to root directory looking for a file named *forge.lua*.  The *forge.lua* file found in the highest level directory is the root build script.  The directory containing the root build script becomes the root directory of the project.

The root build script is a plain Lua script.  When executed it defines the dependency graph describing the build.  Typically the root build script first creates toolsets to represent the tools and settings used in the build and then loads one or more buildfiles that use those toolsets to define the dependency graph.

Buildfiles are plain Lua scripts loaded with the `buildfile()` function.  They have the extension *.forge* and appear throughout directory hierarchy of a project.

### Root Build Script

A simple example root build script that builds a C++ static library and executable to print the classic "Hello World!":

~~~lua
variant = variant or 'debug';

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

cc:all {
    'src/executable/all';
};

buildfile 'src/executable/executable.forge';
buildfile 'src/library/library.forge';
~~~

### Command Line Variables

Variables set on the command line when *forge* was run are available as global variables.  The most common and useful of these is `variant` used to generate several independent builds with slightly different settings.

~~~lua
variant = variant or 'debug';
~~~

The or operator's short circuiting is used to provide default values for variables that aren't set.  Default values should be set for all variables used by a build that might be set on the command line.

The `variant` variable is special in that it must set before the `forge` module is required so that Forge will caches its build database to a directory named after the variant instead of the root directory.  This is important because the build database must be separate for each variant to work properly.  There are no other command line variables that Forge needs to be set before loading.

Typically the three supported variants are *debug*, *release*, and *shipping* but variants can be any useful combination of settings that you care to provide support for.

Forge is mostly agnostic towards variables (some exceptions being `variant` above and `identifier` mentioned in the following section).  This means that default settings are never based on `variant` or other variable.  Configuration based on variant happens entirely in the root build script or buildfiles.

### Toolsets

The toolset for the build is created by requiring the C/C++ module `cc` to retrieve a toolset prototype and then calling that to create the toolset.  Settings to override the defaults are passed in a table to the toolset creation call.

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

The `identifier` variable is special in that it is interpolated and used to identify the toolset later when defining the dependency graph.  The `identifier` variable must be set when creating a top-level toolset.  No other variables appearing in the settings are interpolated.  See the the `add_toolset()` and `toolsets()` functions in the reference section.

The `bin`, `lib`, and `obj` settings are directories specified per variant to keep variant compilations independent.  Keeping these files separate means there's no need to clean a previous build when building a different variant and switching variants won't cause a full rebuild.

Other settings are used internally by the C/C++ module.  They provide control of features such as which architecture to build for, whether or not to generate debug information, which optimization level to use, etc.  Refer to the *cc* module reference for more details.

Note that paths appearing in the root build script use `/` as a delimiter.  All paths appearing in the root build script and buildfiles should use this portable form.  Convert paths to native format prior to passing them to external tools with the `native()` function if necessary.

### Default Targets

Targets built when Forge is invoked from the root directory of the project are specified by making them dependencies of the special "all" target in the root directory through a call to `Toolset.all()` passing the paths to the targets to add as dependencies:

~~~lua
cc:all {
    'src/executable/all';
};
~~~

### Buildfiles

The targets, dependencies, and actions that make up the dependency graph are specified in separately loaded buildfiles:

~~~lua
buildfile 'src/executable/executable.forge';
buildfile 'src/library/library.forge';
~~~

The `buildfile()` function temporarily updates the working directory to the directory containing the buildfile and then executes the buildfile.  This has the effect of making paths in buildfiles relative to the containing directory.
