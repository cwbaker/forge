---
layout: page
title: Toolsets
parent: Getting Started
nav_order: 4
---

- TOC
{:toc}

Toolsets add support for external tools by providing target prototypes and settings that are used to define a dependency graph of targets that represents the build of a project.

For example the *cc* toolset supports C and C++ compilation, the experimental *android* toolset builds Android packages, and the experimental *xcode* toolset builds macOS and iOS app bundles.

### Using Toolsets

Load toolsets as plain Lua modules using `require()`.  The returned value is actually a toolset prototype that can be called to create toolsets for a particular tool (e.g. a C/C++ compiler toolchain) to define the build.

Create a toolset by calling a toolset prototype.  The single, optional parameter to this call is a table containing settings to override any defaults provided by the toolset.  The call is often chained onto the require call and uses Lua's syntactic sugar to omit parentheses in function calls as follows:

~~~lua
local cc = require 'forge.cc' {
    identifier = 'cc_${platform}_${architecture}';
    platform = operating_system();
    bin = root( ('%s/bin'):format(variant) );
    lib = root( ('%s/lib'):format(variant) );
    obj = root( ('%s/obj/cc_%s_x86_64'):format(variant) );
    -- ...
};
~~~

More complex builds may load toolset prototypes into variables and use them explicitly to create toolsets to build for multiple platforms and/or architectures.

Toolsets that are created with `identifier` in their settings are added to the list of toolsets using the interpolated value of `identifier` as their unique identifier.  These toolsets are then available from within buildfiles using the `toolsets()` function.

The toolset is also returned from the toolset prototype call for use within the root build script.  Usually this is to provide conditional configuration that is inconvenient to pass in the settings.

See each individual toolset's documentation for exactly which target protoypes and settings are provided and expected.

### Writing Toolsets

Create a toolset prototype by calling `forge:ToolsetPrototype()`, passing a short identifier as the sole parameter.  This returns a table on which to define the functions provided by the toolset.  Return the toolset prototype from the end of the Lua script.

Any functions that are needed during definition of the dependency graph can be defined on the toolset prototype.  They will then be available from the toolset from the root build script and any loaded buildfiles.

Three functions that can be overridden to have the toolset prototype configure and initialize toolsets that are created from it are `Toolset.configure()`, `Toolset.validate()`, and `Toolset.initialize()`.

`Toolset.configure()` is called only when a toolset is created and there isn't a field in the local settings for the target prototype.  The local settings field is named after the short identifier that is passed to create the target prototype.  The call to `Toolset.configure()` is expected to perform one-off auto-detection of installed tools.

`Toolset.validate()` is called for every toolset that is created.  It should return true when the configuration in local settings is still correct.  Checking that any configured paths to tools are still valid is usually sufficient.  Returning false will quietly ignore the rest of the toolset creation.  If lacking the toolset is a fatal error then this function should assert or raise an error.

`Toolset.initialize()` is called for every toolset that is created and that successfully configures and validates.  It should setup target prototypes and any settings that need to be setup dynamically at runtime (e.g. variant dependent settings not suitable for storing in local settings).

The identifier passed to `forge:ToolsetPrototype()` identifies the block of per-machine settings returned by the `Toolset.configure()` function and stored in the *local_settings.lua* file.  The identifier must be unique within your build.  The configured values are shared between all variants; values that are different between variants should be computed in `Toolset.initialize()` which runs for each toolset in each build.
