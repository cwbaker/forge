---
layout: page
title: Directory Scanning
parent: More Details
nav_order: 1
---

- TOC
{:toc}

## Directory Scanning

The other scenario in which is it useful to depend on a directory is to update one or more targets when that directory changes.  In this case the dependency is an explicit, or in rare cases implicit dependency rather than an ordering dependency.

A directory's timestamp changes when that directory is modified -- that is when files are added to or removed from the directory.  This is exactly the circumstances under which we wish to re-scan the directory when generating targets/part of the dependency graph, by scanning the file system.

Targets depending on a directory because they're implicitly created by scanning that directory are different from targets depending on a directory because they're going to be generated into that directory.  In the latter case the dependency is an *ordering dependency* -- that is a dependency that causes the dependency to be built first but without considering any dependent targets outdated if the dependency (directory) is newer.

Depending on the target representing the current working directory is useful to force a rebuild when the contents of that directory change. This is true of depending on directories in general.

Any target whose dependencies are generated entirely or in part by directory scanning with `ls()` or `find()` should depend on the scanned directories. The depending target will become outdated when files or directories are added to or removed from the scanned directories.

