---
layout: page
title: Versioning
parent: More Details
nav_order: 6
---

### Versioning

Generating version numbers for built components can be rolled any which way you like.  Here the current date, time, and variant is used.  Other possibilities include passing version on the command line if you already have an external means of generating a version number, executing external processes from within the buildfile, or generating a version number from a Lua program running within Forge.

~~~lua
local version = ('%s %s'):format( os.date('%Y.%m.%d %H:%M:%S'), variant or 'debug' );
~~~

The generated version number is passed into the build later as a pre-processor macro.
