
# Sweet Build

Sweet Build is a build tool that tracks dependencies between files; using their timestamps to determine which are out of date; and then carrying out actions to bring those files up to date.

Sweet Build is similar to GNU Make, Perforce Jam, SCons, Waf, Lake, and other  dependency based build tools.  Its main difference to these tools is that it  allows scripts to make arbitrary passes over the dependency graph to carry out actions.  For example a clean action in Sweet Build is implemented by traversing the dependency graph and invoking the clean function for all visited targets while in a more traditional build tool the clean action would be expressed by creating additional phony targets.

Sweet Build handles source trees spanning multiple directories, with multiple variants and compilers.  Multiple processor machines are utilized by allowing dependency scanning and execution of external processes to happen in parallel.

## Features

- Single executable with no external dependencies.
- Lua scripting language to specify dependency graph and actions.
- Visual Studio and XCode project generation.
- Automatic implicit dependency detection.
- Parallel execution to make use of multiple processors.
- Platform independent path and file system operations.
- Support for builds spread across multiple directories.
- Dependency graph save and load for faster incremental builds.
- Variant builds.

## What does it look like?

The following snippet of code is the entire buildfile used to build the Sweet Build executable.

``` lua
local settings = build.push_settings {
    subsystem = "CONSOLE"; 
    stack_size = 32768; 
    architectures = { "x86_64" };
    -- Disable automatic linking to Boost libraries.
    defines = { "BOOST_ALL_NO_LIB" };
};

for _, architecture in ipairs(settings.architectures) do
    build.Executable ("build", architecture) {
        libraries = {
            "boost/libs/filesystem/src/boost_filesystem",
            "boost/libs/system/src/boost_system",
            "sweet/build_tool/build_tool",
            "sweet/cmdline/cmdline",
            "sweet/lua/lua",
            "lua/liblua",
            "sweet/process/process",
            "sweet/rtti/rtti",
            "sweet/thread/thread",
            "sweet/persist/persist",
            "sweet/path/path",
            "sweet/pointer/pointer",
            "sweet/error/error",
            "sweet/assert/assert"
        };
        
        build.Cxx (architecture) {
            "Application.cpp", 
            "main.cpp"
        };    
    }
end

build.pop_settings();
```

The libraries listed resolve to library targets defined in other buildfiles contained within the source tree allowing for nice modular reuse of your build scripts.  The libraries become dependencies of the `build` executable and so are rebuilt when necessary and are automatically added to the linker command line.

Building the `build` executable from the command line on MacOSX *or* Windows is as simple as running the following command line from within the `src/build_tool/build` directory:

```
> build
```

Builds of different variants can be triggered by setting the `variant` variable on the command line:

```
> build variant=release
```

Builds for different platforms can be triggered by setting the `platform` variable on the command line:

```
> build platform=android
```

## Sweet Build vs Other Tools

These are timing tests based on the tests run by Noel Llopis in his "The Quest for the Perfect Build System" blog entries (parts [1](http://gamesfromwithin.com/the-quest-for-the-perfect-build-system), [2](http://gamesfromwithin.com/the-quest-for-the-perfect-build-system-part-2), and [3](http://gamesfromwithin.com/bad-news-for-scons-fans)).

In the test 50 libraries with 100 classes each are built.  Each source file includes 15 headers from its own library and 5 headers from other libraries.

The tests are run on an Intel Core2 Duo P8600 @2.4GHz, 4GB RAM, with a Samsung SSD RBX series 128GB hard drive.

|Tool |Full |Incremental |Single
|--- |--- |--- |---
|GNU Make |456.3s |4.2s |3.2s
|SCons |427.2s |43.1s |8.7s
|Sweet Build |135.4s |1.8s |1.6s

Sweet Build performs well on a full rebuild because it passes all files on the command line to a single invocation of the compiler while both GNU Make and SCons pass each file to a separate invocation of the compiler.
