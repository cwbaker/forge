---
layout: page
title: Conditional Settings
parent: More Details
nav_order: 4
---

### Platform Specific Global Settings

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

### Buildfile Specific Settings and Inheritance

Platform specific settings that apply only to a single buildfile are defined using setting inheritance.

~~~lua
-- Disable warnings on Linux to avoid unused variable warnings in Boost
-- System library headers.
local libraries;
local warning_level = 3;
if operating_system() == 'linux' then
    warning_level = 0;
    libraries = { 
        'pthread', 
        'dl' 
    };
end

for _, forge in toolsets('^cc.*') do
    local forge = forge:inherit {
        subsystem = 'CONSOLE'; 
        stack_size = 32768;
        warning_level = warning_level;    
    };
-- ...
~~~
