---
layout: page
title: Installation
nav_order: 2
---

## Download

Download the latest Forge binary and build scripts from the [GitHub releases](https://github.com/cwbaker/forge/releases) page, unzip it, and then add a symbolic link to the executable `.../forge/bin/forge` from a directory in your path.

## Compile

Install Forge by building it from source and adding a symbolic link to the executable `${HOME}/forge/bin/forge` from a directory in your path.

Change the install location by passing `prefix=${install-directory}` on the command line.  For example `forge prefix=D:\\forge variant=shipping install` on Windows or `forge prefix=/usr/local/forge variant=shipping install` on Linux or macOS.

**Linux:**

From a shell with GCC installed and available on the path:

- `git clone git@github.com:cwbaker/forge.git forge`
- `cd forge`
- `git submodule update --init`
- `bash ./bootstrap-linux.bash`
- `./bootstrap/bin/forge variant=shipping install`
- Link to `${HOME}/forge/bin/forge` from your path

**macOS:**

From a shell with the Xcode command line tools installed:

- `git clone git@github.com:cwbaker/forge.git forge`
- `cd forge`
- `git submodule update --init`
- `bash ./bootstrap-macos.bash`
- `./bootstrap/bin/forge variant=shipping install`
- Link to `${HOME}/forge/bin/forge` from your path

**Windows:**

From a Visual C++ x64 Native Tools command prompt:

- `git clone git@github.com:cwbaker/forge.git forge`
- `cd forge`
- `git submodule update --init`
- `bootstrap-windows.bat`
- `.\bootstrap\bin\forge.exe variant=shipping install`
- Link to `${USERPROFILE}/forge/bin/forge.exe` from your path

NOTE: Forge uses [*vswhere*](https://github.com/Microsoft/vswhere/wiki) to find the Visual C++ compiler for Visual Studio 2015 and later and so requires Visual Studio 15.2 (26418.1 Preview) or later that install *vswhere* to a known location to work out of the box.  Visual Studio 2013, 2012, and 2010 may work but aren't tested.
