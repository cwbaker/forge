---
layout: page
title: Directory Functions
parent: Reference
nav_order: 1
---

- TOC
{:toc}

## Overview

Forge maintains a [working directory](https://en.wikipedia.org/wiki/Working_directory) stack.  The directory at the top of this stack is the current working directory that relative paths are relative to and the working directory for processes spawned by `execute()`.

The `buildfile()` call updates the working directory to the directory containing the buildfile being loaded.  This has the effect of making any relative paths in buildfiles relative to the directory containing those buildfiles.  This keeps paths within buildfiles short and is generally the expected and desired behavior.

The `postorder()` call updates the working directory to the working directory that was set at the time that the visited target was defined.  This has the effect of making any relative paths used in the `Target.build()` and `Target.clean()` overrides relative to the working directory set when the relevant target was created.  Usually this is the working directory of the buildfile that created those targets.

Change the working directory explicitly by calling `cd()`, `pushd()`, and `popd()` from within build scripts and buildfiles.  This overrides the default behavior provided by `buildfile()` and `postorder()` to allow for situations where source files referenced in a buildfile in a different directory or external tools that require relative paths specified from a different base.

The working directory and any working directories pushed by `pushd()` are reset at the end of any `buildfile()` call and after visiting each target in a `postorder()` call.  Working directories are independent in these situations.  The working directory and working directory stack don't need to be restored manually.  However it is good practice to balance calls to `pushd()` with matching calls to `popd()` to be clear about the scope of working directory changes and make future maintenance easier.

Forge's working directory is *not* the process working directory of of the `forge` process itself.  This difference is only apparent when loading Lua modules with native implementations that use underlying system calls like `getcwd()`, `GetCurrentDirectory()`, etc.

## Functions

### cd 

~~~lua
function cd( path )
~~~

Change the directory on top of the working directory stack so that it becomes the current working directory.  If `path` is relative it is considered relative to the current working directory.

**Parameters:**

- `path` the path to set the working directory to.

**Returns:**

Nothing.

### popd

~~~lua
function popd()
~~~

Pop the current working directory and restore the working directory to the working directory saved at the time of the most recent call to `pushd()`.  If there is only one working directory on the directory stack then this function silently does nothing.

**Returns:**

Nothing.

### pushd

~~~lua
function pushd( path )
~~~

Push a new directory onto the working directory stack so that it becomes the current working directory.  If `path` is relative it is considered relative to the current working directory.

**Parameters:**

- `path` the path to set the working directory to.

**Returns:**

Nothing.

### pwd

~~~lua
function pwd()
~~~

Get the current working directory.

**Returns:**

The absolute path to Forge's current working directory.
