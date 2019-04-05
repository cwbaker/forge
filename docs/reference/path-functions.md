---
layout: page
title: Path Functions
parent: Reference
nav_order: 6
---

- TOC
{:toc}

### absolute

~~~lua
function absolute( path, working_directory )
~~~

Convert path into an absolute path by prepending `working_directory` or the 
current working directory if the optional `working_directory` parameter is not specified.  If `path` is already absolute then it is returned unchanged.

### basename

~~~lua
function basename( path )
~~~

Return the basename of `path` (everything except for the extension of which 
the dot "." is considered part, i.e. the dot "." is not returned as part of 
the basename).

### branch

~~~lua
function branch( path )
~~~

Return all but the last element of *path*.

### extension

~~~lua
function extension( path )
~~~

Return the extension of `path` (including the dot ".").

### home

~~~lua
function home( path )
~~~

Convert `path` into a directory relative to the current user's home directory.
If `path` is omitted then the current user's home directory is returned.

### initial

~~~lua
function initial( path )
~~~

Convert `path` into a directory relative to the directory that the forge was invoked from.  If `path` is omitted then the initial directory is returned.

### is_absolute

~~~lua
function is_absolute( path )
~~~

Returns true if `path` is absolute otherwise false.

### is_relative

~~~lua
function is_relative( path )
~~~

Returns true if `path` is relative otherwise false.

### leaf

~~~lua
function leaf( path )
~~~

Return the last element of *path*.

### lower

~~~lua
function lower( value )
~~~

Returns `value` converted to lower case.

### native

~~~lua
function native( path )
~~~

Return `path` converted into its native form.

### relative

~~~lua
function relative( path, working_directory )
~~~

Convert `path` to be relative to `working_directory` or relative to the current working directory if `working_directory` is not specified.  If `path` is already relative then it is returned unchanged.

### root

~~~lua
function root( path )
~~~

Convert `path` to be relative to the root directory.  That is the directory containing the *forge.lua* file that was found in when searching up from the initial directory.  If `path` is omitted then the root directory is returned.

### upper

~~~lua
function upper( value )
~~~

Return `value` converted to upper case.
