# Forge

*Forge* is a Lua scriptable build tool that tracks dependencies between files, uses relative timestamps to determine which are out of date, and carries out actions to bring those files up to date.

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
require 'forge';

forge:initialize();

forge:all {
    forge:Copy 'foo.out' {
        'foo.in'
    };    
};
~~~

The Lua script that defines the `Copy` target used in the Lua-based domain specific language above is defined as follows:

~~~lua
local Copy = forge:FilePrototype( 'Copy' );

function Copy.build( forge, target )
    forge:rm( target );
    forge:cp( target, target:dependency() );
end
~~~

## Installation

*Forge* is installed by building it from source code and then linking the `forge` executable into your path.  The default install location is `${HOME}/forge` and the `forge` executable to link to is at `${HOME}/forge/bin/forge`.

The install location is changed by passing `prefix=${install-directory}` on the command line replacing `${install-directory}` with the full path to the directory to install *Forge* to.

**Linux:**

From a shell with GCC installed and available on the path:

- `git clone git@github.com:cwbaker/forge.git forge`
- `cd forge`
- `git submodule update --init`
- `bash ./bootstrap-linux.bash`
- `./bootstrap-linux/bin/forge variant=shipping install`
- Link to `${HOME}/forge/bin/forge` from your path

**macOS:**

From a shell with Xcode installed:

- `git clone git@github.com:cwbaker/forge.git forge`
- `cd forge`
- `git submodule update --init`
- `bash ./bootstrap-macos.bash`
- `./bootstrap-macos/bin/forge variant=shipping install`
- Link to `${HOME}/forge/bin/forge` from your path

**Windows:**

From a Visual C++ command prompt:

- `git clone git@github.com:cwbaker/forge.git forge`
- `cd forge`
- `git submodule update --init`
- `bootstrap-windows.bat`
- `.\bootstrap-windows\bin\forge.exe variant=shipping install`
- Link to `${USERPROFILE}/forge/bin/forge.exe` from your path

NOTE: *Forge* uses [*vswhere*](https://github.com/Microsoft/vswhere/wiki) to find the Visual C++ compiler for Visual Studio 2015 and later and so requires Visual Studio 15.2 (26418.1 Preview) or later that install *vswhere* to a known location to work out of the box.  Visual Studio 2013, 2012, and 2010 may work but aren't tested.

## Usage

    Usage: forge [options] [variable=value] [command] ... 
    Options: 
      -h, --help         Print this message and exit.
      -v, --version      Print the version and exit.
      -r, --root         Set the root directory.
      -f, --file         Set the name of the root build script.
      -s, --stack-trace  Enable stack traces in error messages.

*Forge* is invoked by running `forge` from a current working directory within the project's directory hierarchy.  The current working directory is used to imply the targets to build.  Commands and variable assignments can be passed on the command line to further configure what the build does.

Run a build from the project's root directory with no arguments to build using default settings:

~~~bash
$ forge
~~~

Specify one or more commands on the command line to perform different actions.  Commands are executed in the order that they are specified.

To remove generated files run a *clean* build:

~~~bash
$ forge clean
~~~

To rebuild everything run *clean* followed by *default* (the default action being to build):

~~~bash
$ forge clean default
~~~

Make assignments on the command line to assign values to global variables in Lua before the build scripts are run to configure things such as variant and platform.

Build the *release* variant:

~~~bash
$ forge variant=release
~~~

## Contributions

All contributions and feedback are welcomed.  Please use the Github issue tracker and/or open a pull request.  Thanks!

## License

*Forge* is licensed under the [MIT License](http://www.opensource.org/licenses/MIT)
