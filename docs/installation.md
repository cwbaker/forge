---
layout: page
title: Installation
nav_order: 2
---

- TOC
{:toc}

Install Forge by building from source and then linking to the `forge` executable from a directory in your path.

Install to a location other than your home directory by passing `prefix=${install-directory}` on the command line.  For example `forge prefix=D:\\forge variant=shipping install` or `forge prefix=/usr/local/forge variant=shipping install`.

### Linux

From a shell with GCC installed and `~/bin` in `$PATH`:

~~~sh
git clone git@github.com:cwbaker/forge.git forge
cd forge
git submodule update --init
bash ./bootstrap-linux.bash
./bootstrap/bin/forge variant=shipping install
ln -s ~/forge/bin/forge ~/bin/forge
~~~

### macOS

From a shell with the Xcode command line tools installed and `~/bin` in `$PATH`:

~~~sh
git clone git@github.com:cwbaker/forge.git forge
cd forge
git submodule update --init
bash ./bootstrap-macos.bash
./bootstrap/bin/forge variant=shipping install
ln -s ~/forge/bin/forge ~/bin/forge
~~~

### Windows

From a Visual C++ x64 Native Tools command prompt with `%USERPROFILE%\\bin` in the `%PATH%`:

~~~
git clone git@github.com:cwbaker/forge.git forge
cd forge
git submodule update --init
bootstrap-windows.bat
.\bootstrap\bin\forge.exe variant=shipping install
mklink %USERPROFILE%\bin\forge.exe %USERPROFILE%\forge\bin\forge.exe
~~~

NOTE: Forge uses [*vswhere*](https://github.com/Microsoft/vswhere/wiki) to find the Visual C++ compiler for Visual Studio 2015 and later and so requires Visual Studio 15.2 (26418.1 Preview) or later that install *vswhere* to a known location to work out of the box.  Visual Studio 2013, 2012, and 2010 may work but aren't tested.

### Github Actions

To install and use Forge from Github Actions:

~~~yaml
name: Build

on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]

jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [macos-latest, windows-latest, ubuntu-latest]
    steps:
    - name: Checkout
      uses: actions/checkout@v1
    - name: Install Forge
      run: |
        OS=$(echo ${RUNNER_OS} |tr '[:upper:]' '[:lower:]')
        curl -L https://github.com/cwbaker/forge/releases/download/v0.9.0/forge-${OS}-v0.9.0.tar.gz > forge.tar.gz
        tar -xvf forge.tar.gz
      shell: bash
    - name: Build
      run: |
        forge/bin/forge variant=release
      shell: bash
~~~
