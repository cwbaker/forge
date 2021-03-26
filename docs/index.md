---
layout: page
title: Overview
nav_order: 1
---

Forge is a Lua scriptable build tool that tracks dependencies between files, uses relative timestamps to determine which are out of date, and carries out actions to bring those files up to date.

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
local Copy = PatternPrototype( 'Copy' );

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
