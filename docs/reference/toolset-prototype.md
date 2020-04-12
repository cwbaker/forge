---
layout: page
title: Toolset Prototype
parent: Reference
nav_order: 11
---

- TOC
{:toc}

## Overview

## Functions

### ToolsetPrototype

~~~lua
function forge:ToolsetPrototype( identifier )
~~~

Call `forge:ToolsetPrototype()` to create a new Forge toolset prototype.  This creates a table that is callable to create toolsets and is suitable for the `Toolset.initialize()`, `Toolset.configure()`, and `Toolset.validate()` functions to be defined on.

**Parameters:**

- `identifier` the toolset prototype identifier (unique within build).

**Returns:**

A table with a metatable and `__call` metamethod that creates toolsets using this toolset prototype.

### Constructor

~~~lua
function Toolset( values )
~~~

Calling a toolset prototype creates and registers a new toolset.

**Parameters:**

- `toolset_prototype` the toolset prototype being called to create a toolset
- `values` settings used to override toolset defaults

**Returns:**

The created toolset or nothing if the toolset prototype fails to configure, validate, or initialize (see below).

### configure

~~~lua
function Toolset.configure( toolset, toolset_settings )
~~~

The `configure()` function is used to generate per-machine settings for the toolset that are changed infrequently (e.g. detecting the paths to executables used by the build).

The `configure()` function is free to execute whatever Lua script and/or external processes it needs to detect its required settings.  For example the *msvc* toolset will look for Microsoft Visual C++ compilers to be installed by executing the *vswhere* tool and parsing its output.

The configuration returned from the `configure()` function is written to the *local_settings.lua* file in the project root directory.  This file should be ignored and left out of version control.

**Parameters:**

- `toolset` the first toolset needing to configure the toolset
- `toolset_settings` settings used to override toolset defaults

**Returns:**

A table containing per-machine settings for the toolset.

### install

~~~lua
function Toolset.install( toolset )
~~~

The `install()` function explicitly installs the target prototypes and default settings for the toolset.

**Parameters:**

- `toolset` the toolset to install target prototypes and settings in

**Returns:**

Nothing.

### initialize

~~~lua
function Toolset.initialize( toolset )
~~~

The `initialize()` function initializes a toolset for use.  Typically this means adding the appropriate target prototypes and default settings to the toolset.  Initialization runs at least once per toolset per build.

**Parameters:**

- `toolset` the toolset to initialize

**Returns:**

True if initialization was successful otherwise false.

### validate

~~~lua
function Toolset.validate( toolset, toolset_settings )
~~~

The `validate()` function returns true if previously generated per-machine configuration for the module is still valid or false if it is invalid.

The `validate()` function is called for every build.  Each toolset is validated before it is initialized to make sure that the tools that it expects are still available in the directories that were configured in the configuration step.

Toolsets that fail validation typically don't register any toolsets with Forge and thus anything built with them quietly fails.  This behaviour can be overridden by asserting that particular toolsets are available or that particular module validation is successful by calling `validate()` explicitly and that it returns true.

**Parameters:**

- `toolset` the toolset to validate
- `toolset_settings` the settings to validate

**Returns:**

True if the settings are valid otherwise false.
