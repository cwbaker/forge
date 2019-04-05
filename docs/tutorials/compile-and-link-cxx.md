---
layout: page
title: Compile and Link C++
parent: Tutorials
nav_order: 2
---

The classic "Hello World!" as built by Forge.  An example of how to build a C or C++ application with Forge covering variant builds, compilation, static libraries, and linking.

Like all Forge projects the root build script, *forge.lua*, defines the root directory of the project and configures and initializes the modules, prototypes, toolsets, and targets in the dependency graph.  A full listing of *forge.lua* for the C/C++ tutorial follows:

~~~lua
variant = variant or 'debug';

local toolset = require 'forge.cc' {
    identifier = 'cc_${platform}_${architecture}';
    platform = operating_system();
    architecture = 'x86_64';
    bin = root( ('%s/bin'):format(variant) );
    lib = root( ('%s/lib'):format(variant) );
    obj = root( ('%s/obj/cc_%s_x86_64'):format(variant, operating_system()) );
    include_directories = {
        root( 'src' );
    };
    library_directories = {
        root( ('%s/lib'):format(variant) );
    };
    assertions = variant ~= 'shipping';
    debug = variant ~= 'shipping';
};

toolset:all {
    'src/executable/all';
};

buildfile 'src/executable/executable.forge';
buildfile 'src/library/library.forge';
~~~

Set the global `variant` to *debug* if it hasn't been set on the command line.  The or syntax used here is a Lua idiom used to provide default values for optional parameters that might not be specified somewhat like the ternary operator.

Require the Forge C/C++ module, *cc*, and initialize it with output directories for binaries, libraries, and object files and include and library directories to be passed on to the compiler and linker respectively.

Add the "all" target of the *src/executable* directory as a dependency of the "all" target in the root directory.  This causes the *hello_world* executable to be built by default when Forge is run from the root directory.

Define the dependency graph by loading in buildfiles that define the "Hello World!" static library and the executable to drive it.  The buildfiles are loaded from separate directories that also contain the source code for the respective library and executable.  When buildfiles are processed Forge's working directory is updated to be the directory containing the buildfile so that relative paths specified in the buildfile are relative to the directory containing the buildfile.

This allows for reuse of buildfiles across multiple projects each using different settings.  Each project is able to have its own *forge.lua* root build script that provides different toolsets and/or settings.  The buildfiles and source that they build can then be shared using submodules or other equivalent version control or package management.

The buildfile *executable.forge* that defines the "Hello World!" executable is as follows:

~~~lua
for _, toolset in toolsets('cc.*') do
    toolset:all {
        toolset:Executable '${bin}/hello_world' {
            '${lib}/hello_world';
            toolset:Cxx '${obj}/%1' {
                'main.cpp';
            };
        };
    };
end
~~~

The `toolsets()` function iterates over any toolsets registered with an identifier that matches the Lua patterns passed as arguments.  This allows the buildfile to generate targets for multiple architectures, platforms, compilers, or toolchains that have been configured in the root build script *forge.lua*.

The `bin`, `lib`, and `obj` variables pass through directly from the initialization.  There's nothing special about them other than their conventional use to name directories for executables, libraries, and object files respectively.

The library `${lib}/hello_world` is referenced in *executable.forge* before the target for the library is defined.  Targets are created and associated with their identifier on first reference and definition may follow on at a later time.

The object files are built using the pattern or group element `${obj}/%1`.  This generates filenames for the object files by expanding the `obj` variable to the object directory, then appending the path from to root directory to the source file and replacing the source extension by `.o` (or `.obj` for Microsoft Visual C++).

The buildfile *library.forge* that defines the "Hello World!" library is as follows:

~~~lua
for _, toolset in toolsets('cc.*') do
    toolset:StaticLibrary '${lib}/hello_world' {
        toolset:Cxx '${obj}/%1' {
            'hello_world.cpp';
        };
    };
end
~~~
