---
layout: page
title: Rules
parent: More Details
nav_order: 10
---

Rules define the behavior for targets as they are created, add dependencies, are built during a build, or removed during a clean.  Rules are to targets as classes are to objects in an object oriented language such as C++ or Java.

Create a rule by calling `Rule()` and passing the name of the rule.  This returns the rule as a Lua table that you can define functions on to provide target behavior.  Rules should be defined in and returned from a stand-alone Lua script and loaded using `require()`.

For example the `Directory` rule defines targets that create a directory with a name matching their identifier when outdated and are not removed in the clean pass:

~~~lua
local Directory = Rule( 'Directory' );

function Directory.create( toolset, id )
    local target = toolset:File( id, Directory );
    target:set_cleanable( false );
    return target;
end

function Directory.build( toolset, target )
    mkdir( target );
end

return Directory;
~~~

The `create()` and `depend()` functions interact with the declarative syntax seen in buildfiles.  The `build()` and `clean()` functions can be provided to interact with the build and clean actions in a build:

- `Rule.create()` is called whenever a rule is called to create a new target.  This captures calls of the form *Rule* **'** *identifier* **'** to create new targets.

- `Rule.depend()` is called when a target is called to add dependencies or settings.  This captures calls of the form *target* **{** *dependencies* **}** that add dependencies or further define a target.

- `Rule.build()` is called when an outdated target is visited as part of a build pass.  The function should build the file(s) represented by the target.

- `Rule.clean()` is called when a target is visited as part of a clean pass.  The function should remove files that were generated during a build.  The default clean action for targets marked as cleanable is to delete the file(s) represented by the target, a custom function is only needed if cleaning a target needs to do more than remove file(s).

All of these functions are optional.  A function need only be provided when there needs to be behavior different from the default.  The `build()` function is most commonly provided as this updates a target, almost all rules define this function.  The `create()` function is also reasonably common, this determines how targets are created when they appear in a buildfile or build script.

There are convenience functions to create rules that already define `create()` and `depend()` to build single files ([File Rules](file-rules.md)), generate filenames from patterns based on source filenames ([Pattern Rules](pattern-rules.md)), and generate groups of targets to be passed to a single invocation of a tool ([Group Rules](group-rules.md)).
