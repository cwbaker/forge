---
layout: page
title: Graph Functions
parent: Reference
nav_order: 3
---

- TOC
{:toc}

## Overview

Forge's core data structure is a dependency graph.  The nodes and connections in the graph are targets and the dependencies between them respectively.

Targets exist in a hierarchical namespace with POSIX path like semantics.  Each target has an identifier, a parent, and zero or more children.  A target path consists of one or more identifiers separated by the "/" character.  The special elements "." and ".." refer to the current working directory and parent of the working directory respectively.  Relative target paths are relative to the current working directory.

Add or find targets in the dependency graph with the `add_target()` and `find_target()` functions.  When no existing target is found `add_target()` function will add one and return it while `find_target()` will simply return nil.  Buildfiles usually call domain specific language functions like `Executable`, `StaticLibrary`, etc to create targets rather than calling `add_target()` and `find_target()` directly.

Add rules with the `add_rule()` function.  Rules are usually created through the higher level functions `Rule()`, `FileRule()`, `PatternRule()`, and `GroupRule()`.

Run a build by visiting the targets in a post-order traversal with `postorder()`.  This visits and builds all dependencies before the targets that depend on them.  Each target is visited by calling a short Lua script that typically calls a function like `build` or `clean` defined on the rule of each visited target.

Define the dependency graph by loading buildfiles with `buildfile()`.  This sets the working directory to the directory containing the buildfile then loads and executes the buildfile as a Lua script.

Load, save, and clear the dependency graph with `load_binary()`, `save_binary()`, and `clear()`.  These functions save target metadata and implicit dependencies that are required for builds to run correctly.

Debug builds by printing the dependency graph with `print_dependencies()` function or the target namespace with the `print_namespace()`.  The dependency information is useful when determining why targets are being built when they shouldn't and vice versa.

## Functions

### add_target

~~~lua
function add_target( id, [rule] )
~~~

Find an existing or create a new target.

The `id` parameter specifies a target path that uniquely identifies the target to find or create.  Relative paths are considered relative to the current working directory.  If `id` is the empty string or nil then an anonymous target is created using an identifier unique within the current working directory.

The `rule` parameter specifies the rule to use when creating the target.  It is an error for the same target to be created with more than one rule.  If `rule` is omitted or nil then a target without any rule is created.

**Parameters:**

- `id` the identifier of the target to find or create (optional)
- `rule` the rule of the target to create (optional)

**Returns:**

The existing or newly created target identified by `id`. 

### add_rule

~~~lua
function add_rule( id )
~~~

Add a rule for use in this dependency graph.

The identifier is used only to describe the type of a target when displaying the dependencies or namespace for a dependency graph.  It should be unique enough to be useful for this purpose.  It doesn't necessarily need to be unique across the entire build.

**Parameters:**

- `id` the identifier of the rule to add

**Returns:**

The new rule.

### anonymous

~~~lua
function anonymous()
~~~

Generate an anonymous identifier that is unique within the current working directory.

Anonymous targets are created by passing an empty string as their identifier.  This creates a target using a uniquely generated identifier.  Use anonymous targets to create targets that don't bind to files and don't need to be referred to by identifier.  This is useful for grouping targets that need to be passed to a single invocation of a tool (e.g. the Microsoft C++ compiler can compile multiple source files in one invocation).

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

The number of errors that occurred loading the buildfile (e.g. returns 0 on success).

### clear

~~~lua
function clear()
~~~

Clear the dependency graph.

Removes all targets, rules, and resets all anonymous counters.

The filename of the cache file is preserved so that the graph can still be saved without being loaded again (attempting to load a graph is the only way to set the filename that graph will be saved to).

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

Function and closure values are *not* saved.  This is generally not a problem because functions and closures are defined in rules and the rule relationship of each target is preserved across a save and a load.

**Parameters:**

- `path` the path to save the current dependency graph to

### postorder

~~~lua
function postorder( target, visit_function )
~~~

Perform a postorder traversal of the dependency graph.

Traverses the dependency graph calling the *visitor* function for each target visited.

Postorder traversal visits each target's dependencies before it visits that target.  This ordering ensures that dependencies are visited before the targets that depend on them.  This is the ordering needed to build dependencies before the targets that depend on them.

Targets are only visited once per traversal even if they are depended upon by more than one depending target.  The first visit is assumed to bring the target up to date and that subsequent visits are unnecessary.

Cyclic references are quietly ignored.

The working directory of a target is whatever the current working directory 
was when the target was created.  Usually this is the directory that contains
the buildfile that indirectly constructed the target (for targets constructed 
in a buildfile) or the current working directory at the time the target was 
created (for targets created at other times).

Report any errors during the visit by calling the standard Lua `error()` function and passing a string that describes the error that has occurred.  This displays the error message on the console and causes the current visit to be counted as a failure.

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

### preorder

~~~lua
function preorder( target, visit_function )
~~~

Perform a preorder traversal of the dependency graph calling *visit_function* for each visited target.

Preorder traversal visits each target before visiting its dependencies.  Targets are visited once per traversal even if they are depended on by more than one target.  Cyclic references are quietly ignored.

The preorder traversal can be pruned by calling the `prune()` function from within the *visit_function* call.  This stops children and descendents of the current target being visited and the traversal continues at the next sibling target.

**Parameters:**

- `target` the target to start the traversal from
- `visit_function` the function called to visit each target

**Returns:**

The number of targets that failed because they generated an error during their visit.

### prune

~~~lua
function prune()
~~~

Prune preorder traversal at the currently visited target.

Stops children and descendents of the target being visited in a preorder pass and continues the traversal at the next sibling target.  Only valid to call from within the visit function of a preorder traversal.

### print_dependencies

~~~lua
function print_dependencies( target )
~~~

Print the dependency tree of targets.

If `target` is nil then dependencies from the entire dependency graph are printed otherwise dependencies are recursively printed from `target`.

**Parameters:**

- `target` the target to print from or nil to print the entire graph

### print_namespace

~~~lua
function print_namespace()
~~~

Print the namespace of targets in the dependency graph.

If `target` is nil then the namespace of the entire dependency graph is printed otherwise only targets that are descended from `target` are printed.

**Parameters:**

- `target` the target to print from or nil to print the entire graph

### working_directory

~~~lua
function working_directory()
~~~

Get the target representing the current working directory.

Depending on the target representing the current working directory is useful to force a rebuild when the contents of that directory change.  This is true of depending on directories in general.

Any target whose dependencies are generated entirely or in part by directory scanning with `ls()` or `find()` should depend on the scanned directories.  The depending target will become outdated when files or directories are added to or removed from the scanned directories.

**Returns:**

The target representing the current working directory.
