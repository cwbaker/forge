---
layout: page
title: Adding Tools
parent: Getting Started
nav_order: 4
---

- TOC
{:toc}

## Rules

Support new tools by creating rules that define what happens when targets using that tool are built and setting them within a toolset.

The `Copy` rule for copying files is defined in the following snippet.  Replacing the calls to `rm()` and `cp()` with a call to `run()` will execute an external tool.  Command line arguments can be determined by settings in the toolset, the target, and its dependencies.

~~~lua
local Copy = PatternRule( 'Copy' );

function Copy.build( toolset, target )
    rm( target );
    cp( target, target:dependency() );
end

return Copy;
~~~

Rules also allow for overriding behavior when creating targets, adding dependencies, and what is removed in a clean pass.  More details are covered in [Rules](rules.md).

## Loading Rules

Rules are usually defined in separate files and loaded using Lua's `require()` function.  For example setting the copy rule in a toolset looks like:

~~~lua
local toolset = forge.Toolset();
toolset.Copy = require 'forge.Copy';
~~~

The `require()` function searches the paths specified in `package.path` for Lua scripts that match the module name.  Once the matching source is found it is loaded and executed and its return value returned to the calling code.

Set `package.path` at the top of the build script if necessary.  Generate paths relative to the project root and Forge executable with the `root()` and `executable()` functions respectively.  Preserve Forge's default search path by prepending the existing value of `package.path` if you wish to use the default scripts provided with Forge, overwrite `package.path` to replace the built-in scripts with new ones.

Tools often require user-defined settings and may need to auto-detect some configuration such as the location of executables on the local machine etc.  To support this rules, default settings, and one-off detection of configuration can be bundled up into a module.  See [Modules](modules.md) for more details.
