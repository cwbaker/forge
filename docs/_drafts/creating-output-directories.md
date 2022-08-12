---
layout: page
title: Creating Output Directories
parent: More Details
nav_order: 1
---

- TOC
{:toc}

### Creating Output Directories

Every file target automatically adds an ordering dependency on the directory that it will be output to.  This ensures that output directories are always created before any targets are written to them.

If you're defining your own meta-prototypes you should make sure to do the same.  It's very convenient to have output directories already created and, because this is now written in the manual, people will expect it to be so!

The fact that the dependency is an ordering directory is important.  An ordering dependency ensures that the dependency is built or visited before the targets that depend on it.  However the ordering dependency doesn't affect the timestamp or outdated flag of the targets that depend on it.

The ordering dependency is built before targets that depend on it without forcing those targets to be rebuilt if the directory changes.  This is exactly what we want to ensure a directory is created before targets that are written there without that directory changing (which will happen each time files are added or removed) causing those targets to be rebuilt.
