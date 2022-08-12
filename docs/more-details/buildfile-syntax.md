---
layout: page
title: Buildfile Syntax
parent: More Details
nav_order: 80
---

- TOC
{:toc}

The language used in build scripts and buildfiles is Lua, officially defined by the [Lua 5.3 Reference Manual](https://www.lua.org/manual/5.3/).  Lua is a small, dynamic language with a syntax that will be mostly familiar to users of other languages.  The few places where Lua's syntax is unusual and relevant within Forge are described in this section.

## Tables

Lua's main structural type is an associative array known as a table that is usable as an array, map, or object interchangeably.

Forge uses tables to represent objects, e.g. rules and targets are tables, and arrays, e.g. as lists of targets added as dependencies of another target.  The Forge source and this user guide use the term table for consistency with Lua and it should be clear from context whether an object or a array is intended.

The index of the first element in a table is 1 and not 0 as you might expect from other languages.  Forge keeps this convention for accesses dependencies of a target, e.g. the call `target:dependency(1)` returns the first dependency of a target.

Tables as objects are able to overload the call operator.  Forge uses this to make rules and targets callable.  Rule calls create targets.  Target calls add dependencies to an existing target.

## Declarative Syntax

The declarative style syntax seen in the build scripts and buildfiles is probably the most foreign seeming to those unfamiliar with Lua's colon operator method calls and optional parentheses on certain calls.  But once those concepts are understood the syntax can easily be interpreted as chained function calls.

Consider the build script to copy the files `{bar,baz,foo}.in` to `output/{bar,baz,foo}.out`:

~~~lua
local forge = require 'forge';
forge:load();

local toolset = forge.Toolset() {
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

From the Lua manual [3.4.10 Function Calls](https://www.lua.org/manual/5.3/manual.html#3.4.10):

> A call of the form f{fields} is syntactic sugar for f({fields}); that is, the argument list is a single new table. A call of the form f’string’ (or f”string” or f[[string]]) is syntactic sugar for f(‘string’); that is, the argument list is a single literal string.

Parentheses are optional for calls with a literal string or table constructor as their only argument.  In Forge this appears in calls to rules to create targets, e.g. *rule* [*identifier* \| **(** *identifier*, *...* **)**], and in calls to targets to add dependencies, e.g. *target* **{** *dependencies* **}**.

Adding the omitted parentheses to the build script:

~~~lua
local forge = require('forge');
forge:load();

local toolset = forge.Toolset()({
    output = root( 'output' );
});

toolset:all({
    toolset:Copy('${output}/%1.out')({
        'bar.in';
        'baz.in';
        'foo.in';
    });
});
~~~

From the Lua Manual - [3.4.10 Function Calls](https://www.lua.org/manual/5.3/manual.html#3.4.10):

> A call v:name(args) is syntactic sugar for v.name(v,args), except that v is evaluated only once.

The colon operator is a call that passes the table on its left-hand side as the first parameter to the call on its right-hand side, where the called function is also looked up as a member of the table on the left-hand side.  Consider this syntax a call to a method on an object.

Expanding method calls in the build script:

~~~lua
local forge = require('forge');
forge.load(forge);

local toolset = forge.Toolset()({
    output = root( 'output' );
});

toolset.all(toolset, {
    toolset.Copy(toolset, '${output}/%1.out')({
        'bar.in';
        'baz.in';
        'foo.in';
    });
});
~~~

These two pieces of syntax, the chaining of the calls to create a target and add dependencies to it, and the creation of dependent targets directly within those dependencies give rise to the declarative style.

## Default Values

In Lua the `or` operator returns its first argument if it is not false or nil; otherwise returning its second argument.  Unassigned variables evaluate to `nil`to give the effect of providing a sensible default for values that haven't already been set.

This appears at the top of `forge.lua` when setting default values for command line variables and often as default values for function parameters.

~~~lua
variant = variant or 'debug';
~~~
