# Sweet Build

*Sweet Build* is a Lua scriptable build tool that tracks dependencies between files; using relative timestamps to determine which are out of date; and then carrying out actions to bring those files up to date.

A simple, real-world example Lua script used to build the *process* library used in *Sweet Build*:

~~~lua
buildfile "process_test/process_test.build";

for _, build in build:default_builds("cc_.*") do
    build:Library "process" {
        build:Cxx () {
            "Error.cpp",
            "Environment.cpp",
            "Process.cpp"
        };
    };
end
~~~

*Sweet Build* is similar to GNU Make, Perforce Jam, SCons, Waf, Lake, and other dependency based build tools.  Its main difference to these tools is that it allows scripts to make arbitrary passes over the dependency graph to carry out actions.  For example a clean action in Sweet Build is implemented by traversing the dependency graph and invoking the clean function for all visited targets while in a more traditional build tool the clean action would be expressed by creating additional phony targets.

*Sweet Build* handles source trees spanning multiple directories, with multiple variants and compilers.

Features:

  - Single executable with no external dependencies.
  - Lua scripting language to specify dependency graph and actions.
  - Support for builds spread across multiple directories.
  - Platform independent path and file system operations.
  - Automatic dependency detection via tracing open files.
  - Filtering output of external processes.
  - Dependency graph save and load for faster incremental builds.
  - Parallel execution makes use of multiple processors.
  - Variant builds.

NOTE: Automatic dependency tracking comes with some caveats!  On Windows dependencies are only automatically tracked for 64 bit processes.  On macOS dependencies are only automatically tracked for non-system executables (it is possible to disable system integrity protection (SIP) to avoid this limitation).

Anti-features:

  - Not in widespread use.

## Usage

    Usage: build [options] [variable=value] [command] ...
    Options:
      -h, --help         Print this message and exit.
      -v, --version      Print the version and exit.
      -f, --file         Set the script file to load.
      -s, --stack-trace  Enable stack traces in error messages.

*Sweet Build* is invoked by running the build executable from the command line.  When invoked the executable searches up from the current working directory until it finds a file named `build.lua`.  Once found this file is executed to initialize the build system and bring the build up to date.

## Examples

The main functionality of *Sweet Build* is contained in a library built by the following build script:

~~~lua

buildfile "build/build.build";
buildfile "build_hooks/build_hooks.build";
buildfile "build_tool_lua/build_tool_lua.build";

for _, build in build:default_builds("cc_.*") do
    local settings =  {
        defines = {
            "_CRT_SECURE_NO_DEPRECATE",
            "_SCL_SECURE_NO_DEPRECATE",
            "_WIN32_WINNT=0x0500",
            "WIN32_LEAN_AND_MEAN",
            "BOOST_ALL_NO_LIB", -- No automatic linking to Boost libraries.
        };
    };

    -- Disable warnings on Linux to avoid unused variable warnings in Boost
    -- System library headers.
    if build:operating_system() == "linux" then
        settings.warning_level = 0;
    end

    local build = build:configure( settings );
    build:Library "build_tool" {
        build:Cxx () {
            "Arguments.cpp",
            "BuildTool.cpp",
            "BuildToolEventSink.cpp",
            "Context.cpp",
            "Error.cpp", 
            "Executor.cpp",
            "Filter.cpp",
            "Graph.cpp",
            "GraphReader.cpp",
            "GraphWriter.cpp",
            "Job.cpp",
            "Reader.cpp", 
            "Scheduler.cpp", 
            "System.cpp",
            "Target.cpp",
            "TargetPrototype.cpp",
            "path_functions.cpp"
        };
    };
end
~~~

The *Sweet Build* executable is built by the following build script:

~~~lua
local settings =  {
    subsystem = "CONSOLE"; 
    stack_size = 32768; 
    architectures = { 
        "x86_64" 
    };
    defines = { 
        "BOOST_ALL_NO_LIB", -- No automatic linking to Boost libraries.
    };
};

-- Disable warnings on Linux to avoid unused variable warnings in Boost
-- System library headers.
if build:operating_system() == "linux" then
    settings.warning_level = 0;
end

local build = build:configure( settings );
build:all {
    build:Executable "build" {
        libraries = {    
            "sweet/build_tool/build_tool",
            "sweet/build_tool/build_tool_lua/build_tool_lua",
            "sweet/process/process",
            "sweet/luaxx/luaxx",
            "sweet/cmdline/cmdline",
            "sweet/error/error",
            "sweet/assert/assert",
            "lua/liblua",
            "boost/libs/filesystem/src/boost_filesystem",
            "boost/libs/system/src/boost_system"
        };

        system_libraries = build:switch {
            platform;
            linux = { 
                "pthread", 
                "dl" 
            };
        };
        
        build:Cxx () {
            "Application.cpp", 
            "main.cpp"
        };    
    };
};
~~~
