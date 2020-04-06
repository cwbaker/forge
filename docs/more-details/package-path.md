---
layout: page
title: Package Path
parent: More Details
nav_order: 2
---

Modify the `package.path` variable to control the list of paths searched for modules by the `require()` function from *forge.lua* and load modules (usually toolset prototypes) from directories other than or in addition to those provided by Forge itself.

The Forge project sets the package path to load the Forge build scripts from within the source tree rather than using the default path relative to the Forge executable so that they can be modified and immediately used and tested during development:

~~~lua
package.path = ('%s;%s'):format(
    root('src/forge/lua/?.lua'),
    root('src/forge/lua/?/init.lua')
);
~~~

A more typical usage is to add search paths to search in addition to those loaded by default.  This allows modules (again usually toolsets) to be loaded to define custom, per-project toolsets used for custom tools used in for a project.
