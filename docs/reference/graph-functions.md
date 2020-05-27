---
layout: page
title: Graph Functions
parent: Reference
nav_order: 3
---

- TOC
{:toc}

## Overview

Forge allows direct manipulation of the dependency graph used to track targets and dependencies.  These functions are described here.

## Functions

### add_target

~~~lua
function add_target( id, [target_prototype] )
~~~

Create or return an existing target.

The `id` parameter specifies a target path that uniquely identifies the target to create or return.  If `id` specifies a relative path then it is considered relative to the current working directory.  If `id` is the empty string or nil then an anonymous target is created using a unique identifier to create a target in the current working directory.

The `target_prototype` parameter specifies the target prototype to use when creating the target.  It is an error for the same target to be created with more than one target prototype.  If the _target_prototype_ parameter is omitted or nil then a target without any target prototype is created.

**Parameters:**

- `id` the identifier of the target to create or find
- `target_prototype` the target prototype of the target to create (optional)

**Returns:**

The newly created or found target.

### add_target_prototype

~~~lua
function add_target_prototype( id )
~~~

Add a target prototype for use in this dependency graph.

The identifier is used only to describe the type of a target when displaying the dependencies or namespace for a dependency graph.  It should be unique enough to be useful for this purpose.  It doesn't necessarily need to be unique across the entire build.

**Parameters:**

- `id` the identifier of the target prototype to add

**Returns:**

The new target prototype.

### anonymous

~~~lua
function anonymous()
~~~

Generate an anonymous identifier that is unique within the current working directory.

**Returns:**

The next anonymous identifier for the current working directory.

### buildfile

~~~lua
function buildfile( path )
~~~

Load the buildfile at `path`.

Temporarily updates the working directory to be the directory containing the buildfile and then executes the buildfile.  Relative paths in the buildfile are then relative to the directory containing the buildfile by default.

The working directory is always restored after executing the buildfile.  It is good practice to make use of matched `pushd()` and `popd()` calls to restore the working directory at the end of the scope that requires it changing.  However this is not required for any reason other than keeping your buildfiles maintainable.

**Parameters:**

- `path` the path to the buildfile to load

**Returns:**

The number of errors that occured when loading the buildfile (e.g. returns 0 on success).

### clear

~~~lua
function clear()
~~~

Clear the dependency graph.

Removes all targets, target prototypes, and resets all anonymous counters.

The filename of the cache file is preserved so that the graph can still be saved without being loaded again (attempting to load a graph is the only way to set the filename that graph will be saved to).

**Returns:**

Nothing.

### current_buildfile

~~~lua
function current_buildfile()
~~~

Get the target representing the currently processed buildfile.

Using the target representing the currently processed buildfile is useful in expressing builds that depend on information in the buildfile that isn't expressed in toolset settings.

**Returns:**

The target that represents the currently processed buildfile (or the root build script).

### find_target

~~~lua
function find_target( id )
~~~

Find a target.

Find the target with a matching identifier.  If `id` is a relative path then it is treated as being relative to the current working directory.

**Parameters:**

- `id` the identifier of the target to find

**Returns:**

The target with a matching identifier or nil if no matching target was found.

### load_binary

~~~lua
function load_binary( path )
~~~

Load a previously saved dependency graph.

See `save_binary()` for more details.

**Parameters:**

- `path` the path to the cached dependency graph to load

**Returns:**

The target representing the cached dependency graph file

### save_binary

~~~lua
function save_binary( path )
~~~

Save the current dependency graph to `path`.

The boolean, numeric, string, and table values stored in the Lua tables representing targets are saved and reloaded as part of the cache.  This includes correctly persisting cyclic relationships between tables and tables that are recursively related to target tables.

Function and closure values are *not* saved.  This is generally not a problem because functions and closures are defined in target prototypes and the target prototype relationship of each target is preserved across a save and a load.

**Parameters:**

- `path` the path to save the current dependency graph to

**Returns:**

Nothing.

### postorder

~~~lua
function postorder( target, visit_function )
~~~

Perform a postorder traversal of the dependency graph.

Traverses the dependency graph calling the *visitor* function for each target visited.

Postorder traversal visits each target's dependencies before it visits that target.  This ordering ensures that dependencies are visited before the targets that depend on them.  This is the ordering needed to build dependencies before the targets that depend on them.

Targets are only visited once per traversal even if they are depended upon by more than one depending target.  The first visit is assumed to bring the target up to date and that subsequent visits are unnecessary.

Cyclic references are quietly ignored.

Report an errors in the visit by calling the standard Lua `error()` function and passing a string that describes the error that has occured.  This displays the error message on the console and causes the current visit to be counted as a failure.

Errors that occur while visiting a target mark the target and any dependent targets as failing.  The dependent targets are not visited (although an error message is displayed for each dependent target that is not visited).  Processing of other targets that aren't part of the same dependency chain continues.

The visit function accepts the target being visited as its only parameter e.g. `function visit_function( target )`.

For example the visit function used to implement the default *build* command:

~~~lua
-- Visit a target by calling a member function "build" if it
-- exists and setting that target's built flag to true if the
-- function returns with no errors.
local function build_visit( target )
    if target:outdated() then
        local build_function = target.build;
        if build_function then 
            local success, error_message = pcall(
                build_function,
                target.toolset,
                target
            );
            target:set_built( success );
            if not success then 
                clean_visit( target );
                assert( success, error_message );
            end
        else
            target:set_built( true );
        end
    end
end
~~~

**Parameters:**

- `target` the target to start the post-order traversal from
- `visit_function` the function to invoke to visit each target

**Returns:**

The number of targets that failed because they generated an error during their visit.

### print_dependencies

~~~lua
function print_dependencies( target )
~~~

Print the dependency tree of targets.

If `target` is nil then dependencies from the entire dependency graph are printed otherwise dependencies are recursively printed from `target`.

**Parameters:**

- `target` the target to print from or nil to print the entire graph

**Returns:**

Nothing.

### print_namespace

~~~lua
function print_namespace()
~~~

Print the namespace of targets in the dependency graph.

If `target` is nil then the namespace of the entire dependency graph is printed otherwise only targets that are descended from `target` are printed.

**Parameters:**

- `target` the target to print from or nil to print the entire graph

**Returns:**

Nothing.

### wait

~~~lua
function wait()
~~~

Wait for all currently executing processes to finish.

Not useful outside of waiting for processes executed to read back configuration settings to complete before attempting to use those settings.

**Returns:**

Nothing.

### working_directory

~~~lua
function working_directory()
~~~

Get the target representing the current working directory.

Depending on the target representing the current working directory or, generally any directory, is useful to force a rebuild when the contents of that directory change.

Any target whose dependencies are generated entirely or part through directory scanning via `ls()` or `find()` should also depend on the scanned directories.  The depending target will be outdated when files or directories are added to or removed from the scanned directories.

**Returns:**

The target that represents the current working directory.
