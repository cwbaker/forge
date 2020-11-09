# Forge

[![Build Status](https://travis-ci.com/cwbaker/forge.svg?branch=master)](https://travis-ci.com/cwbaker/forge)

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

### Linux
```
git clone git@github.com:cwbaker/forge.git forge
cd forge
git submodule update --init
bash ./bootstrap-linux.bash
./bootstrap/bin/forge variant=shipping install
```

### Windows
```
git clone git@github.com:cwbaker/forge.git forge
cd forge
git submodule update --init
bootstrap-windows.bat
.\bootstrap\bin\forge.exe variant=shipping install
```

### macOS
```
git clone git@github.com:cwbaker/forge.git forge
cd forge
git submodule update --init
bash ./bootstrap-macos.bash
./bootstrap/bin/forge variant=shipping install
```

On all platforms, the binaries and associated [https://www.lua.org/](Lua) scripts will be installed in your [https://en.wikipedia.org/wiki/Home_directory](Home) directory, and should be added to your platform's `PATH` variable.

A build may then be made within any directory containing a Forge project as below:

```
> forge
```

See [https://cwbaker.github.io/forge/installation/](Installation) for more complete documentation. Boa sorte!

## Documentation

See [https://cwbaker.github.io/forge/](https://cwbaker.github.io/forge/) for more documentation.

## Contributions

All contributions and feedback are welcomed.  Please use the Github issue tracker and/or open a pull request.  Thanks!

## License

*Forge* is licensed under the [MIT License](http://www.opensource.org/licenses/MIT)
