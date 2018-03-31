
# Getting Started

## Installation

*Sweet Build* is installed by building it from source code and then linking the `build` executable into your path.  The default install location is `${HOME}/sweet_build` and the `build` executable to link to is at `${HOME}/sweet_build/bin/build`.

The install location is changed by passing `prefix=${install-directory}` on the command line replacing `${install-directory}` with the full path to the directory to install *Sweet Build* to.

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
      -s, --stack-trace  Enable stack traces in error messages.

*Sweet Build* is invoked by running `build` from a current working directory within the project's directory hierarchy.  The current working directory is used to imply the targets to build.  Commands and variable assignments can be passed on the command line to further configure the build.

Run a build from the project's root directory with no arguments to build using default settings:

~~~bash
$ build
~~~

Specify one or more commands on the command line to perform different actions.  Commands are executed in the order that they are specified.

To remove generated files run a *clean* build:

~~~bash
$ build clean
~~~

To rebuild everything run *clean* followed by *default* (the default action being to build):

~~~bash
$ build clean default
~~~

Make assignments on the command line to assign values to global variables in Lua before the build scripts are run to configure things such as variant and platform.

Build the *release* variant:

~~~bash
$ build variant=release
~~~

Build the *shipping* variant for Android instead of the host operating system:

~~~bash
$ build variant=shipping platform=android
~~~
