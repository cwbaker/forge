---
layout: page
title: Toolset
parent: Reference
nav_order: 8
---

- TOC
{:toc}

## Construction

~~~lua
function Toolset( values )
~~~

Calling a toolset prototype creates and registers a new toolset with behavior defined by that toolset prototype.

**Parameters:**

- `toolset_prototype` the toolset prototype being called to create a toolset
- `values` settings used to override toolset defaults

**Returns:**

The created toolset or nothing if the toolset prototype fails to configure, validate, or initialize (see below).

## Overrides

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

## Methods

### create

~~~lua
function Toolset.create( toolset, settings )
~~~

Create a new toolset optionally initializing its settings to `settings`.

### clone

~~~lua
function Toolset.clone( toolset, settings )
~~~

Create a new toolset initializing its settings by copying settings from `toolset` and `settings`.  Settings are copied from `toolset` and then `settings` so that values from the latter take precedence.

### inherit

~~~lua
function Toolset.inherit( toolset, settings )
~~~

Create a new toolset initializing its settings by inheriting settings from `toolset` and overriding by `settings`.  Settings are inherited from `toolset` and overridden by values in `settings`.

### append

~~~lua
function Toolset.append( toolset, values, value )
~~~

Append `value` to `values`.  If `value` is a table then all of its array elements are appended to `values`.  Otherwise `value` itself is appended to `values`.

### merge

~~~lua
function Toolset.merge( toolset, destination, source )
~~~

Merge fields with string keys from `source` to `destination`.

### defaults

~~~lua
function Toolset.defaults( toolset, destination, source )
~~~

Set fields in that aren't already set in `destination` to values from `source`.

### copy_settings

~~~lua
function Toolset.copy_settings( toolset, destination, source )
~~~

Recursively copy fields from `source` to `destination`.

### interpolate

~~~lua
function Toolset.interpolate( toolset, template, variables )
~~~

Return `template` substituted with values from `variables`, the settings of `toolset`, the fields and functions of `toolset`, global variables, and finally environment variables.

### dependencies_filter

~~~lua
function Toolset.dependencies_filter( toolset, target )
~~~

Return a dependencies filter to add dependencies to `target`.  The returned function can be passed to `execute()` to automatically detect and add implicit dependencies to `target` when it is built.

### filenames_filter

~~~lua
function Toolset.dependencies_filter( toolset, target )
~~~

Return a filename filter to add output filenames and implicit dependencies to `target`.  The returned function can be passed to `execute()` to automatically detect and add both output filenames and implicit dependencies to `target` when it is built.

### platform_matches

~~~lua
function Toolset.platform_matches( toolset, pattern )
~~~

Return true if the `platform` field of `toolset` settings matches `pattern`.

### cpdir

~~~lua
function Toolset.cpdir( toolset, destination, source, variables )
~~~

Recursively copy files from `source` to `destination`.  Both `source` and `destination` are interpolated before use with the variables optionally passed in `variables`.  Values for interpolation are looked up as per `Toolset.interpolate()`.

### which

~~~lua
function Toolset.which( toolset, filename, paths )
~~~

Return the first file named `filename` that exists at a directory listed in `paths` or nothing if no such file is found.

The `filename` variable is interpolated and any variables referenced with the `${}` syntax are expanded.  See `Toolset:interpolate()` for details.

Searching is not performed when `filename` is an absolute path.  In this case `filename` is returned immediately if it names an existing file.  Otherwise if there is no such file then nothing is returned.

The `paths` variable can be a string containing a `:` or `;` delimited list of paths or a table containing those paths.  If `paths` is nil then its default value is set to that returned by `os.getenv('PATH')`.

### all

~~~lua
function Toolset.all( toolset, dependencies )
~~~

Return the *all* target for the current working directory adding any targets that are passed in `dependencies` as dependencies.

### File

~~~lua
function Toolset.File( toolset, identifier, target_prototype )
~~~

Define a generated file.  It doesn't generally appear in buildfiles but is used internally to expand other targets during the build.

### SourceFile

~~~lua
function Toolset.SourceFile( toolset, identifier )
~~~

Defines a source file that must exist.  It doesn't generally appear in buildfiles but is used internally to expand other targets during the build.

### Target

~~~lua
function Toolset.Target( toolset, identifier, target_prototype )
~~~

### Directory

~~~lua
function Toolset.Directory( toolset, identifier )
~~~

Define a directory.  It doesn't generally appear in buildfiles but is used internally as an ordering dependency of all generated to files to ensure that directories are created before output files are written to them.

### Copy

~~~lua
function Toolset.Copy( toolset, pattern )
~~~

Define a copy target that copies files.

### CopyDirectory

~~~lua
function Toolset.CopyDirectory( toolset, identifier )
~~~

Define a copy directory target that recursively copies a directory hierarchy.
