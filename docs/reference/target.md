---
layout: page
title: Target
parent: Reference
nav_order: 7
---

- TOC
{:toc}

## Overview

Targets are the nodes in the dependency graph.  Typically each target represents a single file but targets can also represent a single directory, multiple files, multiple directories, and even groups of targets that are useful to build together but aren't directly related to anything on disk (e.g. the *all* targets in the root and other directories).

Create targets by passing an identifier to one of the target creation functions `forge:SourceFile()`, `forge:File()`, or `forge:Target()` to create targets representing source files, intermediate files, or targets that will be defined later.

The identifier used to create a target is considered a relative path to the target to create.  The relative paths is considered relative to Forge's notion of the current working directory set to the path of the file currently being executed by Forge (e.g. the root build script of the currently executing buildfile).

Targets exist in a hierarchical namespace with the same semantics as operating system paths.  Targets have an identifier, a parent, and zero or more children.  Targets are referred to via `/` delimited paths by using `.` and `..` to refer to the current target and parent target respectively.

The hierarchical namespace is similar but not identical to the hierarchy of directories and files on disk.  Targets are often bound to files on disk in quite separate locations from their location in the target namespace or even not bound to files at all.

Referring to a target with the same identifier always returns the same target.  This allows late binding in the case of depending targets that refer to their dependencies before those dependencies are fully defined.

Anonymous targets can be created using the `anonymous()` function to generate a unique identifier for the target instead of passing a specific identifier.

The order in which targets are built is specified implicitly by the dependencies between targets.  Dependencies of each target are built before the target itself.  Cyclic dependencies are lazily detected at build-time and generate a warning (but not an error).

Target paths and system paths always use `/` as a delimiter.  Convert paths to native paths prior to passing them to external tools if needed with the `native()` function.

