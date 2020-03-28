# Forge

[![Build Status](https://travis-ci.com/cwbaker/forge.svg?branch=master)](https://travis-ci.com/cwbaker/forge)

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

## Documentation

See [https://cwbaker.github.io/forge/](https://cwbaker.github.io/forge/) for more documentation.

## Contributions

All contributions and feedback are welcomed.  Please use the Github issue tracker and/or open a pull request.  Thanks!

## License

*Forge* is licensed under the [MIT License](http://www.opensource.org/licenses/MIT)
