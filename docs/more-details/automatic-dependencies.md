---
layout: page
title: Automatic Dependencies
parent: More Details
nav_order: 140
---

- TOC
{:toc}

Forge can detect implicit dependencies by hooking operating system calls to open files while a tool is being executed.  Hooking is enabled when a dependency filter is passed to `execute()`.  Calls to open files in the spawned the paths of any files opened for reading or writing are passed back to the dependency filter function.

The dependencies filter is a function that Forge calls for each file that the spawned process opens for reading or writing.  The first argument passed is `line` containing the value `== read '...'` or `== write '...'` to indicate that a file has been opened for reading or writing.  The `...` is replaced by the path to the file.

The path to the file is the path that was passed to open the file.  If relative it is relative to the current working directory of the spawned process which is usually the same as the working directory of the target that the spawned process is being invoked from.  In any case if the spawned process doesn't change the working directory then everything should work fine.  If the path is absolute then this is also fine.

Any extra arguments passed to `execute()` are forwarded on to the dependency filter.  This is an easy way to pass extra context through to filter functions when needed.  Context can also be bound by creating a closure in Lua which is lexically scoped.

The convenience function `Toolset:dependencies_filter()` shows how a dependency filter that captures context in a Lua closure is implemented.  This convenience function detects all files opened for reading, adding those that are within the project source tree as dependencies of the target passed to build it:

~~~lua
-- Add dependencies detected by the injected build hooks library to the
-- target /target/.
function Toolset:dependencies_filter( target )
    target:clear_implicit_dependencies();
    return function( line )
        if line:match('^==') then
            local READ_PATTERN = "^== read '([^']*)'";
            local path = line:match( READ_PATTERN );
            if path then
                local relative_path = relative( absolute(path), root() );
                local within_source_tree = is_relative(relative_path) and relative_path:find( '..', 1, true ) == nil;
                if within_source_tree then
                    local file = self:SourceFile( path );
                    target:add_implicit_dependency( file );
                end
            end
        else
            print( line );
        end
    end
end
~~~

A second utility function, `Toolset:filenames_filter()`, provides the same functionality with respect to dependencies but also detects files opened for writing and adds them as filenames to the target it is invoked for.

The convenience functions aren't the filter themselves.  They create and return a dependency filter as a closure, with the target passed in captured as extra context.

## Caveats

Automatic dependency tracking comes with some caveats!  On Windows dependencies are only automatically tracked for 64 bit processes.  On macOS dependencies are only automatically tracked for non-system executables (it is possible to disable the security feature that is behind this limitation).
