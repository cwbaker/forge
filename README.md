# Sweet Build

*Sweet Build* is a Lua scriptable build tool that tracks dependencies between files, uses relative timestamps to determine which are out of date, and carries out actions to bring those files up to date.

## Features

  - Single executable with no external dependencies
  - Lua scripting language specifies dependency graph and actions
  - Fast incremental builds
  - Automatic implict dependency detection via tracing open files
  - Arbitrary passes over the full or partial dependency graph
  - Platform independent path and file system operations
  - Modular builds spanning multiple directories
  - Variant builds (e.g. debug, release, etc)
  - Runs on Windows, macOS, and Linux

## Example

Copy `foo.in` to `foo.out`:

~~~lua
package.path = build:default_package_path();

require "build";

build:initialize {
    cache = 'copy-file-example.cache';
};

build:all {
    build:Copy 'foo.out' {
        'foo.in'
    };    
};
~~~

## Installation

*Sweet Build* is installed by building it from source code and then linking the `build` executable into your path.  The default install location is `${HOME}/sweet_build` and the `build` executable to link to is at `${HOME}/sweet_build/bin/build`.

The install location is changed by passing `prefix=${install_directory}` on the command line replacing `${install_directory}` with the full path to the directory to install *Sweet Build* to.

**Linux:**

From a shell with GCC installed and available on the path:

- `git clone git@github.com:cwbaker/sweet_build.git sweet_build`
- `cd sweet_build/src`
- `sh ./bootstrap-linux.sh`
- `../bootstrap-linux/build variant=shipping install`
- Link to `${HOME}/sweet_build/bin/build` from your path

**macOS:**

From a shell with Xcode installed:

- `git clone git@github.com:cwbaker/sweet_build.git sweet_build`
- `cd sweet_build/src`
- `sh ./bootstrap-macos.sh`
- `../bootstrap-macos/build variant=shipping install`
- Link to `${HOME}/sweet_build/bin/build` from your path

**Windows:**

From a Visual C++ command prompt:

- `git clone git@github.com:cwbaker/sweet_build.git sweet_build`
- `cd sweet_build\src`
- `bootstrap-windows.bat`
- `..\bootstrap-windows\build.exe variant=shipping install`
- Link to `${USERPROFILE}/sweet_build/bin/build.exe` from your path

## Usage

    Usage: build [options] [variable=value] [command] ...
    Options:
      -h, --help         Print this message and exit.
      -v, --version      Print the version and exit.
      -f, --file         Set the script file to load.
      -s, --stack-trace  Enable stack traces in error messages.

*Sweet Build* is invoked by running `build` from a directory within a project's directory hierarchy.  A build then proceeds through the following four steps:

- Search up from the current working directory to find the first directory containing a file named `build.lua`.

- Assign values to global variables in Lua for all assignments (`variable=value`) passed on the command line to parameterize the build (e.g. `variant=release`, `version=2.0.x`, etc).

- Execute the previously found `build.lua` to configure the build and load the initial dependency graph.  The dependency graph is typically loaded by loading several modular Lua scripts (referred to as buildfiles).

- Call global functions for each command (`command`) passed on the command line to carry out the desired build actions (e.g. `clean`, `default`, etc).

## Contributions

All contributions and feedback are welcomed.  Please use the Github issue tracker and/or open a pull request.  Thanks!

## License

*Sweet Build* is licensed under the [MIT License](http://www.opensource.org/licenses/MIT)
