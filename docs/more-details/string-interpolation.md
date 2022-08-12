---
layout: page
title: String Interpolation
parent: More Details
nav_order: 130
---

- TOC
{:toc}

Forge provides string interpolation for identifiers passed to prototype calls that create targets and strings passed directly as dependencies within buildfiles.

Patterns that match `${...}` within those strings are replaced by looking up the value `...` as follows:

- Toolset variables
- Global variables
- Environment variables

If the looked up value is a function it is called with the list of whitespace separated parameters contained within `...` following the first identifier.

## Target Identifiers

Identifiers passed to create targets, e.g. `${output}/bar.out` in the example above, are interpolated -- that is the syntax `${...}` appearing in an identifier is replaced by looking up the variable appearing as `...` in the toolset, Lua global variables, or environment variables, in that order.

Identifiers are the only place that this happens implicitly in Forge.  That extends to the identifiers for targets that appear only as dependencies too as these are generally forwarded as the identifier for a target eventually.  Keep this in mind for two reasons:

1. Strings in places other than identifiers are **not** interpolated implicitly.  To interpolate them use `Toolset:interpolate()`.

2. Keep this convention when adding `Target.create()` functions and be sure to interpolate the identifier passed in.

## Toolset Tags

Tags passed to toolsets are also interpolated.  The call to `forge.Toolset()` accepts a tag replacement string that is interpolated to become the toolset's tag when the toolset is defined with its settings.
