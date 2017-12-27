# Sweet Build

## Overview

Sweet Build is a build tool that tracks dependencies between files; using
relative timestamps to determine which are out of date; and then carrying 
out actions to bring those files up to date.

Sweet Build is similar to GNU Make, Perforce Jam, SCons, Waf, Lake, and other 
dependency based build tools.  Its main difference to these tools is that it 
allows scripts to make arbitrary passes over the dependency graph to carry out
actions.  For example a clean action in Sweet Build is implemented by 
traversing the dependency graph and invoking the clean function for all 
visited targets while in a more traditional build tool the clean action would
be expressed by creating additional phony targets.

Sweet Build handles source trees spanning multiple directories, with multiple 
variants and compilers.

Features:

  - Single executable with no external dependencies.
  - Lua scripting language to specify dependency graph and actions.
  - Support for builds spread across multiple directories.
  - Platform independent path and file system operations.
  - Automatic dependency detection via tracing open files.
  - Filtering the output of external processes.
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

Sweet Build is invoked by running the build executable from the command line.
When invoked the executable searches up from the current working directory 
until it finds a file named *build.lua*.  Once found this file is executed to 
initialize the build system and bring the build up to date.

## Example 

The following buildfile builds *Sweet Build* executable:

~~~lua
local settings =  {
    subsystem = "CONSOLE"; 
    stack_size = 32768; 
    architectures = { "x86_64" };
    -- Disable automatic linking to Boost libraries.
    defines = { "BOOST_ALL_NO_LIB" };
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
            linux = { "pthread", "dl" };
        };
        
        build:Cxx () {
            "Application.cpp", 
            "main.cpp"
        };    
    };
};
~~~
