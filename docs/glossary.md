---
layout: page
title: Glossary
nav_order: 99
---

- TOC
{:toc}

### Targets

Targets are the nodes in the dependency graph.  Typically each target has a one-to-one relationship with a file in the file system but there are also cases where targets have no associated file or many associated files.

### Prototypes

Prototypes associate targets with the Lua functions that determine what happens when a target is created, when dependencies are added, and when a target is built or cleaned.

### Toolsets

Toolsets are sets of variables and prototypes that provide a configuration and a convenient, largely declarative domain language for specifying the dependency graph.

### Modules

Modules package prototypes and toolsets into resuable components that add support for different compilers, platforms, and tools (e.g. Clang, GCC, Mingw, and Microsoft Visual C++, support for building Android and/or iOS apps, and other tools like parser generators, or asset builders for video games).

### Root Build Script

The root build script, *forge.lua*, marks the root directory of the project and is executed to load and configure the modules and toolsets available when defining the dependency graph and building the project.

### Buildfiles

Buildfiles provide re-usable definitions of portions of the dependency graph using Lua's convenient syntactic sugar and the interface provided by the toolsets registered by the root build script.
