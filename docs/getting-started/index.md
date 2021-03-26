---
layout: page
title: Getting Started
nav_order: 3
has_children: true
---

- TOC
{:toc}

Forge is a Lua scripted build tool that tracks dependencies between files, uses relative timestamps to determine which are out of date, and carries out actions to bring those files up to date.

- [Running Forge](running-forge.md) describes running Forge from the command line to build, clean, run other custom actions defined by the build.

- [Configuring Forge](configuring-forge.md) describes how to configure a build by editing the root build script *forge.lua*.

- [Writing Buildfiles](writing-buildfiles.md) describes writing buildfiles to specify which files are built from which source files.

- [Toolsets and Targets](writing-toolsets.md) describes adding toolset and target prototypes to add support for new tools.
