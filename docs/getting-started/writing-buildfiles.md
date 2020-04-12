---
layout: page
title: Writing Buildfiles
parent: Getting Started
nav_order: 3
---

- TOC
{:toc}

Buildfiles are the plain Lua scripts that specify the targets and dependencies for a build.  They have the extension *.forge* and appear throughout the directory hierarchy of a project.

The `buildfile()` function, used to load a buildfile, temporarily updates the working directory to the directory containing the buildfile and then executes the buildfile.  This makes relative paths in the buildfile relative to the buildfile.

Having buildfile relative paths leads naturally to having a buildfile per source containing directory with buildfiles in parent directories loading buildfiles from any directories beneath their own.

### Buildfiles

Here we continue the example of the previous section building a C++ static library and executable to print the classic "Hello World!".  The two buildfiles involved are listed fully below and significant features described in the following paragraphs.

From *src/library/library.forge* a C++ static library implementing the classic "Hello World!" is compiled and archived:

~~~lua
for _, cc in toolsets('^cc.*') do
    cc:StaticLibrary '${lib}/hello_world' {
        cc:Cxx '${obj}/%1' {
            'hello_world.cpp';
        };
    };
end
~~~

From *src/executable/executable.forge* a C++ executable is linked that uses the library above to print "Hello World!":

~~~lua
local version = os.date( '%Y.%m.%d' );

for _, cc in toolsets('^cc.*') do
    local cc = cc:inherit {
        subsystem = 'CONSOLE'; 
        stack_size = 32768;
    };

    cc:all {
        cc:Executable '${bin}/hello_world' {
            '${lib}/hello_world';
            cc:Cxx '${obj}/%1' {
                defines = {    
                    ('VERSION="\\"%s\\""'):format( version );
                };
                'main.cpp';
            };
        };
    };
end
~~~

### Accessing Toolsets

Buildfiles retrieve toolsets defined in the root build script by looping over toolsets with identifiers that match a pattern.  Multiple toolsets are useful when building for multiple architectures (e.g. armv7, arm64, etc) for Android or iOS.

~~~lua
for _, cc in toolsets('^cc.*') do
    -- Define targets using the `cc` toolset here.
end
~~~

The loop over `toolsets('^cc.*')` iterates over the toolsets that were registered in *forge.lua* with identifiers that start with `cc`.  This pattern is a Lua pattern not a regular expression.  See [6.4.1 Patterns](https://www.lua.org/manual/5.3/manual.html#6.4.1) in the Lua manual.

### Targets and Dependencies

Targets are the nodes in the dependency graph.  Typically each target has an associated file that it builds, a target prototype to define its behavior, and a list of dependencies that must be built first.

Create targets by calling the target prototypes defined on a toolset like the calls made to `cc:StaticLibrary` and `cc:Cxx` below.  Identifiers are interpolated with settings or functions looked up from the toolset, global variables, and environment variables in that order.

Add dependencies by making a call on the depending target passing the dependencies in a table.  Strings passed in this call are implicitly converted into targets representing source files also with interpolation.

~~~lua
for _, cc in toolsets('^cc.*') do
    cc:StaticLibrary '${lib}/hello_world' {
        cc:Cxx '${obj}/%1' {
            'hello_world.cpp';
        };
    };
end
~~~

Target creation and dependency calls are usually chained together.  You will often see, as in this example, C++ source files being compiled to object files as `Cxx` targets being passed straight to a `StaticLibrary` target to be archived into a static library.

### Buildfile Specific Settings

Apply setting specific to a buildfile by inheriting settings from a toolset into a new, temporary toolset and overriding those settings that need to change.

~~~lua
for _, cc in toolsets('^cc.*') do
    local cc = cc:inherit {
        subsystem = 'CONSOLE'; 
        stack_size = 32768;
    };
    -- Define targets using the inherited `cc` toolset here.
end
~~~

### Default Targets

Buildfiles communicate their entry-point/root-level targets by adding them as dependencies of a special target "all" in their directory.  The root build script for a project can then add these "all" targets to the "all" target in the root directory so that building from the root directory defaults to building all important targets for the project.

~~~lua
for _, cc in toolsets('^cc.*') do
    cc:all {
        -- Define or list default targets here.
    };
end
~~~

### Linking Libraries

Libraries are linked into an executable or dynamic library by listing them as dependencies of the executable target.  String values appearing as dependencies are interpolated so that `${lib}/hello_world` is expanded appropriately.

~~~lua
for _, cc in toolsets('^cc.*') do
    -- ...
    cc:all {
        cc:Executable '${bin}/hello_world' {
            '${lib}/hello_world';
            cc:Cxx '${obj}/%1' {
                'main.cpp';
            };
        };
    };
end
~~~

### Linking Third-Party Libraries

Third party libraries that exist outside of the project can be passed as a list in the `library` attribute of the dependencies call.  In this case the libraries are the system libraries *pthread* and *dl* for thread and dynamic linking support on Linux.  Other scenarios might be third-party libraries that are not provided in source form.

~~~lua
local libraries;
if operating_system() == 'linux' then
    libraries = { 
        'pthread', 
        'dl' 
    };
end

-- ...

for _, cc in toolsets('^cc.*') do
    cc:all {
        cc:Executable '${bin}/forge' {
            '${lib}/forge_${architecture}';
            libraries = libraries;
            -- ...
        };
    };
end
~~~

### Per-Target Settings

Some settings are available to be set on each target by passing fields with string keys in a dependency call.  See the documentation for each target prototype for the exact settings that this is available for.

For example pre-processor macros are specified by setting the `defines` attribute of the dependencies call to for a `Cc` or `Cxx` target.  Consecutive calls are cumulative with each other and with defines specified in the settings.

~~~lua
local version = ('%s'):format( os.date('%Y.%m.%d') );

-- ...

forge:Cxx '${obj}/%1' {
    defines = {    
        ('VERSION="\\"%s\\""'):format( version );
    };
    'Application.cpp', 
    'main.cpp'
}; 

-- ...
~~~
