---
layout: page
title: String Substitution
parent: More Details
nav_order: 5
---

### String Substitution

The strings passed as identifiers in the buildfile are interpolated with the variables coming from the toolset settings specified in *forge.lua* when the toolset was created, the toolset itself, global variables, and finally environment variables.

The `lib`, `obj`, and `architecture` variables resolve to values set when the toolset was created in *forge.lua*.  The `lib` and `obj` are set only so that they can be referenced later in buildfiles.  The `architecture` variable is used to determine the architecture to compile as well as adding to the name of the file the static library is built to.

String interpolated variables can resolve to functions defined on the toolset.  In this case the function is called passing the toolset and any whitespace separated values following the identifier before the closing brace as arguments.  For example `${foo a b c}` calls the function `foo( toolset, a, b, c)` when used to interpolate a string on a toolset.

The `%1` in `forge:Cc '${obj}/%1'` generates names for the object files using string substitution.  The `%1` represents the first capture that is matched in the pattern; in this case the portion of the path relative to the project root directory without its extension.

Other replacement values for the default pattern are `%0` the root relative path including extension, `%1` the root relative path excluding extension, `%2` the filename excluding extension, and `%3` the extension without any leading dot.
