---
layout: page
title: Transitive Dependencies
parent: More Details
nav_order: 1
---

- TOC
{:toc}

## Transitive Dependencies

Transitive dependencies are dependencies that are implied, indirectly by depending on a target.  The canonical example of this is archiving and linking with static libraries in C or C++.  A library A logically depends on another library B when some source from library A includes headers for code that is compiled and archived into library B.  However library B isn't needed at the time library A is archived, it's only needed when library A is linked into a binary.  Once library A is linked into a binary library B must be linked with too.

Forge's passive dependencies are used to implement transitive dependencies.  Targets that imply transitive dependencies, like static libraries in the previous example, implement a `depend()` method that adds passive dependencies for any static libraries that are added as dependencies.  A prepare pass propagates those passive dependencies to the targets those dependencies are used in, the binaries in the example.  The binary targets implement a `prepare()` method that walks all dependencies and adds those it needs as direct, explicit dependencies.
