---
layout: page
title: Depending on Buildfiles
parent: More Details
nav_order: 1
---

- TOC
{:toc}

### Depending on Buildfiles

The `current_buildfile()` function returns the target representing the currently processed buildfile.  Targets that are built differently based on information in the buildfile (beyond the identifiers, dependencies, and settings) should depend on that buildfile.  This causes those targets to become outdated when that buildfile changes.
