---
layout: page
title: Path and String Functions
parent: Reference
nav_order: 4
---

- TOC
{:toc}

## Overview

Path functions operate on paths expressed as strings, they have no interaction with the filesystem.  Unless stated the path format is always a portable representation with forward slashes ("/").  Back slashes ("\\") are supported in all situations when parsing and are generated from the `native()` function on Windows.

## Functions

### absolute

~~~lua
function absolute( path, working_directory )
~~~

Convert a path into an absolute path.

Converts `path` into an absolute path by prepending `working_directory` or the
current working directory if `working_directory` is not specified.  If `path` is already absolute then it is returned unchanged.

**Parameters:**

- `path` a relative path to make absolute
- (optional) `working_directory` the path that `path` is relative to

**Returns:**

The relative path `path` expressed as an absolute path or `path` itself if it was already absolute.

### basename

~~~lua
function basename( path )
~~~

Get the basename of a path.

Extract the basename from `path` (everything except for the extension of which
the dot "." is considered part, i.e. the dot "." is not returned as part of
the basename).

**Parameters:**

- `path` the path to extract the basename from

**Returns:**

The basename of `path`.

### branch

~~~lua
function branch( path )
~~~

Get the branch portion of a path.

The branch portion of a path is all elements excluding the last, right-most element.

**Parameters:**

- `path` the path to extract the branch portion from

**Returns:**

Returns a path corresponding to `path` with last element removed.

### extension

~~~lua
function extension( path )
~~~

Get the extension from a path.

Extract the extension portion of `path` (including the period ".").  If there is no period in `path` then returns the empty string.  Multiple dots will return the right-most period and trailing characters.  A trailing period will return the string ".".

**Parameters:**

- `path` the path to extract the extension from

**Returns:**

The extension portion of `path` including the leading period ".".

### home

~~~lua
function home( path )
~~~

Convert a path into an absolute path relative to the current user's home directory.

If the optional `path` parameter is omitted then the current user's home directory is returned.  If `path` already specifies an absolute directory then it is returned unchanged.

**Parameters:**

- `path` the relative path to make absolute relative to the home directory

**Returns:**

The relative path `path` expressed as an absolute path relative to the current user's home directory.

### initial

~~~lua
function initial( path )
~~~

Convert a path into an absolute path relative to the directory that Forge was started from.

If the optional `path` parameter is omitted then the initial directory is returned.  If `path` already specifies an absolute directory then it is returned unchanged.

**Parameters:**

- `path` the relative path to make absolute relative to the initial directory

**Returns:**

The relative path `path` expressed as an absolute path relative to the initial directory.

### is_absolute

~~~lua
function is_absolute( path )
~~~

Is a path absolute?

Paths that start with a leading slash or drive specifier (e.g. "C:", "D:", etc on Windows) are considered to be absolute.  All other paths are considered relative.

**Parameters:**

- `path` the path to check for being absolute or not

**Returns:**

True if `path` is absolute otherwise false.

### is_relative

~~~lua
function is_relative( path )
~~~

Is a path relative?

Paths that start with a leading slash or drive specifier (e.g. "C:", "D:", etc on Windows) are considered to be absolute.  All other paths are considered relative.

**Parameters:**

- `path` the path to check for being relative or not

**Returns:**

True if `path` is relative otherwise false.

### leaf

~~~lua
function leaf( path )
~~~

Get the leaf portion of a path.

The leaf portion of a path is the last element.

**Parameters:**

- `path` the path to extract the leaf portion from

**Returns:**

Returns the last element of `path`

### lower

~~~lua
function lower( value )
~~~

Convert a string to lower case.

**Parameters:**

- `value` the string to convert to lower case

**Returns:**

The string `value` converted to lower case.

### native

~~~lua
function native( path )
~~~

Convert a path to its native form.

This converts paths to use back slashes ("\\") on Windows.  Paths on Linux and macOS match their portable forms using forward slashes ("/") and are returned unchanged.

**Parameters:**

- `path` the path to convert to native form

**Returns:**

The path specified by `path` converted to native form.

### relative

~~~lua
function relative( path, working_directory )
~~~

Convert a path to be relative to another.

Convert `path` to be relative to `working_directory` or relative to the current working directory if `working_directory` is nil or not specified.  If `path` is already relative then it is returned unchanged.

**Parameters:**

- `path` the absolute path to convert to a relative path
- (optional) `working_directory` the path to convert `path` to be relative to

**Returns:**

The absolute path `path` expressed relative to `working_directory` or the current working directory if `working_directory` is nil or not specified.

### root

~~~lua
function root( path )
~~~

Convert a path into an absolute path relative to the root directory.

Converts `path` to be relative to the root directory.  That is the directory containing the root build script, *forge.lua*, that was found when searching up from the initial directory when `forge` was started.

If the optional `path` parameter is omitted then the initial directory is returned.  If `path` already specifies an absolute directory then it is returned unchanged.

**Parameters:**

- `path` the relative path to make absolute relative to the root directory

**Returns:**

The relative path `path` expressed as an absolute path relative to the root directory.

### script

~~~lua
function script( path )
~~~

Convert a path into an absolute path relative to the calling Lua script.

Converts `path` to be absolute by prepending the current script's directory.

If the optional `path` parameter is omitted then the calling script's directory is returned.  If `path` is already absolute then it is returned unchanged.

**Parameters:**

- `path` the path to make absolute relative to the script directory

**Returns:**

The path `path` expressed as absolute relative to the calling Lua script's directory.

### upper

~~~lua
function upper( value )
~~~

Convert a string to upper case.

**Parameters:**

- `value` the string to convert to upper case

**Returns:**

The string `value` converted to upper case.
