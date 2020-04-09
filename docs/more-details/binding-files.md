---
layout: page
title: Binding Files
parent: More Details
nav_order: 3
---

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
