
# Command Line

~~~
Usage: build [options] [variable=value] [command] ...
Options:
  -h, --help         Print this message and exit.
  -v, --version      Print the version and exit.
  -f, --file         Set the script file to load.
  -W, --warn         Set the warning level.
  -s, --stack-trace  Enable stack traces in error messages.
~~~

Sweet Build is invoked by running the build executable from the command line.  When invoked the executable searches up from the current working directory until it finds a file named `build.lua`.  Once found this file is executed to load and initialize the build system.

Information is passed from the command line in the form of assignments and commands.  Expressions of the form _variable=value_ are interpreted as assignments.  Any other identifiers on the command line are interpreted as commands.

The assignments listed on the command line specify values that are assigned to global variables before `build.lua` is loaded.  The assignments provide a way to pass arguments to the build system.  For example the variant to build can be specified by setting the value of the _variant_ variable:

```
> build variant=release
```

The commands listed on the command line specify functions that are called after `build.lua` has been loaded.  The commands provide a way to specify which build actions the build system should carry out.  For example a clean action can be carried out by passing the _clean_ command:

```
> build clean
```

## Commands

The default command, used when no command is passed on the command line, is `default`.  This carries out a build action that traverses the dependency graph and brings outdated files up to date.

Valid commands are:

- `default` - builds targets.  This is the default.

- `clean` - remove generated and intermediate files created during a build.

- `dependencies` - print the targets in the dependency graph.

- `namespace` - print the namespace of targets in the dependency graph.

- `reconfigure` - regenerate the per-machine configuration in 
`local_settings.lua`.

- `sln` - generate a Visual Studio solution and projects.

- `xcodeproj` - generate an Xcode project.

Invocation from the top level directory of a project with an empty command line builds everything in that project for the "debug" variant and the "msvc" or "llvmgcc" platform depending on operating system.

```
> build
```

Invocation with the "clean" command removes files generated during a previous build:

```
> build clean
```

## Variants

The **variant** variable can be set to control the settings used when building by passing "variant=_variant_" on the command line.  The default value, used when no variant is passed on the command line, is "debug".  Other accepted values are:

 - `debug` - Build with debug information and no optimization to produce executables and static libraries suitable for debugging.  This is the default.

 - `debug_dll` - Build with debug information and no optimization to produce execuables and dynamic libraries suitable for debugging.

 - `release` - Build with optimization and runtime debugging functions to produce executables and static libraries suitable for testing.

 - `release_dll` - Build with optimization and runtime debugging functions to produce executables and dynamic libraries suitable for testing.

 - `shipping` - Build with optimization to produce executables and static libraries for shipping.

 - `shipping_dll` - Build with optimization to produce executables and dynamic libraries for shipping.

Invocation with the "release" variant builds using release options:

```
> build variant=release
```

## Platforms

The **platform** variable can be set to control the tools used when building by passing "platform=_platform_" on the command line.  The default value, used when no platform is passed on the command line, is either "msvc" or "llvmgcc" depending on whether the build is being run on Microsoft Windows or Apple MacOSX.

- `clang` - build using Apple Clang.  This is the default when running 
on MacOSX.

- `msvc` - build using Microsoft Visual C++ 9.0 or 10.0.  This is the 
default when running on Microsoft Windows.

- `mingw` - build using MinGW.

## Goal

The **goal** variable can be set to to specify the target to build by passing "goal=_goal_" on the command line.  The goal is interpreted as a path to the target to build.  Relative values are considered relative to the current working directory.  The target should always be specified using forward slashes as it is a target path not an operating system path.  The default, used when no goal is passed on the command line, is to use the goal that corresponds to the current working directory.

The executables and libraries in the `sweet/build_tool` directory can be built (as opposed to building all of the executables and libraries under the `src` directory) by specifying the relative path to the `sweet/build_tool` directory from the command line using the goal variable:

```
> build goal=sweet/build_tool
```

Alternatively the executables and libraries in the `sweet/build_tool` directory can be built by changing to the `sweet/build_tool` directory before invoking the build:

```
> cd sweet/build_tool
> build
```

## Version

The **version** variable can be set to control the value of the preprocessor macro `BUILD_VERSION` by passing "version=_version_" on the command line.  The default, used when no version is set on the command line, is to use the date, time, variant, and platform of the current build.

## Jobs

The **jobs** variable can be set to control the maximum number of jobs to allow in parallel by passing "jobs=_jobs_" on the command line.  The default, used when jobs is not set on the command line, is four.
