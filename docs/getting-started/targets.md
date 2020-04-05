---
layout: page
title: Targets
parent: Getting Started
nav_order: 5
---

- TOC
{:toc}

Target prototypes associate targets with the Lua functions that determine what happens when a target is created, when dependencies are added, and when targets are built and cleaned.

Target prototypes are to targets as classes are to objects in languages like C++, C#, and Java.  If you're familiar with dynamic, prototype based languages like Lua or JavaScript then target prototypes are literally the prototypes that define the behavior of targets.

Create a prototype by calling `forge:TargetPrototype()` passing a descriptive identifier.  Override `create()`, `depend()`, `build()`, and/or `clean()` to provide custom behavior during target creation, adding dependencies, building, and cleaning respectively.

Targets without target prototypes are valid.  These targets are typically source files that aren't updated by the build system and so need to be tracked in order to determine when intermediate files that depend on them are outdated but don't need any custom behaviors.

The `Directory` target prototype is a good example that overrides the default behaviour for create, build, and clean actions:

~~~lua
local Directory = forge:TargetPrototype( 'Directory' );

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

The target prototype is created with a call to `forge:TargetPrototype()` passing `'Directory'` as the identifier for the target prototype.  The identifier only provides debug information during the dump of the dependency graph and so is only required to be informative.

The create function is called when the `Directory` prototype is used to create a target, e.g. when `toolset:Directory(identifier)` is called from a buildfile.  The `toolset` and `identifier` arguments from this call are forwarded through to `Directory.create()` and the target prototype, `Directory`, is forwarded into the third parameter.

The build function is called when running a build and the directory is determined to be outdated.  This simply uses the built-in `mkdir()` function to create the directory.  The `toolset` parameter is the same toolset that was used to create the directory target.  The `target` is the target that represents the directory that needs to be built.

The clean function is called when running the clean command.  The clean function here does nothing because the default clean behavior of deleting a file doesn't work on the directory.  The directory could be removed here with a call to `rmdir()` but that is often problematic if the directory is not empty.

The `Directory` prototype is returned at the end of the script.  This is the value that is returned from the `require()` call that is used to load the target prototype and store it into a toolset (e.g. `toolset.Directory = require 'forge.Directory';`).

Directories don't often appear directly in buildfiles.  Usually they are implicitly created and added as ordering dependencies of other built files.  That way any directories that files will be built into are created before files are built to them.

### Create

~~~lua
function Target.create( toolset, identifier, target_prototype )
~~~

The `create()` function is called whenever a target for that target prototype is created by calling the target prototype table returned from `build.TargetPrototype()`.

The parameters passed are the toolset that the target is being created with, the identifier that was specified, and the target prototype that was used to create the target (possibly nil).

### Depend

~~~lua
function Target.depend( toolset, target, dependencies )
~~~

The `depend()` function is called whenever a call is made on a target for that target prototype.  Typically this captures the use of calls of the form *target* **{** *dependencies* **}** to add dependencies or further define a target.

The parameters passed are the toolset that the target was created with, the target itself, and the dependencies that were passed in.

### Build

~~~lua
function Target.build( toolset, target )
~~~

The `build()` function is called whenever an outdated target is visited as part of a build traversal.  The function should carry out whatever actions are necessary to build the file(s) that it represents up to date.

The parameters passed in are the toolset that the target was created with and the target itself.

### Clean

~~~lua
function Target.clean( toolset, target )
~~~

The `clean()` function is called whenever a target is visited as part of a clean traversal.  The function should carry out whatever actions are necessary to remove files that were generated during a build traversal.

Default behavior when visiting a cleanable target is to remove any files that the target is bound to.  Custom clean behavior is only needed if removing all of the built files is not desired.

The parameters passed in are the toolset that the target was created with and the target itself.

### File Prototypes

~~~lua
function forge:FilePrototype( identifier )
~~~

Create target prototypes that build a single output file from zero or more input files with the `forge:FilePrototype()` prototype function.

The `forge:FilePrototype()` function is a special short-hand that provides a create function on the target prototypes that it returns.  That create function creates a target in the same way as the directory example above adding the directory that contains the file as an ordering dependency.

### Pattern Prototypes

~~~lua
function forge:PatternPrototype( identifier, replacement_modifier, pattern )
~~~

Create target prototypes that generate an output file per input file using pattern matching and replacement to generate identifiers for the output files.

The `forge:PatternPrototype()` function is a special short-hand that provides a proxy create function that generates output and input targets as dependencies are added in the build script.

The `Copy` prototype provided by *Forge* is a very simple pattern-based target prototype that only defines a build action:

~~~lua
local Copy = forge:PatternPrototype( 'Copy' );

