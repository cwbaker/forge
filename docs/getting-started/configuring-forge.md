---
layout: page
title: Configuring Forge
parent: Getting Started
nav_order: 2
---

- TOC
{:toc}

Configure Forge by editing the file named `forge.lua` at the root of a project's directory hierarchy.  This file, known as the build script, is the entry-point into a build.  The build script creates toolsets for the tools and settings used in the build and loads more scripts, known as buildfiles, that define which files are built.

A build script to build the classic "Hello World!" as a C++ library and executable might look like:

~~~lua
variant = variant or 'debug';

local forge = require 'forge';
forge:load( variant );

local toolset = forge.Toolset 'cc_${platform}_${architecture}' {
    platform = operating_system();
    architecture = 'x86_64';
    bin = root( ('%s/bin'):format(variant) );
    lib = root( ('%s/lib'):format(variant) );
    obj = root( ('%s/obj/cc_%s'):format(variant, architecture) );
    include_directories = {
        root( 'src' );
    };
    library_directories = {
        root( ('%s/lib'):format(variant) );
    };
    assertions = variant ~= 'shipping';
    debug = variant ~= 'shipping';
};

toolset:install( 'forge.cc' );

toolset:all {
    'src/executable/all';
};

buildfile 'src/executable/executable.forge';
buildfile 'src/library/library.forge';
~~~

## Syntax

The build script and buildfiles are written in the progamming language Lua.  For the most part Lua will be familiar to anyone familiar with another programming language but there are a few syntax features that will seem strange to those unfamiliar with Lua.  These are:

- Lua's associative arrays, known as tables, are used as objects and arrays.  Forge's source and this user guide use the term table for consistency and imply which usage is intended from context.

- Parentheses are optional for calls taking a literal string or table constructor as their sole argument.  The call `require 'forge'` is really `require('forge')`.

- The colon operator `:` is a method-call that passes the table on its left as an implicit argument.  The call `forge:load(variant)` is really `forge.load(forge, variant)`.

- Calls, usually using the colon operator and with parentheses omitted, are often chained together to give a declarative style as in the toolset creation above and especially in buildfiles.

A longer explanation of this syntax as it appears in Forge is provided by [Buildfile Syntax](../more-details/buildfile-syntax.md).  The full and official definition of the language appears in the [Lua 5.3 Reference Manual](https://www.lua.org/manual/5.3/).

## Command Line Variables

Variables set on the command line when `forge` is run are available as global variables from the build script and any other Lua scripts that it loads.  The most common and useful of these is `variant` which is used to generate independent builds with different settings.

~~~lua
variant = variant or 'debug';
~~~

Lua's or operator short circuiting is used to provide default values for variables that aren't set.  Default values should be set for all variables used by a build that might be set on the command line.

The three supported variants are *debug*, *release*, and *shipping* but variants can be any useful combination of settings that you care to provide support for.  The assignment to `assertions = variant ~= 'shipping'` enables asserts when the variant isn't `shipping` could be replaced with any arbitrary logic desired.  Forge itself is agnostic towards variants.

Forge is agnostic towards variables in general.  Default settings that might be based on the value of `variant` are always able to be overridden in the build script.  Other variables in a toolset are passed through to buildfiles and rules.

## Dependency Graph Caching

The dependency graph is cached to a file between runs.  This stores timestamps, hashes, and implicit dependencies used to determine which targets are outdated.  The call to `forge:load()` is made to set the directory that the dependency graph is saved to.  Using a different directory per variant keeps variants independent.

Any boolean, numeric, string, and table values stored in the Lua tables representing targets are saved and reloaded as part of the dependency graph.  This includes correctly persisting arbitrary, possibly cyclic, relationships between tables.

~~~lua
local forge = require 'forge';
forge:load( variant );
~~~

## Creating Toolsets

Toolsets are collections of tools and settings to apply when using, e.g. a C/C++ compiler toolchain providing compiler, archiver, and linker.  A toolset appears within Forge as a Lua object (known as a table) with attributes storing settings and methods to build files using the tools in the toolset.

Creating toolsets and settings are discussed here.  Using toolset methods to invoke tools is discussed in [Writing Buildfiles](writing-buildfiles.md).  Defining toolset methods to support new tools is dicussed in [Targets and Toolsets](targets-and-toolsets.md).

Create a toolset by calling `forge.Toolset` passing a tag for the toolset, and then calling the result of that passing a table containing settings for the toolset.  The tag is interpolated when the settings are provided, so variables like `platform` and `architecture` can provide values.

~~~lua
local cc = forge.Toolset 'cc_${platform}_${architecture}' {
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

The `bin`, `lib`, and `obj` settings are directories specified per variant to keep variant compilations independent.  Keeping these files separate means there's no need to clean a previous build when building a different variant and switching variants won't cause a full rebuild.

Other settings are used internally by the C/C++ module.  They provide control of features such as which architecture to build for, whether or not to generate debug information, which optimization level to use, etc.  Refer to the *cc* module reference for more details.

Note that paths appearing in the build script use `/` as a delimiter.  All paths appearing in the build script and buildfiles should use this portable form.  Convert paths to native format prior to passing them to external tools with the `native()` function if necessary.

The toolset's methods are added by installing modules, like `forge.cc` below.  Modules will provide default values for any settings that aren't already set in the toolset when they are installed.

~~~lua
cc:install( 'forge.cc' );
~~~

## Default Targets

Targets built when Forge is invoked from the root directory of the project are specified by making them dependencies of the special "all" target in the root directory through a call to `Toolset.all()` passing the paths to the targets to add as dependencies:

~~~lua
cc:all {
    'src/executable/all';
};
~~~

## Loading Buildfiles

The targets, dependencies, and actions that make up the dependency graph are specified in separately loaded buildfiles:

~~~lua
buildfile 'src/executable/executable.forge';
buildfile 'src/library/library.forge';
~~~

The `buildfile()` function temporarily updates the working directory to the directory containing the buildfile and then executes the buildfile.  This has the effect of making paths in buildfiles relative to the containing directory.
