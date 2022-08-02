---
layout: page
title: Pattern Rules
parent: More Details
nav_order: 40
---

- TOC
{:toc}

`PatternRule()` creates targets with filenames generated from the source filenames used to build them.  For example compiling C or C++ source files to object files, generating the object filenames based on the names of their corresponding sources files.

## Usage

For example the `Cxx` rule is created and set in a toolset as follows:

~~~lua
local Cxx = PatternRule( 'Cxx', gcc.object_filename );
Cxx.build = clang.compile_cxx;
toolset.Cxx = Cxx;
~~~

Then the `Cxx` rule is called from a buildfile to compile C++ source files to object files.  The source filenames `EventSink.cpp` and `forge.cpp` below generate object files named `${obj}/src/forge/forge/EventSink.o` and `${obj}/src/forge/forge/forge.o`:

~~~lua
toolset:Cxx '${obj}/%1' {
    'EventSink.cpp',
    'forge.cpp'
};
~~~

As with other rule calls that create targets pattern rules interpolate their identifier before using it.  This expands `${obj}` to its value in the toolset as expected.  This expansion happens in the replacement string before any patterns are matched.

## Replacements

There are four replacement strings that may appear in the identifier passed to a pattern rule: `%0`, `%1`, `%2`, and `%3`.  They generate the full path, the full path without an extension, the basename, and the extension without a leading dot respectively.

For example the source filename `EventSink.cpp` generates the following output filenames when used with each replacement below:

- `%0`: `src/forge/forge/EventSink.cpp`
- `%1`: `src/forge/forge/EventSink`
- `%2`: `EventSink`
- `%3`: `cpp`

Source filenames are converted to their full paths relative to the root directory before they're matched against the pattern.  This expands `EventSink.cpp` to `src/forge/forge/EventSink.cpp` which is useful for generating object file paths that avoid filename collisions.

## Patterns and Overriding Identifiers

As with file rules the target identifiers and filenames can be overridden.  In this case the identifier passed to the override function has been generated from the source filename, the replacement string has been interpolated and all replacements applied.

For example `gcc.object_filename()` appends the `.o` extension to object files compiled by GCC as follows, `identifier` already contains the root-relative path to the output file without an extension:

~~~lua
function gcc.object_filename( toolset, identifier )
    return ('%s.o'):format( identifier );
end
~~~
