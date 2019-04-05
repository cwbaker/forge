---
layout: page
title: Target
parent: Reference
nav_order: 8
---

- TOC
{:toc}

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

### prototype

~~~lua
function Target.prototype( target )
~~~

Return the target prototype for this target or nil if this target was implicitly created as a working directory for other targets or doesn't have a target prototype.

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

Add `dependency` as an explicit dependency of `target`.  Cyclic dependencies are not valid but are not reported by this function.  If a cyclic dependency is detected during a traversal then it is silently ignored.  If `dependency` is nil then this function will silently do nothing.

### remove_dependency

~~~lua
function Target.remove_dependency( target, dependency )
~~~

Remove `dependency` as a dependency of `target`.  Removes the dependency regardless of whether that dependency is explicit, implicit, and ordering dependencies.

### add_implicit_dependency

~~~lua
function Target.add_implicit_dependency( target, dependency )
~~~

Add `dependency` as an implicit dependency of `target`.

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

### implicit_dependency

~~~lua
function Target.implicit_dependency( target, n )
~~~

Return the `n`th implicit dependency of `target` or nil if `n` is greater than the number of dependencies of `target.`

### implicit_dependencies

~~~lua
function Target.implicit_dependencies( target )
~~~

Iterate over the implicit dependencies of `target`.  The `start` and `finish` parameters are optional and default to 1 and `INT_MAX` respectively to give the effect of iterating over all the implicit dependencies of `target`.

### ordering_dependency

~~~lua
function Target.ordering_dependency( target )
~~~

Return the `n`th ordering dependency of `target`.  Nil is returned if `n` is greater than the number of ordering dependencies of `target`.

### ordering_dependencies

~~~lua
function Target.ordering_dependencies( target )
~~~

Iterate over the ordering dependencies of `target`.  The `start` and `finish` parameters are optional and default to 1 and `INT_MAX` respectively to give the effect of iterating over all the ordering dependencies of `target`.

### any_dependency

~~~lua
function Target.any_dependency( target )
~~~

Return the `n`th dependency of `target` considering dependencies of all types ordered by explicit, implicit, and ordering.  Nil is returned if `n` is greater than the total number of dependencies of `target`.

### any_dependencies

~~~lua
function Target.any_dependencies( target )
~~~

Iterate over all dependencies of `target`.  The `start` and `finish` parameters are optional and default to 1 and `INT_MAX` respectively to give the effect of iterating over all dependencies of `target`.
