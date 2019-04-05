---
layout: default
title: Command Line
parent: Reference
nav_order: 1
---

# Command Line

~~~sh
Usage: forge [options] [variable=value] [command] ...
Options:
  -h, --help         Print this message and exit.
  -v, --version      Print the version and exit.
  -r, --root         Set the root directory.
  -f, --file         Set the name of the root build script.
  -s, --stack-trace  Enable stack traces in error messages.
Variables:
  goal               Target to build (relative to working directory).
  variant            Variant built (debug, release, shipping).
Commands:
  build              Build outdated targets.
  clean              Clean all targets.
  reconfigure        Regenerate per-machine configuration settings.
  dependencies       Print targets by dependency hierarchy.
  namespace          Print targets by namespace hierarchy.
~~~

Forge is invoked by running the *forge* executable from the command line.  When invoked the executable searches up from the current working directory until it finds a file named *forge.lua*.  Once found this file is executed to load and initialize the build system.

Information is passed from the command line in the form of assignments and commands.  Expressions of the form _variable=value_ are interpreted as assignments.  Any other identifiers on the command line are interpreted as commands.

The assignments listed on the command line specify values that are assigned to global variables before *forge.lua* is loaded.  The assignments provide a way to pass arguments to the build system.  For example the variant to build can be specified by setting the value of the *variant* variable:

~~~
> forge variant=release
~~~

The commands listed on the command line specify functions that are called after *forge.lua* has been loaded.  The commands provide a way to specify which build actions the build system should carry out.  For example a clean action can be carried out by passing the *clean* command:

~~~
> forge clean
~~~

## Commands

The default command, used when no command is passed on the command line, is *default*.  This carries out a build action that traverses the dependency graph and brings outdated files up to date.

Valid commands are:

- `default` - builds targets by calling through to the `build` command below.  This is the default.

- `build` - builds targets.

- `clean` - remove generated and intermediate files created during a build.

- `dependencies` - print the targets in the dependency graph.

- `reconfigure` - regenerate the per-machine configuration in 
`local_settings.lua`.

Invocation from the top level directory of a project with an empty command line builds everything in that project for the "debug" variant with the default compiler toolchain for the current platform (GCC on Linux, Clang on macOS, and Microsoft Visual C++ on Windows).

~~~
> forge
~~~

Invocation with the "clean" command removes files generated during a previous build:

~~~
> forge clean
~~~

## Variants

The **variant** variable can be set to control the settings used when building by passing "variant=_variant_" on the command line.  The default value, used when no variant is passed on the command line, is "debug".  Other accepted values are:

- `debug` - Build with debug information and no optimization to produce executables and static libraries suitable for debugging.  This is the default.

- `release` - Build with optimization and runtime debugging functions to produce executables and static libraries suitable for testing.

- `shipping` - Build with optimization to produce executables and static libraries for shipping.

Invocation with the "release" variant builds using release options:

~~~
> forge variant=release
~~~

## Goal

The **goal** variable can be set to to specify the target to build by passing "goal=_goal_" on the command line.  The goal is interpreted as a path to the target to build.  Relative values are considered relative to the current working directory.  The target should always be specified using forward slashes as it is a target path not an operating system path.  The default, used when no goal is passed on the command line, is to use the goal that corresponds to the current working directory.

The executables and libraries in the `src/forge` directory can be built (as opposed to building all of the executables and libraries in the project) by specifying the relative path to the `src/forge` directory from the command line using the goal variable:

~~~
> forge goal=src/forge
~~~

Alternatively the executables and libraries in the `src/forge` directory can be built by changing to the `src/forge` directory before invoking the build:

~~~
> cd src/forge
> forge
~~~
