---
layout: page
title: Outdated Calculation
parent: More Details
nav_order: 90
---

- TOC
{:toc}

A target is outdated if any of its dependencies have changed.  If any of the input files change then the output files must be rebuilt.  Changes between a target and its dependencies are detected by comparing the last write times of the files the target and its dependencies are bound to.

The change detection is also extended to the settings in the toolset changing too.  If the settings used to build an output file change then that output file must be rebuilt.  Settings changes are detected by comparing hashes of settings at the time the target was last built and the current hash of the settings.

Targets determined to be outdated or not in an internal postorder traversal known as the *bind pass*.  The bind pass is made over the dependency graph before each postorder pass in a call to `postorder()` for the same targets that will be visited in that pass.  Understanding the bind pass is useful because it determines which targets are considered outdated and provides more insight into the usefulness of the different dependency types.

The bind pass, like the general postorder traversal, happens only over explicit and implicit dependencies.  Ordering dependencies don't contribute to the timestamp or outdated calculation of the targets that depend on them, they only enforce an ordering on the following postorder traversal.  Passive dependencies are a convenience for propagating dependencies in earlier passes.  See [Dependencies](#dependencies).

## Timestamps

Each target is visited in a postorder traversal, that is dependencies are visited before the targets that depend on them.  Visiting each target involves binding to the target's files and dependencies.

Binding a target to a single file set the target's timestamp and last write time to the last write time of the file that it is bound to.  Binding a target to its dependencies sets the target's timestamp to the latest timestamp of any of its dependencies and marks the target outdated if any of its dependencies are outdated.  This propagates outdated flags and timestamps through the dependency graph.  Targets with outdated dependencies are themselves outdated.  Targets have a timestamp representing the latest last write time for themselves and any of their dependencies.

Target's that aren't already outdated by their dependencies are marked as outdated if any of the following conditions are true:

1. The target's timestamp is later than its last write time
2. Settings have changed since the target was last built
3. The target is cleanable and has not been built

The first condition marks targets as outdated when their dependencies have changed.  Because the timestamp stores the latest timestamp of any of the target's dependencies comparing it against the last write time of the target compares whether the target's dependencies have changed since the target was last written.

The second condition marks targets as outdated when the settings used to generate them have changed.  Because settings affect the way in which targets are built changing them must cause targets to be rebuilt, e.g. enabling asserts when compiling a C or C++ source file generates quite different object files so enabling or disabling asserts must rebuilt those object files.  Settings changes are detected by comparing the hash of the current settings with the hash of the settings used the last time the target was built.  See [Hashes](#hashes).

The third condition covers the corner case of all files being built but the dependency graph cache not being able to be read from disk.  In this situation all timestamps indicate that nothing needs to be rebuilt but Forge is missing any implicit dependency information such as header files included by C/C++ source files.  Forge delays adding implicit dependencies like header files until after a target has been built and, when it doesn't have information about implicit dependencies, those header files can be changed without causing the source files and object files that depend on them to be outdated and rebuilt.

## Hashes

Changes to settings in a toolset trigger a rebuild of targets built with that toolset.  For example enabling asserts when compiling a C or C++ source file produces quite different object files -- so enabling or disabling asserts must rebuild those object files.

To track changes to settings each target stores a hash of the settings used at the time it was last built.  This hash is updated each time the target is built, if the hash changes at that time then the target is marked as outdated.

The hash is an order-independent hash calculated over the settings in a toolset, that is all the fields in the toolset that aren't functions or rules.  The hash is recursive through tables, cyclic links are quietly ignored but should be avoided because they lead to inconsistent hashes.

## Multiple Files

Binding a target to multiple files sets the target's timestamp and last write time to the latest and earliest last write times respectively of all the files the target is bound to.  This has the effect of propagating the latest write time on to determine whether depending targets are outdated while comparing the earliest write time against the target's own dependencies.

## Phony Targets

Targets that are not bound to any files are known as phony targets.  They set their timestamp and last write time to the start of the Unix epoch (i.e. zero).  Timestamps and outdated are propagated entirely from dependencies for phony targets.

Phony targets exist as a convenience to group other dependencies together in places where the it is useful to process more than one dependency at once.  One example is the `all` target that groups together all of the top-level targets in a directory that should be built by default.  Another example is targets generated by rules based on `GroupRule` such as the Microsoft Visual C++ compiler support for passing multiple source files to a single invocation of the compiler.
