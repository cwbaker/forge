---
layout: page
title: More Details
nav_order: 4
has_children: true
---

This sections provides more detail about different parts of Forge.  Read as your interest or need directs.

- [Commands](commands.md) describes how to write high-level commands for the build system to carry out, e.g. install.

- [Rules](rules.md) decribes rules and how they define behavior for targets

- [File Rules](file-rules.md) describes how to define rules that build a single output file from one or more input files using the file rule function.

- [Pattern Rules](pattern-rules.md) describes how to define rules that build one or more output files with filenames based on the filenames of their input files using the pattern rule function.

- [Group Rules](group-rules.md) describes how to define rules that group source files that generate one or more output files so they can be passed to a single invocation of a tool, e.g. building C or C++ source files with Microsoft Visual C++.

- [Modules](modules.md) describes how to bundle rules, default settings, and per-machine detection of tools and paths into resuable modules.

- [Buildfile Syntax](buildfile-syntax.md) describes the declarative syntax seen in buildfiles and some Lua idioms that appear often in Forge.

- [Dependencies](dependencies.md) describes the different types of dependencies, how they are used, and how they affect whether or not targets are outdated and the order in which targets are built.

- [Outdated Calculation](outdated-calculation.md) describes how targets are determined to be outdated based on their dependencies and the settings used to build them.

- [String Interpolation](string-interpolation.md) describes the string interpolation used to expand `${...}` references in toolset tags and target identifiers.

- [Automatic Dependencies](automatic-dependencies.md) describes how implicit dependencies can be automatically detected by tracing calls that open files for reading or writing when running a tool.
