---
layout: page
title: Targets
parent: Getting Started
nav_order: 6
---

- TOC
{:toc}

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

