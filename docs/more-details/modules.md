---
layout: page
title: Modules
parent: More Details
nav_order: 20
---

- TOC
{:toc}

Modules provide a way to encapsulate related rules, default settings, and detection of tools installed on a machine into a package that can be reused between multiple projects.

Create a module by creating a Lua table containing an `install()` function.  The `install()` function accepts a toolset in which the module should set rules and default settings that it supports.  The rules are used to create targets for the tools the module supports.  The default settings provide default values for any settings that weren't already set by the caller, usually the build script `forge.lua`.  Modules should be returned from a stand-alone Lua script and loaded using `require()`.

For example the following script defines the `lalr` module in [forge/lalr/init.lua](https://github.com/cwbaker/lalr/blob/main/lalr/forge/lalr/init.lua):

~~~lua
local lalr = {};

function lalr.configure( toolset, lalr_settings )
    return {
        lalrc = lalr_settings.lalrc or '${bin}/lalrc';
    };
end

function lalr.install( toolset )
    local settings = toolset:configure_once( 'lalr',
        lalr.configure
    );
    assert( exists(settings.lalrc) );
    toolset.Lalrc = require( 'forge.lalr.Lalrc' );
    return true;
end

return lalr;
~~~

## Detecting Tools

The function `Toolset.configure_once()` is a convenience to allow modules to perform detection of tools that might take an unreasonable amount of time if run for every build, e.g. scanning the file system or executing processes to find tools etc.  It accepts the unique name of the settings to cache and a function to generate the settings if they don't exist.

Settings are cached into the file `local_settings.lua` stored in the cache directory.  If the cache directory is per-variant then so are the local settings.

## Installing into a Toolset

The `lalr` module is loaded by calling `require()` and installed into any toolsets that need it from the build script.  In the example below the `lalr` module installed into the same toolset used to build C/C++ source:

~~~lua
local forge = require( 'forge.cc' ):load( variant );

local cc = forge.Toolset 'cc_${platform}_${architecture}' {
    -- Settings to initialize cc and lalr toolsets here...
};

local lalr = require 'forge.lalr';
cc:install( lalr );
~~~
