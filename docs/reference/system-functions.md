---
layout: page
title: System Functions
parent: Reference
nav_order: 6
---

- TOC
{:toc}

## Overview

Functions that interact with the operating system.  The most commonly used of these are `execute()` to execute external processes for the build, `print()` to print to the standard output stream, and `operating_system()` to determine the underlying host operating system.

## Functions

### execute

~~~lua
function execute( command, arguments, environment, dependencies_filter, stdout_filter, stderr_filter, ... )
~~~

Executes `command` passing `arguments` as the command line and optionally using `dependencies_filter`, `stdout_filter`, and `stderr_filter` to process the output.

Any other arguments are passed as extra arguments to the filter functions when they process a line of output.

The command will be executed in a thread and processing of any jobs that can be performed in parallel continues.  Returns the value returned by command when it exits.

The filter parameters are optional.  Passing nil for the dependency filter disables automatic dependency detection.  Passing nil to the stdout and/or stderr filters passes output to the appropriate console unchanged.

The `execute()` call suspends processing on the Lua coroutine that it is made on until the executed process completes.  This leads to race conditions when the results of multiple `execute()` calls update shared data without proper synchronization (i.e. calling `wait()`).  This usually occurs when using `execute()` to generate local settings.

Note that use of `execute()` within a traversal orders by dependencies and has barriers in place to ensure that targets aren't visited until all of their dependencies have been successfully visited.  So long as shared data isn't updated (uncommon during a traversal) there should be no problem.

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

### run

~~~lua
function run( command, arguments, environment, dependencies_filter, stdout_filter, stderr_filter, ... );
~~~

Executes `command` as for [`execute()`](#execute) but raises an error if the process exits with a non-zero exit code.

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

### wait

~~~lua
function wait()
~~~

Wait for all currently executing processes to finish.

Used to wait for processes executed to read back configuration settings to complete before attempting to use those settings.
