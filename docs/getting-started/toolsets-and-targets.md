---
layout: page
title: Toolsets and Targets
parent: Getting Started
nav_order: 4
---

- TOC
{:toc}

Define new toolset and target types by creating toolset and target prototypes and overriding the functions that interact with Forge's configuration and internal dependency graph.

Prototypes are to objects (e.g. toolsets and targets) as classes are to objects in languages like C++, C#, and Java.  If you're already familiar with prototypes in languages like Lua or JavaScript then prototypes are literally prototypes.

Toolsets add support for external tools by providing target prototypes and settings used to define dependency graphs representing the targets and dependencies in a build.  For example the *cc* toolset supports C and C++ compilation.

Target prototypes provide targets with behaviors that determine what happens when a target is created, when dependencies are added, and when the target is built and cleaned.

### Writing Toolsets

Create a toolset prototype by calling `forge:ToolsetPrototype()` passing a short identifier as the sole parameter.  This returns a table on which to define the functions provided by the toolset.  Return the toolset prototype from the end of the Lua script that defines it.

Three functions can be overridden to interact with Forge's configuration and initialization of toolsets:

- `Toolset.configure()` is called only when a toolset is created and there isn't a field in the local settings for the target prototype.  The local settings field is named after the short identifier that is passed to create the target prototype.  The call to `Toolset.configure()` is expected to perform one-off auto-detection of installed tools.

- `Toolset.validate()` is called for every toolset that is created.  It should return true when the configuration in local settings is still correct.  Checking that any configured paths to tools are still valid is usually sufficient.  Returning false will quietly ignore the rest of the toolset creation.  If lacking the toolset is a fatal error then this function should assert or raise an error.

- `Toolset.initialize()` is called for every toolset that is created and that successfully configures and validates.  It should setup target prototypes and any settings that need to be setup dynamically at runtime (e.g. variant dependent settings not suitable for storing in local settings).

Other functions needed during definition of the dependency graph can be defined on the toolset prototype also.  These functions are available on toolsets using that toolset prototype from the root build script and any loaded buildfiles.

The identifier passed to `forge:ToolsetPrototype()` identifies the block of per-machine settings returned by the configure function within the *local_settings.lua* file.  The identifier must be unique within your build.  The configuration itself is shared between toolsets and variants.  Any settings that are different between toolsets or variants should be computed in the initialize function that executes for each toolset created in each build.

### Target Prototypes

Target prototypes associate targets with the Lua functions that determine what happens when a target is created, when dependencies are added, and when targets are built and cleaned.

Create a prototype by calling `forge:TargetPrototype()` passing a descriptive identifier.  Override `create()`, `depend()`, `build()`, and/or `clean()` to provide custom behavior during target creation, adding dependencies, building, and cleaning respectively.

Targets without target prototypes are valid.  These targets are typically source files that aren't updated by the build system and so need to be tracked in order to determine when intermediate files that depend on them are outdated but don't need any custom behaviors.
