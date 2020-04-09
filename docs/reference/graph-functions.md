---
layout: page
title: Graph Functions
parent: Reference
nav_order: 5
---

- TOC
{:toc}

### add_target

~~~lua
function add_target( id, target_prototype, table )
~~~

Create or return an existing target.

The `id` parameter specifies a target path that uniquely identifies the target to create or return.  If `id` specifies a relative path then it is considered relative to the current working directory.  If `id` is the empty string or nil then an anonymous target is created using a unique identifier to create a target in the current working directory.

The _target_prototype_ parameter specifies the target prototype to use when creating the target.  It is an error for the same target to be created with more than one target prototype.  If the _target_prototype_ parameter is omitted or nil then a target without any target prototype is created.

The _table_ parameter specifies the table to use to represent the target in the Lua virtual machine.  If the _table_ parameter is omitted or nil then an empty table is created and used.

### add_target_prototype

~~~lua
function add_target_prototype()
~~~

Add a target prototype for use in this dependency graph.

### anonymous

~~~lua
function anonymous()
~~~

Generate an anonymous identifier that is unique within the current working directory.

### buildfile

~~~lua
function buildfile( path )
~~~

Load a Forge buildfile from *path*.

### clear

~~~lua
function clear()
~~~

Clear the dependency graph.  This removes all targets, target prototypes, and resets all anonymous counters.  The filename of the cache file is preserved so that the graph can still be saved without being loaded again (attempting to load a graph is the only way to set the filename that graph will be saved to).

### current_buildfile

~~~lua
function current_buildfile()
~~~

Return a target that represents the currently processed buildfile.  Add this target as a dependency of another target to rebuild the dependending target when the buildfile changes.

### find_target

~~~lua
function find_target( id )
~~~

Find the target whose identifier matches `id`.  If `id` is a relative path then it is treated as being relative to the current working directory.

### load_binary

~~~lua
function load_binary( path )
~~~

Load a previously saved dependency graph from *path*.  See `save_binary()` for details on which values are persisted.

### save_binary

~~~lua
function save_binary( path )
~~~

Save the current dependency graph to `path`.

The boolean, numeric, string, and table values stored in Lua tables are saved and reloaded as part of the cache.  This includes correctly persisting cyclic relationships between tables.

Function and closure values are *not* saved.  This is generally not a problem because functions and closures are defined in target prototypes and the target prototype relationship of each target is preserved across a save and a load.

### postorder

~~~lua
function postorder( visitor, target )
~~~

Perform a postorder traversal of targets calling the *visitor* function for each target visited.  Returns the number of visited targets that failed because they generated an error during their visitation.

Postorder traversal visits each target's dependencies before it visits that target.  This ordering ensures that dependencies are built before the targets that depend on them.

Targets are only visited once per traversal even if they are depended upon by more than one depending target.  It is assumed that the first visit brings the target up to date and that subsequent visits are unnecessary.

Cyclic references are quietly ignored.

Errors are reported by calling the standard Lua `error()` function passing a string that describes the error that has occured.  This displays the error message on the console and causes the current processing to fail.

Errors that occur while visiting a target mark the target and any dependent targets as failing.  The dependent targets are not visited (although an error message is displayed for each dependent target that is not visited).  Processing of other targets that aren't part of the same dependency chain continues.

### print_dependencies

~~~lua
function print_dependencies( target )
~~~

Print the dependency tree of targets.  If *target* is nil then dependencies from the entire dependency graph are printed otherwise dependencies are recursively printed from *target*.

### print_namespace

~~~lua
function print_namespace()
~~~

Print the namespace of targets in the dependency graph.  If *target* is nil then the namespace of the entire dependency graph is printed otherwise only targets that are descended from *target* are printed.

### wait

~~~lua
function wait()
~~~

Wait for all currently executing processes to finish.  Not generally useful outside of waiting for processes executed to read back configuration settings to complete before attempting to use those settings.

### working_directory

~~~lua
function working_directory()
~~~

Return a target representing the current working directory.  Depend on this target to force a rebuild when the contents of the working directory change, e.g. when you're building based on directory scanning.
