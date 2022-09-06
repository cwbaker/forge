---
layout: page
title: Running Forge
parent: Getting Started
nav_order: 1
---

- TOC
{:toc}

### Usage

~~~sh
Usage: forge [options] [variable=value] [command] ...
Options:
  -h, --help         Print this message and exit.
  -v, --version      Print the version and exit.
  -r, --root         Set root directory.
  -f, --file         Set root build script filename.
  -s, --stack-trace  Stack traces on error.
Variables:
  goal={goal}        Target to build.
  variant={variant}  Variant to build.
Commands:
  build              Build outdated targets.
  clean              Clean all targets.
  reconfigure        Re-run auto-detected configuration.
  dependencies       Print dependency hierarchy.
  namespace          Print target hierarchy.
~~~

Run `forge` from a directory within the project.  When invoked `forge` searches up from the current working directory looking for files named `forge.lua`.  The file in the highest level directory is the root build script, executed to configure the build.  The directory containing `forge.lua` becomes the root directory of the project.

The directory that `forge` is run from is the initial working directory.  By default the target named *all* in this directory is built.  Building from the root directory of the project typically builds all useful outputs for a project.  Building from sub-directories of the project typically builds targets defined in that directory only.

### Commands

Pass commands (e.g. *clean*, *build*, *dependencies*, etc) on the command line to determine what the build does and in what order.  The default, when no other command is passed, is *build* which typically brings all files up to date by building them.

Multiple commands passed on the same command line are executed in order.  All state is restored between commands so passing multiple commands in a single run is the equivalent to passing the same commands to separate runs.  Duplicate commands are executed multiple times.

Build useful outputs by running from the project's root directory:

~~~bash
$ forge
~~~

Remove generated files with the *clean* command:

~~~bash
$ forge clean
~~~

Rebuild by running the *clean* and *build* commands in the same invocation:

~~~bash
$ forge clean build
~~~

Regenerate settings for the local machine by running *reconfigure*:

~~~bash
$ forge reconfigure
~~~

### Variables

Assign values to variables (e.g. *variant={debug, release, shipping}*) on the command line to configure the build.  All assignments are made to global variables in Lua before the root build script and commands are executed. Typically this is used to configure variant, target to build, and install prefix.

Assignments and commands are made and executed in order.  Only assignments made before a command on the command line are made before that command is executed.  Assignment to the same variable overwrites the previous value, this is useful to run the same command for different variants for example.

The **variant** variable can be set to control the settings used when building by passing "variant=_variant_" on the command line.  The default value, used when no variant is passed on the command line, is "debug".  Other accepted values are:

- `debug` - Build with debug information and no optimization to produce executables and static libraries suitable for debugging.  This is the default.

- `release` - Build with optimization and runtime debugging functions to produce executables and static libraries suitable for testing.

- `shipping` - Build with optimization to produce executables and static libraries for shipping.

Build the *release* variant by setting `variant=release`:

~~~bash
$ forge variant=release
~~~

Clean the *release* variant by setting `variant=release` with the *clean* command:

~~~bash
$ forge variant=release clean
~~~

List the dependency graph for the *release* variant:

~~~bash
$ forge variant=release dependencies
~~~

Rebuild the debug and then the release variants:

~~~bash
$ forge variant=debug clean build variant=release clean build
~~~

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
