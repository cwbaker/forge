---
layout: page
title: Toolsets
parent: Getting Started
nav_order: 4
---

- TOC
{:toc}

Toolsets add support for external tools used during the build by providing target prototypes and configuration for use within toolsets used during a build.  For example the *cc* toolset supports C and C++ compilation, the experimental *android* toolset builds Android packages, and the experimental *xcode* toolset builds macOS and iOS app bundles.

### Using Toolsets

Toolsets are loaded as Lua modules using the `require()` function and then used to encapsulate target prototypes and settings for a particular tool or tools (e.g. a C/C++ compiler toolchain) for use in defining the build.

Calling a toolset prototype creates a new toolset initialized to use the tools provided by that toolset.  The single, optional parameter to this call is a table containing settings that override any defaults provided by the toolset.  The call is typically chained onto the require call and uses Lua's syntactic sugar to omit parentheses in function calls as follows:

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

More complex builds load toolset prototypes into variables and use them explicitly to create multiple toolsets to build for multiple platforms and/or architectures.

The toolset is returned from the call on the toolset prototype for use within the root build script.  This use is usually to provide some conditional configuration that is inconvenient to pass in settings when the toolset is initially created.

Toolsets that are created with `identifier` in their settings are added to the list of toolsets using the interpolated value of `identifier` as their unique identifier.  These toolsets are then available from within buildfiles using the `toolsets()` function.

See each individual toolset's documentation for exactly which target protoypes and settings are provided and expected.

### Writing Toolsets

Create a toolset prototype by calling `forge:ToolsetPrototype()`, passing a short identifier as the sole parameter.  This returns a table on which the functions provided by the toolset are defined.  Return the toolset prototype from the end of the Lua script that defines it.

~~~lua
local gcc = forge:ToolsetPrototype( 'gcc' );
-- ... function definitions skipped here for clarity
return gcc;
~~~

The identifier passed to `forge:ToolsetPrototype()` identifies the block of per-machine settings returned by the `Toolset.configure()` function and stored in the *local_settings.lua* file.  The identifier must be unique within your build and the configured values are shared between all variants.

Forge toolsets should provide one or more of the special functions `configure()`, `validate()`, and `initialize()` to interact with the configuration and initialization of toolsets.


