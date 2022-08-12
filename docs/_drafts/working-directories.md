---
layout: page
title: Working Directories
parent: More Details
nav_order: 1
---

- TOC
{:toc}

## Working Directories

Forge maintains a current working directory that relative paths in the target namespace are considered relative to.  This working directory applies to functions that accept a path to find or create a target and is the working directory for processes spawned by `execute()`.

The working directory can be retrieved by calling `working_directory()` which returns the *target* representing the current working directory.

The working directory of a target is whatever the current working directory was when the target was created.  Usually this is the directory that contains the buildfile that indirectly constructed the target (for targets constructed in a buildfile) or the current working directory at the time the target was created (for targets created at other times).
