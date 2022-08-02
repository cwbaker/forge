---
layout: page
title: Group Rules
parent: More Details
nav_order: 50
---

- TOC
{:toc}

`GroupRule()` creates targets with filenames generated from source filenames and groups them as dependencies of a single, anonymous target.  This allows multiple targets to be handled from within a single build function.

## Usage

The creation and use of a group rule is identical that of a pattern rule.  The difference comes in how the dependency graph of targets is constructed around those targets.  Pattern targets generate and return their targets directly, those targets becomes dependencies of the target the pattern rule is invoked in.  Group targets generate their targets similarly but add them as dependencies of the single anonymous target that is returned to become a dependency of the target the group rule is invoked in.

~~~lua
local Cxx = GroupRule( 'Cxx', msvc.object_filename );
Cxx.build = msvc.compile_cxx;
toolset.Cxx = Cxx;
~~~

~~~lua
-- ...
toolset:Cxx '${obj}/%1' 
    'EventSink.cpp', 
    'forge.cpp'
};    
-- ...
~~~

The group rule becomes the rule for that anonymous target.  This allows the build action invoked on the anonymous target to iterate over dependencies of the anonymous target as a group.  For example when compiling C or C++ source with Microsoft Visual C++ all changed source files can be passed to one invocation of the compiler as shown by `msvc.compile()` below:

~~~lua
function msvc.compile( toolset, target, language )
    -- ...
    local objects_by_source = {};
    local sources_by_directory = {};
    for _, object in target:dependencies() do
        if object:outdated() then
            local source = object:dependency();
            local directory = branch( source:filename() );
            local sources = sources_by_directory[directory];
            if not sources then
                sources = {};
                sources_by_directory[directory] = sources;
            end
            local source = object:dependency();
            table.insert( sources, source:id() );
            objects_by_source[leaf(source:id())] = object;
            object:clear_implicit_dependencies();
        end
    end
    -- ...
end
~~~
