---
layout: page
title: File System Functions
parent: Reference
nav_order: 4
---

- TOC
{:toc}

### cp

~~~lua
function cp ( source, destination )
~~~

Copy a source file to a destination file.

### exists

~~~lua
function exists ( path )
~~~

Check whether or not the file or directory at path exists.  Returns true if 
path refers to an existing file or directory otherwise false.

### find

~~~lua
function find ( path )
~~~

Recursively list the contents of path and any descended directories. If path
is relative then it is treated as being relative to the current working 
directory.  The directory passed in is assumed to be a directory and its 
contents returned as an iterator.  Glob patterns are not used - any filtering
based on pattern matching must be done by the caller as each entry in the 
directory tree is returned.

### is_directory

~~~lua
function is_directory ( path )
~~~

Check whether or not path is a directory.  Returns true if path is a directory
otherwise false.

### is_file

~~~lua
function is_file ( path )
~~~

Check whether path is a file.  Returns true if path is a file otherwise false.

### ls

~~~lua
function ls ( path )
~~~

List the contents of path (which is assumed to be a directory).  If path is 
relative then it is treated as being relative to the current working 
directory.  The directory passed in is assumed to be a directory and its 
contents returned as an iterator.  Glob patterns are not used - any filtering
based on pattern matching must be done by the caller as each entry in the 
directory is returned.

### mkdir

~~~lua
function mkdir ( path )
~~~

Make the directory path.  If path is relative it is treated as being relative
to the current working directory.  Any intermediate directories specified in
the directory passed in that do not already exist are also created.

### rm

~~~lua
function rm ( path )
~~~

Remove a file.

### rmdir

~~~lua
function rmdir ( path )
~~~

Recursively remove a directory.  Recursively removes a directory and all of
its content.  Be careful!

### touch

~~~lua
function touch ( path )
~~~

Update the timestamp of the file at *path* to the current time.
