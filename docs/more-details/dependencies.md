---
layout: page
title: Dependencies
parent: More Details
nav_order: 80
---

- TOC
{:toc}

There are four different types of dependency: explicit, implicit, ordering, and passive.  All dependencies are references from one target to another targets where the latter is a dependency of the former.  The different types differ in their interactions with the dependency graph cache, determining which targets are outdated, and with preorder and postorder traversals.

A useful illustration of how the various types of dependency differ is shown in this table.  Outdated refers to whether or not the dependency determines that the targets depending on it are outdated.  Traversed is whether or not the dependency is traversed in a preorder or postorder traversal.  Cached is whether or not the dependency is written to the dependency graph cache between builds.

|Dependency Type |Traversed |Outdated |Cached |
|--- |--- |--- |--- |
|Explicit |Y |Y |N |
|Implicit |Y |Y |Y |
|Ordering |Y |N |N |
|Passive  |N |N |N |

Explicit, implicit, and ordering dependencies are all traversed as part of a preorder or postorder traversal.  In this sense they determine the order in which targets are built so that dependencies are built before targets that depend on them.

Explicit and implicit dependencies both determine whether or not targets that depend on them are outdated.  When explicit or implicit dependencies change the targets depending on them are marked as outdated so that they're rebuilt.

Ordering dependencies aren't considered when determining whether or not targets that depend on them are outdated.  This is used to express dependencies that must be built before targets that depend on them but don't directly contribute to how that target is built.  For example the directory that a file is written to must be created before the file is written there, but the directory changing doesn't mean that files written there also need to be rebuilt.

Implicit dependencies are written into the dependency graph cache.  Because they're discovered after a target is built there is nowhere else to store them.  Each time a target is built any implicit dependencies it has are cleared and new implicit dependencies added either as the target is built (via automatic dependency detection) or after the target is built by reading some extra output of the tool that builds that target, e.g. the dependency output of a C/C++ compiler's preprocessor.

Explicit and ordering dependencies are not written into the dependency graph cache.  They're expected to be recovered each time the build script runs and generates a new dependency graph.  In this way changes to the structure of the dependency graph are reflected immediately the next time a build is run.

## Explicit Dependencies

Explicit dependencies are added explicitly when the dependency graph is constructed.  These are the dependencies known statically, without needing to build anything.  For example an object file explicitly depends on the source file compiled to generate it.

Explicit dependencies are not saved in the dependency graph cache as they're expected to be recreated again when the build script runs.  This means that changes to the build script and buildfiles directly affect the structure of the dependency graph.

Explicit dependencies are considered when determining whether or not targets that depend on them are outdated.  If an explicit dependency is changed then targets that depend on it are considered outdated.  Explicit dependencies are also traversed to visit targets in a preorder or postorder traversal.  Explicit dependencies determine the order.

## Implicit Dependencies

Implicit dependencies are added implicitly for a target only after a target has been built.  These are dependencies that aren't known until the file has been built at least once.  For example the header files included by a C or C++ source file aren't known until after the compiler has built the file.

Implicit dependencies are saved in the dependency graph cache.  Because they're only discovered after building there's no other place for them to be stored.  Each time a target is built any implicit dependencies it has are cleared and new ones are added either as the target is built (via automatic dependency detection) or afterwards by reading some extra output of the tool that builds that target, e.g. the dependency output of a C/C++ compiler's preprocessor. 

Otherwise implicit dependencies behave the same as explicit dependencies in that they contribute to determining whether or not targets that depend on them are outdated and are edges followed to visit targets in a preorder or postorder traversal.

## Ordering Dependencies

Ordering dependencies are added explicitly when the dependency graph is constructed but they have slightly different behavior to explicit dependencies.  An ordering dependency is built before the targets that depend on it but it doesn't affect whether or not targets that depend on it are outdated.  This is useful for having targets depend on the directory they will be built into without having changes to that directory cause those targets to become outdated.

## Passive Dependencies

Passive dependencies aren't directly involved in binding or traversals.  They provide a convenient place store dependencies that need to be
handled in the build without involving changes to whether or not a target is outdated or what order it is built in.  Transitive dependencies are the general example of this.  Specifically static libraries that logically depend on other static libraries are an example of transitive dependencies.  The static libraries don't require their dependencies at the time they're built but those dependencies are required to be linked into binaries that link the depending static libraries.
