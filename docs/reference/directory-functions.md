---
layout: page
title: Directory Functions
parent: Reference
nav_order: 1
---

- TOC
{:toc}

### cd 

~~~lua
function cd( path )
~~~

Change the current working directory to `path`.  If `path` is relative it is considered relative to the current working directory.

### popd

~~~lua
function popd()
~~~

Pop the current working directory and restore the working directory to the working directory saved by the most recent call to `pushd()`.  If the current working directory is the only directory on the directory stack then this function silently does nothing.

### pushd

~~~lua
function pushd( path )
~~~

Push the current working directory (so that it can be returned to later by calling `popd()`) and set the new current working directory to `path`.  If `path` is relative it is considered relative to the current working directory.

### pwd

~~~lua
function pwd()
~~~

Return the current working directory.  This is Forge's internal current working directory which defaults to the directory containing the currently processed buildfile, the root build script, or the working directory at the time a target was created when visting that target in a traversal (e.g. when `Target.build()` or `Target.clean()` are called).
