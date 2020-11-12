# Forge

![Build](https://github.com/cwbaker/forge/workflows/Build/badge.svg)

*Forge* is a Lua scriptable build tool that tracks dependencies between files, uses relative timestamps to determine which are out of date, and carries out actions to bring those files up to date.

## Example

Copy `{bar,baz,foo}.in` to `output/{bar,baz,foo}.out`:

~~~lua
local toolset = require 'forge' {
    output = root( 'output' );
};

toolset:all {
    toolset:Copy '${output}/%1.out' {
        'bar.in';
        'baz.in';
        'foo.in';
    };    
};
~~~

The Lua script that defines the `Copy` target used in the Lua-based domain specific language above is defined as follows:

~~~lua
local Copy = forge:PatternPrototype( 'Copy' );

function Copy.build( toolset, target )
    rm( target );
    cp( target, target:dependency() );
end

return Copy;
~~~

Execute the build by running `forge` from a directory within the project:

~~~bash
$ forge
~~~

## Installation

Install Forge by building it from source and linking to `${HOME}/forge/bin/forge` or `${USERPROFILE}/forge/bin/forge.exe` from your path from macOS/Linux and Windows respectively.  Boa sorte!

Install to another location by passing `prefix=${install-directory}` on the command line.  For example `forge prefix=D:\\forge variant=shipping install` or `forge prefix=/usr/local/forge variant=shipping install`.

**Linux:**

From a shell with GCC installed and that `~/bin` is in the path:

~~~sh
git clone git@github.com:cwbaker/forge.git forge
cd forge
git submodule update --init
bash ./bootstrap-linux.bash
./bootstrap/bin/forge variant=shipping install
ln -s ~/forge/bin/forge ~/bin/forge
~~~

**macOS:**

From a shell with the Xcode command line tools installed and `~/bin` in the path:

~~~sh
git clone git@github.com:cwbaker/forge.git forge
cd forge
git submodule update --init
bash ./bootstrap-macos.bash
./bootstrap/bin/forge variant=shipping install
ln -s ~/forge/bin/forge ~/bin/forge
~~~

**Windows:**

From a Visual C++ x64 Native Tools command prompt with `%USERPROFILE%\\bin` in the path:

~~~
git clone git@github.com:cwbaker/forge.git forge
cd forge
git submodule update --init
bootstrap-windows.bat
.\bootstrap\bin\forge.exe variant=shipping install
mklink %USERPROFILE%\bin\forge.exe %USERPROFILE%\forge\bin\forge.exe
~~~

NOTE: Forge uses [*vswhere*](https://github.com/Microsoft/vswhere/wiki) to find the Visual C++ compiler for Visual Studio 2015 and later and so requires Visual Studio 15.2 (26418.1 Preview) or later that install *vswhere* to a known location to work out of the box.  Visual Studio 2013, 2012, and 2010 may work but aren't tested.

## Documentation

See [https://cwbaker.github.io/forge/](https://cwbaker.github.io/forge/) for more documentation.

## Contributions

All contributions and feedback are welcomed.  Please use the Github issue tracker and/or open a pull request.  Thanks!

## License

*Forge* is licensed under the [MIT License](http://www.opensource.org/licenses/MIT)
