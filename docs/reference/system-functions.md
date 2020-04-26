---
layout: page
title: System Functions
parent: Reference
nav_order: 6
---

- TOC
{:toc}

### execute

~~~lua
function execute( command, arguments, filter )
~~~

Executes `command` passing `arguments` as the command line and optionally using `filter` to process the output. 

The command will be executed in a thread and processing of any jobs that can be performed in parallel continues.  Returns the value returned by command when it exits.  The `filter` parameter is optional and can be nil to pass the output through to the console unchanged.

The `execute()` function suspends execution of the Lua coroutine that it is called on until the executed process finishes.  This leads to race conditions when the results of multiple `execute()` calls update shared data without proper synchronization (e.g. calling `wait()`).  This usually occurs when using `execute()` outside of a traversal.

Note that use of `execute()` within a traversal orders by dependencies and has barriers in place to ensure that targets aren't visited until all of their dependencies have been successfully visited.  So long as shared data isn't updated (uncommon during a traversal) there should be no problem.

### forge_hooks_library

~~~lua
function forge_hooks_library()
~~~

Returns the currently set Forge hooks library.  This library is injected into executed processes to intercept open calls and detect implicit dependencies.

### getenv

~~~lua
function getenv( name )
~~~

Get the value of the environment variable `name`.  If the environment variable isn't set then this function returns nil.

### hash

~~~lua
function hash( table )
~~~

Calculate the order independent hash of the fields in `table`.

### operating_system

~~~lua
function operating_system()
~~~

Return a string that identifies the operating system that Forge is running on - "linux", windows", or "macos".

### print

~~~lua
function print( text )
~~~

Print `text` to stdout.

### set_forge_hooks_library

~~~lua
function set_forge_hooks_library( forge_hooks_library )
~~~

Set the path to the Forge hooks library.  The default is *libforge_hooks.so* on Linux, *forge_hooks.dylib* on macOS, and *forge_hooks.dll* on Windows all within the same directory as the running Forge executable.

Pass an empty string in `forge_hooks_library` to disable the use of hooking open calls to trace dependencies when executing external processes.

### sleep

~~~lua
function sleep( duration )
~~~

Do nothing for `duration` milliseconds.

### ticks

~~~lua
function ticks()
~~~

Return the number of ticks elapsed since Forge started.
