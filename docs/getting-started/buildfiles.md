---
layout: page
title: Buildfiles
parent: Getting Started
nav_order: 3
---

- TOC
{:toc}

Buildfiles specify the targets and dependencies that make up the dependency graph for a project.  They have the *.forge* extension and appear throughout the directory hierarchy of a project.

The separation of configuration (in the root build script, *forge.lua*) and the dependency graph definition (in the buildfiles, *\*.forge* throughout the project hierarchy) allows the components of a project to be built in a modular and reusable way.

Looking back and continuing on from our previous use of the Forge project as an example we can see that the first buildfile that is loaded to define the dependency graph for the Forge project is *assert.forge* as follows:

~~~lua
-- ...
buildfile 'src/assert/assert.forge';
-- ...
~~~

The `buildfile()` function loads a buildfile.  The working directory is temporarily changed to the directory containing the buildfile and then the buildfile is executed.  Relative paths expressed within the buildfile become relative to the directory containing the buildfile by default.  The working directory is restored once the buildfile has been loaded.

The contents of the buildfile *src/assert/assert.forge*:

~~~lua
for _, forge in toolsets('^cc.*') do
    forge:StaticLibrary '${lib}/assert_${architecture}' {
        forge:Cc '${obj}/%1' {
            'assert.cpp'
        };
    };
end
~~~

### Multiple Toolsets

The loop over `toolsets('^cc.*')` iterates over all toolsets that were registered in *forge.lua* with identifiers that match the Lua pattern `^cc.*` (the toolsets with identifiers that start with `cc`).

This looping allows for components to be built multiple times with different settings.  For example the *assert* library might being built for multiple architectures for Android or iOS (e.g. armv7, arm64, etc).  

It is possible to build multiple variants using multiple toolsets but that is uncommon.  Variants are usually separated to keep the dependency graph smaller and build faster.

### String Interpolation

The strings passed as identifiers in the buildfile are interpolated with the variables coming from the toolset settings specified in *forge.lua* when the toolset was created, the toolset itself, global variables, and finally environment variables.

The `lib`, `obj`, and `architecture` variables resolve to values set when the toolset was created in *forge.lua*.  The `lib` and `obj` are set only so that they can be referenced later in buildfiles.  The `architecture` variable is used to determine the architecture to compile as well as adding to the name of the file the static library is built to.

String interpolated variables can resolve to functions defined on the toolset.  In this case the function is called passing the toolset and any whitespace separated values following the identifier before the closing brace as arguments.  For example `${foo a b c}` calls the function `foo( toolset, a, b, c)` when used to interpolate a string on a toolset.

The `%1` in `forge:Cc '${obj}/%1'` generates names for the object files using string substitution.  The `%1` represents the first capture that is matched in the pattern; in this case the portion of the path relative to the project root directory without its extension.

Other replacement values for the default pattern are `%0` the root relative path including extension, `%1` the root relative path excluding extension, `%2` the filename excluding extension, and `%3` the extension without any leading dot.

### Linking

The buildfile that builds the Forge executable references the *assert* library built in the previous example:

~~~lua
-- Set the version as date/time YYYY.mm.dd HH:MM:SS and variant.
local version = ('%s %s'):format( os.date('%Y.%m.%d %H:%M:%S'), variant or 'debug' );

-- Disable warnings on Linux to avoid unused variable warnings in Boost
-- System library headers.
local libraries;
local warning_level = 3;
if operating_system() == 'linux' then
    warning_level = 0;
    libraries = { 
        'pthread', 
        'dl' 
    };
end

for _, forge in toolsets('^cc.*') do
    local forge = forge:inherit {
        subsystem = 'CONSOLE'; 
        stack_size = 32768;
        warning_level = warning_level;    
    };

    forge:all {
        forge:Executable '${bin}/forge' {
            '${lib}/forge_${architecture}';
            '${lib}/forge_lua_${architecture}';
            '${lib}/process_${architecture}';
            '${lib}/luaxx_${architecture}';
            '${lib}/cmdline_${architecture}';
            '${lib}/error_${architecture}';
            '${lib}/assert_${architecture}';
            '${lib}/liblua_${architecture}';
            '${lib}/boost_filesystem_${architecture}';
            '${lib}/boost_system_${architecture}';

            libraries = libraries;
            
            forge:Cxx '${obj}/%1' {
                defines = {    
                    'BOOST_ALL_NO_LIB'; -- Disable automatic linking to Boost libraries.
                    ('BUILD_VERSION="\\"%s\\""'):format( version );
                };
                'Application.cpp', 
                'main.cpp'
            };    
        };
    };
end
~~~

### Versioning

Generating version numbers for built components can be rolled any which way you like.  Here the current date, time, and variant is used.  Other possibilities include passing version on the command line if you already have an external means of generating a version number, executing external processes from within the buildfile, or generating a version number from a Lua program running within Forge.

~~~lua
local version = ('%s %s'):format( os.date('%Y.%m.%d %H:%M:%S'), variant or 'debug' );
~~~

The generated version number is passed into the build later as a pre-processor macro.

### Buildfile Specific Settings and Inheritance

Platform specific settings that apply only to a single buildfile are defined using setting inheritance.

~~~lua
-- Disable warnings on Linux to avoid unused variable warnings in Boost
-- System library headers.
local libraries;
local warning_level = 3;
if operating_system() == 'linux' then
    warning_level = 0;
    libraries = { 
        'pthread', 
        'dl' 
    };
end

for _, forge in toolsets('^cc.*') do
    local forge = forge:inherit {
        subsystem = 'CONSOLE'; 
        stack_size = 32768;
        warning_level = warning_level;    
    };
-- ...
~~~

### Linking Libraries

Libraries are linked into an executable or dynamic library by listing them as dependencies of the executable target.  String values appearing as dependencies are interpolated so that `${lib}/assert_${architecture}` is expanded appropriately.

~~~lua
    forge:all {
        forge:Executable '${bin}/forge' {
            '${lib}/forge_${architecture}';
            '${lib}/forge_lua_${architecture}';
            '${lib}/process_${architecture}';
            '${lib}/luaxx_${architecture}';
            '${lib}/cmdline_${architecture}';
            '${lib}/error_${architecture}';
            '${lib}/assert_${architecture}';
            '${lib}/liblua_${architecture}';
            '${lib}/boost_filesystem_${architecture}';
            '${lib}/boost_system_${architecture}';

            libraries = libraries;
            
~~~

Libraries that exist outside of the project and build are passed as a list in the `library` attribute of dependencies.  In this case the libraries are the system libraries *pthread* and *dl* for thread and dynamic linking support on Linux.  Other scenarios might be third-party libraries that are not provided in source form.

### Preprocessor Macros

Pre-processor macros are passed as a list in the `defines` attribute of dependencies and settings.  The list specified in settings is combined with the lists specified in dependencies so that all listed macros are defined.  Multi

~~~lua
forge:Cxx '${obj}/%1' {
    defines = {    
        'BOOST_ALL_NO_LIB'; -- Disable automatic linking to Boost libraries.
        ('BUILD_VERSION="\\"%s\\""'):format( version );
    };
    'Application.cpp', 
    'main.cpp'
};    
~~~
