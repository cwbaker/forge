---
layout: page
title: File Rules
parent: More Details
nav_order: 30
---

- TOC
{:toc}

`FileRule()` creates a rule that creates files.  This wraps the `Toolset:File()` convenience function discussed in [Targets](#targets) adding the ability to call the rule to create targets from buildfiles.

## Usage

For example using `FileRule` to create a rule to archive object files into a static library.  It uses `FileRule()` to create the rule and overrides `depend()` and `build()` but avoids having to add any boiler-plate code in `create()` to interact with the buildfile domain language:

~~~lua
local StaticLibrary = FileRule( 'StaticLibrary', gcc.static_library_filename );
StaticLibrary.build = gcc.archive;
StaticLibrary.depend = cc.static_library_depend;
toolset.StaticLibrary = StaticLibrary;
~~~

The `StaticLibrary` prototype is called from a buildfile to archive object files into a static library:

~~~lua
for _, cc in toolsets('^cc.*') do
    cc:StaticLibrary '${lib}/assert_${architecture}' {
        cc:Cc '${obj}/%1' {
            'assert.cpp'
        };
    };
end
~~~

## Identifiers and Filenames

The second parameter to `FileRule` allows the identifier and filename of the created target to be overridden.  The default, used when nothing is passed in the second parameter, uses `Toolset.interpolate()` to expand the identifier passed to create the target to use as both identifier and filename for the target.  Otherwise the function passed in can return the identifier and filename for the target based on the toolset and identifier that is passed in.

For example `gcc.static_library_filename()` is implemented as:

~~~lua
function gcc.static_library_filename( toolset, identifier )
    local identifier = absolute( toolset:interpolate(identifier) );
    local filename = ('%s/lib%s.a'):format( branch(identifier), leaf(identifier) );
    return identifier, filename;
end
~~~
