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
  -r, --root         Set the root directory.
  -f, --file         Set the name of the root build script.
  -s, --stack-trace  Enable stack traces in error messages.
Variables:
  goal               Target to build.
  variant            Variant built (debug, release, shipping).
Commands:
  build              Build outdated targets.
  clean              Clean all targets.
  reconfigure        Regenerate configuration settings.
  dependencies       Print targets by dependency hierarchy.
  namespace          Print targets by namespace hierarchy.
~~~

Run `forge` from a directory within the project.  Forge will search up from that directory to the root of the file system looking for files named *forge.lua*.  The *forge.lua* file found in the highest directory is the root build script executed to define the build.  The directory containing the root build script is the root directory of the project.

The directory that `forge` is run from is the initial working directory.  By default the target named *all* in this initial directory is built.  Building from the root directory of the project typically builds all useful outputs for a project.  Building from sub-directories of the project typically builds targets defined in that directory only.

### Commands

Pass commands (e.g. *clean*, *build*, *dependencies*, etc) on the command line to determine what the build does and in what order.  The default, when no other command is passed, is *build* which typically brings all files up to date by building them.

Multiple commands passed on the same command line are executed in order.  All state is restored between commands so passing multiple commands to one invocation is functionally the same as passing the same commands to separate invocations.  Duplicate commands are executed multiple times.

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

Assign values to variables (e.g. *variant={debug, release, shipping}*) on the command line to configure the build.  All assignments are made to global variables in Lua before the root build script and any actions are executed.  Typically this is used to configure variant, target to build, and/or install location.

Later assignments override earlier ones in the case of duplicate variables.  However because all assignments are made before any commands are executed interleaving assignments and commands is not generally useful.

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
