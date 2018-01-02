
# Getting Started

## Downloading

Precompiled binaries packaged with build scripts for MacOSX and Windows for the latest version of Sweet Build can be downloaded from [http://www.sweetsoftware.co.nz/](http://www.sweetsoftware.co.nz/).

- [Windows](http://www.sweetsoftware.co.nz/sweet_build_tool-2012-08-14-win32.zip)
- [MacOSX](http://www.sweetsoftware.co.nz/sweet_build_tool-2012-08-14-macosx.zip)

Source code for Sweet Build is available from Bitbucket (the main development repository built from several submodules linking to other repositories) and GitHub (a mirror of the latest release of the Bitbucket repositories combined into a single repository):

- [http://bitbucket.org/cwbaker/sweet_build_tool](http://bitbucket.org/cwbaker/sweet_build_tool)
- [http://github.com/cwbaker/sweet_build_tool](http://github.com/cwbaker/sweet_build_tool)

## Installing

Sweet Build consists of a single executable and the build scripts that define how to build C/C++ programs.

The executable has no external dependencies and can be installed by copying it anywhere that your build process can execute it from.  The build scripts need  to be loaded from a file named `build.lua` in each project being built.  They  can be copied anywhere your build script can locate them.

The recommended installation method is to store the executable and build  scripts in a directory that is version controlled along with the rest of  the source code in your project.  From there the executable and the build  scripts can be easily located by the build process.  This method versions the executable and the build scripts along with the rest of the code in the  project.  Reverting back to a previous revision of the project also reverts  to the build process used to build that revision.

Another installation method is to copy the executable into a directory in the executable path specified by the `PATH` environment variable and the build  scripts into a directory in the Lua search path specified by the `LUA_PATH`  environment variable.  In this scenario the executable and the build scripts  are available to all projects on the machine.  The `build.lua` file can easily load the build scripts using Lua's `require()` function.  This method allows  the executable and the build scripts to be shared across multiple projects.

## Running

One of the design goals of Sweet Build is to allow source code to be built simply by running `build` from the root of the project.  So assuming that you have the build executable and its associated Lua scripts available building your project is as simple as:

```
> build
```

You can specify one or more commands by identifier on the command line:

````
> build clean
````

You can build different variants and/or cross compile for different platforms by setting variables on the command line:

```
> build variant=release
> build variant=shipping platform=android
```

## Building

To build the latest release version of Sweet Build clone the GitHub repository at `git@github.com:cwbaker/sweet_build_tool.git` and run `build/build` from the source directory:

```
> git clone git@github.com:cwbaker/sweet_build_tool.git sweet_build_tool
> cd sweet_build_tool/src
> build/build
```

To build the latest development version of Sweet Build clone the Bitbucket repository at `git@bitbucket.org:cwbaker/sweet_build_tool.git`, making sure to also initialize and update submodules, and then run `build/build` from the source directory:

```
> git clone git@bitbucket.org:cwbaker/sweet_build_tool.git sweet_build_tool 
> cd sweet_build_tool/src
> build/build
```
