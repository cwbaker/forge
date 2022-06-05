---
layout: page
title: File System Functions
parent: Reference
nav_order: 2
---

- TOC
{:toc}

## Overview

These functions operate directly on files and/or directories in the file system.  See [Directory Functions](directory-functions.md) and [Path Functions](path-functions.md) for functions that work with Forge's directory stack and manipulate paths independently of the file system.

File system operations made from Forge during a post-order graph traversal are synchronized by the ordering implied by dependencies.  As usual when dealing with the file system there is no synchronization with other processes that might be working with the same files or directories.

## Functions

### cp

~~~lua
function cp ( source, destination )
~~~

Copy a file from `source` to `destination`.

It is an error to try and copy to an already existing file.  Use `rm()` to remove any existing file when it is expected that there is already a file at the path specified by `destination`.

**Parameters:**

- `source` the path to the file to copy from
- `destination` the path to file to copy to

### exists

~~~lua
function exists ( path )
~~~

Check whether or not the file or directory at path exists.  Returns true if 
path refers to an existing file or directory otherwise false.

**Parameters:**

- `path` the path to check for existence

**Returns:**

True if there is a file system entry (file, directory, or otherwise) at `path` otherwise false.

### find

~~~lua
function find ( path )
~~~

Recursively list the contents of path and any descended directories.

The directory passed in is assumed to refer to a directory and its contents and descendents returned as an iterator.  Relative paths are relative to the current working directory.

Glob patterns are not used - any filtering based on pattern matching must be done by the caller as each entry in the directory tree is returned.

**Parameters:**

- `path` the path to recursively find files within and beneath

**Returns:**

An iterator that recursively iterates over files within and beneath the directory specified by `path`.

### is_directory

~~~lua
function is_directory ( path )
~~~

Check whether or not path is a directory.

**Parameters:**

- `path` the path to check for being a directory

**Returns:**

True if `path` is a directory otherwise false.

### is_file

~~~lua
function is_file ( path )
~~~

Check whether or not a path is a file.

**Parameters:**

- `path` the path to check for being a file

**Returns:**

True if `path` is a file otherwise false.

### ls

~~~lua
function ls ( path )
~~~

List the contents of path (which is assumed to be a directory).

The path passed in is assumed to refer to a directory and its contents are returned as an iterator.  Relative paths are relative to the current working directory.

Glob patterns are not used - any filtering based on pattern matching must be done by the caller as each entry in the directory is returned.

**Parameters:**

- `path` the path to list files within

**Returns:**

An iterator that recursively iterates over files within the directory specified by `path`.

### mkdir

~~~lua
function mkdir ( path )
~~~

Make a directory at `path`.

Relative paths are relative to the current working directory.  Intermediate directories in `path` that do not already exist are also created.

**Parameters:**

- `path` the path to the directory to create

### rm

~~~lua
function rm ( path )
~~~

Remove the file or directory at `path`.

When removing a directory only the directory itself is removed.  Attempting to remove a directory that still contains files or other directories generates an error.

**Parameters:**

- `path` the path to the file or directory to remove

### rmdir

~~~lua
function rmdir ( path )
~~~

Remove the directory and content at `path`.

Recursively removes a directory and all of its content.  Be careful!

**Parameters:**

- `path` the path to the directory to remove

### touch

~~~lua
function touch ( path )
~~~

Update the timestamp of the file at *path* to the current time.

**Parameters:**

- `path` the path to the file to touch
