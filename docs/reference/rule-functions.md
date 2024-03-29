---
layout: page
title: Rule Functions
parent: Reference
nav_order: 5
---

- TOC
{:toc}

## Overview

These functions create rules on which you can define functions to provide behavior for types of targets.

## Functions

### FileRule

~~~lua
function FileRule( identifier )
~~~

Create rules that build a single output file from zero or more input files with `FileRule()`.

`FileRule()` is a special short-hand that provides a create function on the rules that it returns.  That create function creates a target in the same way as the directory example, that is it adds the directory that contains the file as an ordering dependency.

### GroupRule

~~~lua
function GroupRule( identifier, replacement_modifier, pattern )
~~~

Create rules that generate an output file per input file using a single invocation of a tool to build multiple files at once.  For example Microsoft's Visual C++ compiler will compile multiple source files at once.

The `GroupRule()` is a special short-hand that provides a proxy create function that generates targets as dependencies are added in the build script.

For example the `Cxx` rule used by the Microsoft Visual C++ module to compile C++ is defined as follows:

~~~lua
local Cxx = GroupRule( 'Cxx', msvc.object_filename );
Cxx.language = 'c++';
Cxx.build = msvc.compile;
toolset.Cxx = Cxx;
~~~

### PatternRule

~~~lua
function PatternRule( identifier, replacement_modifier, pattern )
~~~

Create rules that generate an output file per input file using pattern matching and replacement to generate identifiers for the output files.

`PatternRule()` is a special short-hand that provides a proxy create function that generates output and input targets as dependencies are added in the build script.

The `Copy` rule provided by *Forge* is a very simple pattern-based rule that only defines a build action:

~~~lua
local Copy = PatternRule( 'Copy' );

function Copy.build( toolset, target )
    rm( target );
    cp( target, target:dependency() );
end

return Copy;
~~~

### Rule

~~~lua
function Rule( identifier )
~~~

Create a basic rule.  This creates a basic rule with no pre-defined behavior at all.  At least `Target.create()` and `Target.build()` need to be provided for the rule to be useful.

The `Directory` rule is a good example using `Rule()` that overrides the default behaviour for the create, build, and clean actions:

~~~lua
local Directory = Rule( 'Directory' );

function Directory.create( toolset, identifier )
    local target = toolset:Target( identifier, Directory );
    target:set_filename( target:path() );
    target:set_cleanable( true );
    return target;
end

function Directory.build( toolset, target )
    mkdir( target );
end

function Directory.clean( toolset, target )
end

return Directory;
~~~

The rule is created with a call to `Rule()` passing `'Directory'` as the identifier.  The identifier only provides debug information during the dump of the dependency graph and so only needs to be informative not unique.

The create function is called when the `Directory` rule is used to create a target, e.g. when `toolset:Directory(identifier)` is called from a buildfile.  The `toolset` and `identifier` arguments from this call are forwarded through to `Directory.create()` and the rule, `Directory`, is forwarded into the third parameter.

The build function is called when running a build and the directory is determined to be outdated.  This simply uses the built-in `mkdir()` function to create the directory.  The `toolset` parameter is the same toolset that was used to create the directory target.  The `target` is the target that represents the directory that needs to be built.

The clean function is called when running the clean command.  The clean function here does nothing because the default clean behavior of deleting a file doesn't work on the directory.  The directory could be removed here with a call to `rmdir()` but that is often problematic if the directory is not empty.

The `Directory` rule is returned at the end of the script.  This is the value that is returned from the `require()` call that is used to load the rule and store it into a toolset (e.g. `toolset.Directory = require 'forge.Directory';`).

Directories don't often appear directly in buildfiles.  Usually they are implicitly created and added as ordering dependencies of other built files.  That way any directories that files will be built into are created before files are built to them.

### ToolsetRule

~~~lua
function ToolsetRule( identifier )
~~~

Call `ToolsetRule()` to create a new Forge toolset rule.  This creates a table that is callable to create toolsets and is suitable for the `Toolset.initialize()`, `Toolset.configure()`, and `Toolset.validate()` functions to be defined on.

**Parameters:**

- `identifier` the toolset rule identifier (unique within build).

**Returns:**

A table with a metatable and `__call` metamethod that creates toolsets using this toolset rule.
