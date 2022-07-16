---
layout: page
title: Lua Syntax and Idioms
nav_order: 98
---

- TOC
{:toc}

The language used in build scripts and buildfiles is Lua.  The language is described in detail in the [Lua 5.3 Reference Manual](https://www.lua.org/manual/5.3/).

### Function Call Syntax

Consider the following root build script for a build that copies the files `{bar,baz,foo}.in` to `output/{bar,baz,foo}.out`:

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

This is a plain Lua script.  The syntax may appear a little foreign if you aren't familiar with Lua's syntactic sugar that allows parentheses to be left out for function arguments with tables and string literals and "method" calls made with the colon syntax.

From the Lua Manual - [3.4.10 Function Calls](https://www.lua.org/manual/5.3/manual.html#3.4.10):

> A call v:name(args) is syntactic sugar for v.name(v,args), except that v is evaluated only once.

An identifier followed by colon followed by another identifier is 
syntactic sugar for a function call on a table.  The first identifier refers 
to the table and second to a function stored in that table.  The table is passed as the implicit first parameter to the function.  The colon syntax is also used in function definition in which case it implies a hidden first parameter named `self`. 

> A call of the form f{fields} is syntactic sugar for f({fields}); that is, the argument list is a single new table. A call of the form f'string' (or f"string" or f[[string]]) is syntactic sugar for f('string'); that is, the argument list is a single literal string.

An identifier followed by a string literal or a table constructor is syntactic sugar for a function call.  This syntax shows up in Forge as the use of *rule* [*identifier* \| **(** *identifier*, *...* **)**] to create a target and the use of *target* **{** *dependencies* **}** to add dependencies to an already specified target.

Adding the omitted parentheses and parameters makes the function calls and their arguments a little clearer:

~~~lua
local toolset = require( 'forge' )( {
    output = root( 'output' );
} );

toolset.all( toolset, {
    toolset.Copy( toolset, '${output}/%1.out' )( {
        'bar.in';
        'baz.in';
        'foo.in';
    } );
} );
~~~

These two idioms and the fact that dependent targets may be specified directly as part of *dependencies* allow convenient chaining of target creation and dependencies into **rule** (*identifier* \| **(** *identifier* **,** *...* **)**) **{** *dependencies* **}** to provide the declarative syntax used in buildfiles.

With syntax explained we can work through what this script actually means in the context of a build.  The logical split into two steps is quite clear with the first step being the creation of the toolset and the second step the calls to create targets and specify dependencies.

The toolset is created by requiring the `forge` module and then making a call on it passing the settings for apply to the toolset as the sole argument (recall that calls with a single table argument may omit parentheses).  The required module acts as a rule for toolsets and calling it creates a toolset with behavior defined by that rule.

The toolset is then used to define the dependency graph of files and actions in the build.  The call to `toolset.all()` makes the output files dependencies of the special "all" target so that they are built by default when `forge` is run from that directory.  The `toolset.Copy()` call creates targets that copy the source files into the output directory using pattern matching and replacement to generate the output filenames from the inputs.

### Tables

Lua's main data type is an associative array known as a table.  Tables are constructed using curly braces containing an optional list of fields.  Tables can be treated as arrays, maps, and objects interchangeably.

Array indices are numbered from 1 and not 0 like you might expect.  Don't worry!  It works far better in practice than you're expecting.

### Strings

Strings can be delimited with double square brackets (`[[` and `]]`) as 
well as the usual single and double quotes.  Strings delimited with double
square brackets don't do normal escaping while single and double quoted 
strings do.

### Functions and Lexical Scoping

Functions are first class values and show up as fields within tables, 
local variables, parameters, and return values.

Lexical scoping is used to determine the visibility of variables.  Local
variables are available from within functions defined within the same scope
even after that scope has been released from the stack.

### Default Values

Default values are provided for variables that are optionally set on the command line using a Lua idiom of assigning a value to itself or a default value.  In Lua the `or` operator returns its first argument if it is not false; otherwise it will return its second argument.  Unassigned variables evaluate to `nil` which is interpreted as false.  Thus the effect here is to preserve any value that may have been set on the command line or provide a sensible default otherwise.
