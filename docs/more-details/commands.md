---
layout: page
title: Commands
parent: More Details
nav_order: 10
---

- TOC
{:toc}

Commands are the high-level steps taken in a build, e.g. build, clean, or install.  They are implemented as global functions within Lua and should be defined once the build script has finished executing in order to be successfully used.

Pass commands to carry out on the command line when invoking `forge`.  These commands are called as global functions with no arguments.  Pass arguments, if needed, by setting variables on the command line to have that value assigned to a global of the same name.  Order is important so that variables must be set before commands that use them.

## Build Command

The default build command provided by Forge:

~~~lua
-- Provide global build command.
function build()
    local target = find_initial_target( goal );
    local failures = prepare( target ) + postorder( target, build_visit );
    forge:save();
    printf( "forge: default (build)=%dms", math.ceil(ticks()) );
    return failures;
end
~~~

The `build_visit()` function that visits each target in a build pass calls the `build()` function for each outdated target that provides it.  In this way outdated targets are built and brought up to date:

~~~lua
-- Visit a target by calling a member function "build" if it exists and 
-- setting that Target's built flag to true if the function returns with
-- no errors.
function build_visit( target )
    if target:outdated() then
        local build_function = target.build;
        if build_function then 
            local success, error_message = pcall( build_function, target.toolset, target );
            target:set_built( success );
            if not success then 
                clean_visit( target );
                assert( success, error_message );
            end
        else
            target:set_built( true );
        end
    end
end
~~~

## Reconfigure Command

Commands don't need to process the dependency graph.  They can be a convenient place to put scripts relating to the build.  For example the reconfigure command removes the `local_settings.lua` file in which automatically detected settings are cached:

~~~lua
-- Provide global reconfigure command.
function reconfigure()
    local cache_directory = forge.cache_directory;
    local filename = cache_directory and root( ('%s/local_settings.lua'):format(cache_directory) ) or root( 'local_settings.lua' );
    rm( filename );
    return 0;
end
~~~

## Default Command

The default command, invoked when no other commands are provided on the command line, is the build command.  This is specified indirectly as `default()` but can be overridden by defining `default()` from the build script:

~~~lua
-- Provide global default command that calls through to `build()`.
function default()
    return build();
end
~~~

## Compound Commands

Commands can also call other commands.  Here the install command for Forge calls `default()` to build Forge before copying the required files to the install destination specified by `prefix`:

~~~lua
function install()
    prefix = prefix and root( prefix ) or home( 'forge' );
    local failures = build();
    if failures == 0 then 
        toolset:cpdir( '${prefix}/bin', '${bin}' );
        toolset:cpdir( '${prefix}/lua', 'src/forge/lua' );
    end
end
~~~
