---
layout: page
title: Modules
parent: Getting Started
nav_order: 4
---

- TOC
{:toc}

Modules add support for external tools used during the build by providing target prototypes and configuration for use within toolsets used during a build.  For example the *cc* module supports C and C++ compilation, the experimental *android* module builds Android packages, and the experimental *xcode* module builds macOS and iOS app bundles.

### Using Modules

Modules are loaded with Lua's `require()` function and then used to add target prototypes, configuration, and default settings to toolsets that are then used in a build.

Calling a module creates a new toolset initialized to use the tools provided by that module.  The single, optional parameter to this call is a table containing settings that override any defaults provided by the module.  The call is typically chained onto the require call using Lua's syntactic sugar to omit parentheses in function calls as follows:

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

More complex builds load modules into variables and then use them explicitly to register multiple toolsets to build for multiple platforms and/or architectures.

The toolset is returned from the call on the module for use within the root build script.  This use is usually to provide some conditional configuration that is inconvenient to pass in settings when the toolset is initially created.

Toolsets that are created with `identifier` in their settings are added to the list of toolsets using the interpolated value of `identifier` as their unique identifier.  These toolsets are then available from within buildfiles using the `toolsets()` function.

See each individual module's documentation for exactly which target protoypes, configuration, and settings are provided and expected.

### Writing Modules

Create a Forge module by calling `forge:Module()`, passing a short identifier as the sole parameter.  This returns a table that the functions provided by the module are defined.  Return the module table from the end of the Lua script that defines the module.

~~~lua
local gcc = forge:Module( 'gcc' );
-- ... function definitions skipped here for clarity
return gcc;
~~~

The identifier passed to `forge:Module()` identifies the block of per-machine settings returned by the module's `configure()` function and stored in the *local_settings.lua* file.  The identifier must be unique within your build.

Forge modules should provide one or more of the special functions `configure()`, `validate()`, and `initialize()` to interact with the configuration and initialization of toolsets.

Modules that provide arbitrary functions without needing to interact with Forge's configuration and initialization can simply be defined and used as with any normal Lua module.  Only modules that need to detect per-machine configuration or install themselves into a toolset need to be explicitly created as Forge modules.