function Copy.build( toolset, target )
    rm( target );
    cp( target, target:dependency() );
end

return Copy;
~~~

### Group Prototypes

~~~lua
function forge:GroupPrototype( identifier, replacement_modifier, pattern )
~~~

Create target prototypes that generate an output file per input file using a single invocation of a tool to build multiple files at once.  For example Microsoft's Visual C++ compiler will compile multiple source files at once.

The `forge:GroupPrototype()` function is a special short-hand that provides a proxy create function that generates targets as dependencies are added in the build script.

For example the `Cxx` target prototype used by the Microsoft Visual C++ module to compile C++ is defined as follows:

~~~lua
local Cxx = forge:GroupPrototype( 'Cxx', msvc.object_filename );
Cxx.language = 'c++';
Cxx.build = msvc.compile;
toolset.Cxx = Cxx;
~~~

Targets are the nodes in the dependency graph.  Typically each target represents a single file but targets can also represent a single directory, multiple files, multiple directories, and even groups of targets that are useful to build together but aren't directly related to anything on disk (e.g. the *all* targets in the root and other directories).

Create targets by passing an identifier to one of the target creation functions `forge:SourceFile()`, `forge:File()`, or `forge:Target()` to create targets representing source files, intermediate files, or targets that will be defined later.

The identifier used to create a target is considered a relative path to the target to create.  The relative paths is considered relative to Forge's notion of the current working directory set to the path of the file currently being executed by Forge (e.g. the root build script of the currently executing buildfile).

Targets exist in a hierarchical namespace with the same semantics as operating system paths.  Targets have an identifier, a parent, and zero or more children.  Targets are referred to via `/` delimited paths by using `.` and `..` to refer to the current target and parent target respectively.

The hierarchical namespace is similar but not identical to the hierarchy of directories and files on disk.  Targets are often bound to files on disk in quite separate locations from their location in the target namespace or even not bound to files at all.

Referring to a target with the same identifier always returns the same target.  This allows late binding in the case of depending targets that refer to their dependencies before those dependencies are fully defined.

Anonymous targets can be created using the `anonymous()` function to generate a unique identifier for the target instead of passing a specific identifier.

The order in which targets are built is specified implicitly by the dependencies between targets.  Dependencies of each target are built before the target itself.  Cyclic dependencies are lazily detected at build-time and generate a warning (but not an error).

Target paths and system paths always use `/` as a delimiter.  Convert paths to native paths prior to passing them to external tools if needed with the `native()` function.

### Traversal

Targets and dependencies provide the dependency graph with structure, target prototypes associate actions and behavior with targets, and traversals provide the correct order for those actions to be carried out.

Postorder traversal visits each target's dependencies before it visits that target.  This ordering ensures that dependencies are built before the targets that depend on them.

Targets are only visited once per traversal even if they are depended upon by more than one depending target.  It is assumed that the first visit brings the target up to date and that subsequent visits are unnecessary.

Cyclic references are quietly ignored.

### Binding

Targets are bound to files, directories, and dependencies in an implicit post-order traversal that happens as part of a `postorder()` call before the explicit post-order traversal.

Binding determines whether or not targets are outdated with respect to their dependencies.

Binding provides targets with timestamps, last write times, and determines whether the target has changed since the build system last checked by considering the file(s) that they are bound to and the targets that they depend on.

Targets that aren't bound to files don't track last write time or changed status.  These targets exist solely to group other dependencies together in a convenient way.

Targets that are bound to files that don't exist yet have their timestamp set to the latest possible time so that the target will always be newer than any targets that depend on it.

Targets that are bound to existing directories and have no dependencies have their timestamp set to the earliest possible time so that targets that depend on them won't be considered outdated.  This behavior is to allow targets to depend on the directory that they will be generated into without being outdated based on the last write time of the directory.

Targets that are bound existing files or directories with dependencies have their timestamp and last write time set to the last write time of the file.  Any targets that depend on this target will be outdated if they are older than this target.  Additionally if the last write time of the file or directory has changed from the last time the build system checked the target is marked as having changed.

Targets that are bound to multiple files have their timestamp set to the latest last write time of any of the files and their last write time set to the earliest last write time of any of the files.  This combination covers the range of time represented by this target and interacts correctly with the outdated calculation and timestamp propagation that follows.

Targets are bound to their dependencies after they have been bound to files.  Binding a target to its dependencies sets the timestamp of the target to be the latest timestamp of any of its dependencies and its last write time.

Targets that are bound to files are marked as being outdated if any of their dependencies have timestamps newer than the last write time of the target.  Targets that aren't bound to files are considered to be outdated if any of their dependencies are outdated.
