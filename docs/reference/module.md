---
layout: page
title: Module
parent: Reference
nav_order: 10
---

- TOC
{:toc}

### Module

~~~lua
function forge:Module( identifier )
~~~

Call `forge:Module()` to create a new Forge module.  This creates a table that is callable to create toolsets and is suitable for the `initialize()`, `configure()`, and `validate()` functions to be defined in.

**Parameters:**

- `identifier` the module identifier (unique within build).

**Returns:**

A table with a metatable and `__call` metamethod that implements functionality for a Forge module.

### Constructor

~~~lua
function Module( module, settings )
~~~

Calling a module creates and registers a new toolset initialized to use the toolset provided by the module.

**Parameters:**

- `module` the module being called to create a toolset
- `settings` settings used to override module defaults

**Returns:**

The created toolset or nothing if the module failed to configure, validate, or initialize (see below).

### configure

~~~lua
function Module.configure( toolset, module_settings )
~~~

The `configure()` function is used to generate per-machine settings for the module that is changed infrequently (e.g. detecting the paths to executables used by the build).

The `configure()` function is free to execute whatever Lua script and/or external processes it needs to detect its required settings.  For example the *msvc* module will look for Microsoft Visual C++ compilers to be installed by executing the *vswhere* tool and parsing its output.

The configuration returned from the `configure()` function are written to the *local_settings.lua* file in the project root directory.  This file should be ignored and left out of version control.

**Parameters:**

- `toolset` the first toolset needing to configure the module
- `module_settings` settings used to override module defaults

**Returns:**

A table containing per-machine settings for the module.

### install

~~~lua
function Module.install( module, toolset )
~~~

The `install()` function explicitly installs the the module with a toolset.  This approach is used in builds that require reusing the same module multiple times with different settings to build for multiple platforms and/or architectures.

**Parameters:**

- `module` the module to install
- `toolset` the toolset to install the module to

**Returns:**

Nothing.

### initialize

~~~lua
function Module.initialize( toolset )
~~~

The `initialize()` function initializes a toolset to be used with the tool(s) provided by the module.  Typically this means adding the appropriate target prototypes and default settings to the toolset.

Initialization runs at least once per module per build and possibly more than once per module in the case of a modules that register multiple toolsets, e.g. to support building multiple architectures.

**Parameters:**

- `toolset` the toolset to install the module to

**Returns:**

True if initialization was successful otherwise false.

### validate

~~~lua
function Module.validate( toolset, module_settings )
~~~

The `validate()` function returns true if previously generated per-machine configuration for the module is still valid or false if it is invalid.

The `validate()` function is called for every build.  Each module is validated before it is installed into a toolset to make sure that the tools that are expected are still available in the directories that were configured in the configuration step.

Modules that fail validation typically don't register any toolsets with Forge and thus anything built with them quietly fails.  This behaviour can be overridden by asserting that particular toolsets are available or that particular module validation is successful by calling `validate()` explicitly and that it returns true.

**Parameters:**

- `toolset` the toolset to validate
- `module_settings` the settings to validate

**Returns:**

True if the settings are valid otherwise false.
