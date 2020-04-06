---
layout: page
title: Conditional Settings
parent: More Details
nav_order: 4
---

Override settings globally or locally for specific situations by changing the `settings` table of a toolset explicitly after it has been returned.

Changing settings of the toolset directly affects all targets that are created with that toolset.  Toolsets can also be inherited and cloned to isolate changes to within buildfiles or to provide separate toolsets that duplicate most settings but differentiate others (e.g. machine architecture being the prime example).

The following snippet bumps the C++ standard required from C++11 to C++14 when building for Windows with Microsoft Visual C++ (this is the earliest standard supported by Visual C++):

~~~lua
-- Bump the C++ standard to c++14 when building on Windows as that is the 
-- closest standard supported by Microsoft Visual C++.
local settings = cc.settings;
if settings.platform == 'windows' then
    settings.standard = 'c++14';
end
~~~
