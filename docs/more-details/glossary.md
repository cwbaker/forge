% Glossary

## Objects

*Dependency graph* is the directed acyclic graph of targets (nodes, mostly corresponding to files) and dependencies between them (edges).

*Targets* are the nodes in the dependency graph representing the files in the build.

*Target prototypes* associate behavior and actions with targets.  Target prototypes are to targets as classes are to objects in a language like C++, C#, or Java.

*Build script* is a file named `forge.lua` at the root of a project directory hierarchy.  The build script is a Lua script that is run to configure the build and generate the dependency graph.

*Toolsets* are groups of settings and tools that define the targets in a build.  The build script defines one or more toolsets used to create targets in the dependency graph for the build.

*Modules* are Lua modules that install settings and target prototypes into toolsets to integrate external tools with Forge.  Modules bundle together useful target prototypes and their initialization for use in a toolset.

*Buildfiles* are Lua scripts using a simple Lua DSL to define the dependency graph of targets.  They have the `.forge` extension to mark their logical difference from the build script, modules, and other Lua scripts.

## Operations

*Binding* is an implicit binding pass that binds targets to files and calculating timestamps and which targets are outdated.

*Pre-order traversal* visits each target in the graph before visiting its dependencies.  The pre-order traversal is used to propagate transitive dependencies before the build is run.

*Post-order traversal* traverses the graph visiting dependencies before the targets that depend on them.  The post-order traversal is the traversal that updates outdated targets.

*Loading* reloads a previously saved dependency graph.  Implicit dependencies not specified in buildfiles are reloaded, other dependencies are set when the build script and buildfiles are executed.

*Saving* stores the previously saved dependency graph.  Targets last write time, last build time, hash, etc are saved.  Implicit dependencies are saved.  Other dependencies are not.
