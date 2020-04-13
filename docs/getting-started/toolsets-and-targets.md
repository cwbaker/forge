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

### Writing Toolsets

Toolsets add support for external tools by providing target prototypes and settings used to define dependency graphs representing the targets and dependencies in a build.  For example the *cc* toolset supports C and C++ compilation.

Create a toolset prototype by calling `forge:ToolsetPrototype()` passing a short identifier as the sole parameter.  This returns a table on which to define the functions provided by the toolset.  Return the toolset prototype from the end of the Lua script that defines it.

Three functions can be overridden to interact with Forge's configuration and initialization of toolsets:

- `Toolset.configure()` is called only when a toolset is created and there isn't a field in the local settings for the target prototype.  The local settings field is named after the short identifier that is passed to create the target prototype.  The call to `Toolset.configure()` is expected to perform one-off auto-detection of installed tools.

- `Toolset.validate()` is called for every toolset that is created.  It should return true when the configuration in local settings is still correct.  Checking that any configured paths to tools are still valid is usually sufficient.  Returning false will quietly ignore the rest of the toolset creation.  If lacking the toolset is a fatal error then this function should assert or raise an error.

- `Toolset.initialize()` is called for every toolset that is created and that successfully configures and validates.  It should setup target prototypes and any settings that need to be setup dynamically at runtime (e.g. variant dependent settings not suitable for storing in local settings).

Other functions needed during definition of the dependency graph can be defined on the toolset prototype also.  These functions are available on toolsets using that toolset prototype from the root build script and any loaded buildfiles.

The identifier passed to `forge:ToolsetPrototype()` identifies the block of per-machine settings returned by the configure function within the *local_settings.lua* file.  The identifier must be unique within your build.  The configuration itself is shared between toolsets and variants.  Any settings that are different between toolsets or variants should be computed in the initialize function that executes for each toolset created in each build.

### Writing Targets

Target prototypes provide targets with behaviors that determine what happens when a target is created and when dependencies are added in buildfiles, and what happens when a target is built and cleaned.

Two functions can be overridden to interact with target creation and dependency specification in buildfiles:

- `Target.create()` is called whenever a target prototype is called to create a new target.  Typically this captures calls of the form *TargetPrototype* **'** *identifier* **'** to create new targets.

- `Target.depend()` is called whenever a target of the target prototype is called to add dependencies.  Typically this captures calls of the form *target* **{** *dependencies* **}** to add dependencies or further define a target.

Two functions can be overridden to interact with build and clean actions during a traversal:

- `Target.build()` is called whenever an outdated target is visited as part of a build traversal.  The function should carry out whatever actions are necessary to build the file or files represented by the target.

- `Target.clean()` is called whenever a target is visited as part of a clean traversal.  The function should carry out whatever actions are necessary to remove files that were generated during a build traversal.  Default clean action for targets marked as cleanable is to delete the file or files represented by the target and a custom function is only needed if this behavior isn't desirable.

For target prototypes that build to single files or build to multiple files based on a pattern or group there are a few functions that create target prototypes with create and depend functions already defined:

- `forge:FilePrototype()` returns a target prototype with create and depend functions that support building one or more source files to a single file.

- `forge:PatternPrototype()` returns a target prototype with create and depend functions that support building multiple source files to multiple files (with a one-to-one mapping) where the destination filename is derived from a pattern matching the source filename.

- `forge:GroupPrototype()` returns a target prototype with create and depend fucntions that support building multiple source files to multiple files in groups where the destination filename is derived from a pattern matching the source filename.  For example the Visual C++ compiler accepts multiple C++ source files in one invocation and it is useful to group these together.

For target prototypes where you must provide your own create and depend functions there is 

- `forge:TargetPrototype()` returns a raw target prototype on which at least a create function must be defined.

Targets without target prototypes are valid.  These targets are typically source files that aren't updated by the build system and but still need to be tracked in order to determine when intermediate files that depend on them are outdated.
