---
layout: page
title: Lua Syntax and Idioms
nav_order: 98
---

The language used in both build scripts and buildfiles is Lua.  The language is described in detail in the [Lua 5.3 Reference Manual](https://www.lua.org/manual/5.3/).

Some differences between Lua and other programming languages that show up in
build scripts and buildfiles are:

- Strings can be delimited with double square brackets (`[[` and `]]`) as 
well as the usual single and double quotes.  Strings delimited with double
square brackets don't do normal escaping while single and double quoted 
strings do.

- Lua's main data type is an associative array known as a table.  Tables are constructed using curly braces containing an optional list of fields.  Tables can be treated as arrays, maps, and objects interchangeably.  Array indices are numbered from 1 and not 0 like you might expect.  Don't worry!  It works far better in practice than you're expecting.

- Functions are first class values and show up as fields within tables, 
local variables, parameters, and return values.

- Lexical scoping is used to determine the visibility of variables.  Local
variables are available from within functions defined within the same scope
even after that scope has been released from the stack.

- An identifier followed by a string literal or an identifier followed by a
table constructor is syntactic sugar for a function call taking a string or 
table as its only argument.  This syntax shows up in the Forge DSL as the use of **prototype** [*identifier* | **(** *identifier*, *...* **)**] to specify a target to be built and the use of *target* **{** *dependencies* **}** to add dependencies to an already specified target.

- These two idioms and the fact that dependent targets may be specified directly as part of *dependencies* allow convenient chaining of target creation and dependencies into **prototype** (*identifier* \| **(** *identifier* **,** *...* **)**) **{** *dependencies* **}** to provide the declarative syntax used in buildfiles.

- An identifier followed by colon "`:`" followed by another identifier is 
syntactic sugar for a function call on a table.  The first identifier refers 
to the table and second identifier to a function stored in that table.  The 
table is passed as the implicit first parameter to the function.  The "`:`" 
syntax can also be used when the function is defined in which case it implies
a hidden first parameter named "`self`". 

- Lua is a prototype based language similar to *Self* and *JavaScript*.  A table can set another table to act as its prototype and have lookups for fields that aren't defined automatically redirected to be looked up in the prototype table.  The table inherits the fields that it doesn't define itself from its prototype.

- Settings inheritance in Forge is implemented using prototypes.  Per target settings are inherited from their parent target's settings or the global settings.  The global settings are inherited the default settings specified by the build system and the project being built.

- The behaviour of targets in the build system is also defined using prototypes.  In this case each target has its own target prototype and the target prototype defines the functions that specify which actions are taken for that type of target in a traversal of the dependency graph.

---

- The root build script, *forge.lua*, that sets up the build for the copy file tutorial is the following:

~~~lua
-- Initialize Forge and configure the output directory to *output* relative
-- to the root directory that contains *forge.lua*.
local toolset = require 'forge' {
    output = root( 'output' );
};

-- Use the Lua-based DSL to create a dependency graph that copies `foo.in` to
-- `output/foo.out` when `foo.in` is newer or `output/foo.out` doesn't exist.
toolset:all {
    toolset:Copy '${output}/foo.out' {
        'foo.in'
    };    
};
~~~

- The syntax may appear a little foreign if you aren't familiar with Lua's syntactic sugar for function arguments with tables and string literals and "method" calls.

- From the Lua Manual - [3.4.10 Function Calls](https://www.lua.org/manual/5.3/manual.html#3.4.10):

> A call v:name(args) is syntactic sugar for v.name(v,args), except that v is evaluated only once.

> A call of the form f{fields} is syntactic sugar for f({fields}); that is, the argument list is a single new table. A call of the form f'string' (or f"string" or f[[string]]) is syntactic sugar for f('string'); that is, the argument list is a single literal string.

- Adding the omitted parentheses and parameters for function calls into our example makes where calls are being made and with which parameters a little clearer:

~~~lua
-- Initialize Forge and configure the output directory to *output* relative
-- to the root directory that contains *forge.lua*.
local toolset = require( 'forge' )( {
    output = root( 'output' );
} );

-- Use the Lua-based DSL to create a dependency graph that copies `foo.in` to
-- `output/foo.out` when `foo.in` is newer or `output/foo.out` doesn't exist.
toolset.all( toolset, {
    toolset.Copy ( toolset, '${output}/foo.out' )( {
        'foo.in'
    } );
} );
~~~