Although targets usually bind to a single file it is possible to create targets with no associated file (i.e. Make's phony targets) or with multiple associated files (i.e. tools that generate multiple output files in one invocation).

The string passed to create a target is that target's identifier or is used to generate the identifier for multiple targets.  A target's identifier is often different from its filename.  In the example above the static library target's identifier is `hello_world` but it will build to a file named *libhello_world.a* or *hello_world.lib* when built with Unix and Windows toolchains respectively.

Targets without rules are valid.  These targets are typically source files that aren't updated by the build system and but still need to be tracked in order to determine when intermediate files that depend on them are outdated.

Create dependencies conditionally by storing a target in a local variable and making further dependency calls on it.  Alternatively passing the same identifier (after string substitution) will return an already created target.  So really all that needs to happen is the dependency call is made conditionally onto the same target.

Create dependencies dynamically using the same pattern as for conditional dependencies of storing a target in a local variable or reusing an existing identifier.  A typical example of dynamic dependencies is looping over all toolsets to build executables to be combined into a macOS or iOS fat binary.

## Overrides

### create

~~~lua
function Target.create( toolset, identifier, rule )
~~~

The `create()` function is called whenever a target for that rule is created by calling the rule table returned from `Rule()`.

The parameters passed are the toolset that the target is being created with, the identifier that was specified, and the rule that was used to create the target (possibly nil).

### depend

~~~lua
function Target.depend( toolset, target, dependencies )
~~~

The `depend()` function is called whenever a call is made on a target for that rule.  Typically this captures the use of calls of the form *target* **{** *dependencies* **}** to add dependencies or further define a target.

The parameters passed are the toolset that the target was created with, the target itself, and the dependencies that were passed in.

### build

~~~lua
function Target.build( toolset, target )
~~~

The `build()` function is called whenever an outdated target is visited as part of a build traversal.  The function should carry out whatever actions are necessary to build the file(s) that it represents up to date.

The parameters passed in are the toolset that the target was created with and the target itself.

### clean

~~~lua
function Target.clean( toolset, target )
~~~

The `clean()` function is called whenever a target is visited as part of a clean traversal.  The function should carry out whatever actions are necessary to remove files that were generated during a build traversal.

Default behavior when visiting a cleanable target is to remove any files that the target is bound to.  Custom clean behavior is only needed if removing all of the built files is not desired.

The parameters passed in are the toolset that the target was created with and the target itself.

## Methods

### id

~~~lua
function Target.id( target )
~~~

Return the identifier of this target.

### path

~~~lua
function Target.path( target )
~~~

Return the full path of this target.

### branch

~~~lua
function Target.branch( target )
~~~

Return the directory part of the path of this target.

### rule

~~~lua
function Target.rule( target )
~~~

Return the rule for this target or nil for target's without rules.

### set_cleanable

~~~lua
function Target.set_cleanable( target, cleanable )
~~~

Set whether or not `target` should be automatically cleaned.  This has no effect on whether or not a clean function is called during a traversal simply whether or not the target will be automatically deleted if no explicit clean function is provided.

### cleanable

~~~lua
function Target.cleanable( target )
~~~

Return true if `target` is cleanable otherwise false.

### set_built

~~~lua
function Target.set_built( target, built )
~~~

Set whether or not `target` is considered to have been built successfully.

### built

~~~lua
function Target.built( target )
~~~

Return true if `target` has been built successfully at least once.

### timestamp

~~~lua
function Target.timestamp( target )
~~~

Return the timestamp of `target`.

If `target` isn't bound to a file then the last write time is always the beginning of the epoch - January 1st, 1970, 00:00 GMT.  Because this is the oldest possible timestamp this will leave unbound targets always needing to be updated.

### last_write_time

~~~lua
function Target.last_write_time( target )
~~~

Return the last write time of `target`.

### outdated

~~~lua
function Target.outdated( target )
~~~

Returns true if `target` is outdated otherwise false.

### add_filename

~~~lua
function Target.add_filename( target, filename )
~~~

Append `filename` to the list of filenames that `target` binds to.

### set_filename

~~~lua
function Target.set_filename( target, filename, index )
~~~

Set filename at `index` of this `target` to `filename`.  This is the name of the file that this target will attempt to bind to during the bind traversal.

### clear_filenames

~~~lua
function Target.clear_filenames( target )
~~~

Clear the list of filenames that `target` binds to.

### filename

~~~lua
function Target.filename( target, index )
~~~

Get the filename at `index` for this `target`.  Return the filename of this target or an empty string if `target` hasn't been bound to a file or `index` is greater than the number of files to bind for `target`.  The `index` parameter is optional and defaults to 1 (the first filename) if not specified.

### filenames

~~~lua
function Target.filenames( target, start, finish )
~~~

Iterate over the filenames of `target` from `start` to `finish`.  The `start` and `finish` parameters are optional and default to 1 and `INT_MAX` respectively to give the effect of iterating over all filenames of `target`.

### directory

~~~lua
function Target.directory( target, n )
~~~

Get the directory portion of the `n`th filename that `target` will bind to.  Return the empty string if `n` is greater than the number of files to bind for `target`.

### set_working_directory

~~~lua
function Target.set_working_directory( target, working_directory )
~~~

Set the working directory of `target` to the target `working_directory` or to the root directory of its containing graph if `working_directory` is nil.

The working directory is the target that specifies the directory that files specified in scripts for the target are relative to by default and that any commands executed by the target's script functions are started in.  If a target has no working directory then the root target is used instead.

### add_dependency

~~~lua
function Target.add_dependency( target, dependency )
~~~

Add `dependency` as an explicit dependency of `target`.

If `dependency` is null or the same as `target` then this function quietly does nothing.

If `dependency` is already an implicit, ordering, or passive dependency of `target` then it is made the last explicit dependency of `target`.  That is the dependency is removed and added as the last explicit dependency.

Cyclic dependencies are not valid but are not reported here.  Cyclic dependencies are detected during traversals and silently ignored.

### remove_dependency

~~~lua
function Target.remove_dependency( target, dependency )
~~~

Remove `dependency` as a dependency of `target`.  Removes the dependency regardless of whether that dependency is an explicit, implicit, ordering, or passive dependency.

### add_implicit_dependency

~~~lua
function Target.add_implicit_dependency( target, dependency )
~~~

Add `dependency` as an implicit dependency of `target`.

If `dependency` is null, the same as `target`, anonymous, or already an explicit dependency of `target` then this function quietly does nothing.

If `dependency` is already an implicit, ordering, or passive dependency of `target` then it is made the last implicit dependency of `target`.  That is the dependency is removed and added as the last implicit dependency.

### clear_implicit_dependencies

~~~lua
function Target.clear_implicit_dependencies( target )
~~~

Clear the list of implicit dependencies of `target`.

### add_ordering_dependency

~~~lua
function Target.add_ordering_dependency( target, dependency )
~~~

Add `dependency` as an ordering dependency of `target`.

If `dependency` is null, the same as `target`, anonymous, or already an explicit or implicit dependency of `target` then this function quietly does nothing.

If `dependency` is already an ordering or passive dependency of `target` then it is made the last ordering dependency of `target`.  That is the dependency is removed and added as the last ordering dependency.

### add_passive_dependency

~~~lua
function Target.add_passive_dependency( target, dependency )
~~~

Add `dependency` as a passive dependency of `target`.

If `dependency` is null, the same as `target`, anonymous, or already an explicit, implicit, or ordering dependency of `target` then this function quietly does nothing.

If `dependency` is already a passive dependency of `target` then it is made the last passive dependency of `target`.  That is the dependency is removed and added as the last passive dependency.

### parent

~~~lua
function Target.parent( target )
~~~

Return the parent of `target`.  This is the target's parent in the hierarchical namespace of targets not the target's parent in the dependency graph (of which there may be many).

### working_directory

~~~lua
function Target.working_directory( target )
~~~

Return the target that is the working directory of `target` or nil if `target` doesn't have a working directory.

### dependency

~~~lua
function Target.dependency( target, n )
~~~

Return the `n`th explicit dependency of `target` or nil if `n` is greater than the number of dependencies of `target`.

### dependencies

~~~lua
function Target.dependencies( target, start, finish )
~~~

Iterate over the explicit dependencies of `target`.  The `start` and `finish` parameters are optional and default to 1 and `INT_MAX` respectively to give the effect of iterating over all the explicit dependencies of `target`.

### ordering_dependencies

~~~lua
function Target.ordering_dependencies( target )
~~~

Iterate over the ordering dependencies of `target`.  The `start` and `finish` parameters are optional and default to 1 and `INT_MAX` respectively to give the effect of iterating over all the ordering dependencies of `target`.

### any_dependencies

~~~lua
function Target.all_dependencies( target )
~~~

Iterate over all dependencies of `target`.  The `start` and `finish` parameters are optional and default to 1 and `INT_MAX` respectively to give the effect of iterating over all dependencies of `target`.
