
# Functions

## Configuration Functions

*set_maximum_parallel_jobs* ( _maximum_parallel_jobs_ )

Set the maximum number of commands allowed to execute at once.  The minimum
is 1 and the maximum is 64.  The default is 1.

*get_maximum_parallel_jobs* ()

Get the maximum number of commands that can be executed at once.

*set_stack_trace_enabled* ( _stack_trace_enabled_ )

Set whether or not stack traces are displayed when errors occur.  The default
is false.

*is_stack_trace_enabled* ()

Are stack traces displayed when errors occur?  Returns true if stack traces
are enabled otherwise false.

## Environment Functions

*getenv* ( _name_ )

Get the value of an environment attribute.  If the environment attribute doesn't exist then this function returns nil.

*putenv* ( _attribute_, _value_ )

Set the value of an environment attribute.  Setting an environment attribute to the empty string clears that environment attribute.

## File System Functions

*cp* ( _source_, _destination_ )

Copy a source file to a destination file.

*cpdir* ( _source_, _destination_ )

Recursively copy source to destination.  Recursively copies newer files from 
the source directory to the destination directory.  Directories and files that
start with a dot (.) are ignored.

*exists* ( _path_ )

Check whether or not the file or directory at path exists.  Returns true if 
path refers to an existing file or directory otherwise false.

*find* ( _path_ )

Recursively list the contents of path and any descended directories. If path
is relative then it is treated as being relative to the current working 
directory.  The directory passed in is assumed to be a directory and its 
contents returned as an iterator.  Glob patterns are not used - any filtering
based on pattern matching must be done by the caller as each entry in the 
directory tree is returned.

*is_directory* ( _path_ )

Check whether or not path is a directory.  Returns true if path is a directory
otherwise false.

*is_file* ( _path_ )

Check whether path is a file.  Returns true if path is a file otherwise false.

*ls* ( _path_ )

List the contents of path (which is assumed to be a directory).  If path is 
relative then it is treated as being relative to the current working 
directory.  The directory passed in is assumed to be a directory and its 
contents returned as an iterator.  Glob patterns are not used - any filtering
based on pattern matching must be done by the caller as each entry in the 
directory is returned.

*mkdir* ( _path_ )

Make the directory path.  If path is relative it is treated as being relative
to the current working directory.  Any intermediate directories specified in
the directory passed in that do not already exist are also created.

*rm* ( _path_ )

Remove a file.

*rmdir* ( _path_ )

Recursively remove a directory.  Recursively removes a directory and all of
its content.  Be careful!

## Operating System Functions

*execute* ( command, arguments, filter )

Executes _command_ passing _arguments_ as the command line and optionally using _filter_ to process the output. 

The command will be executed in a thread and processing of any jobs that can be performed in parallel continues.  Returns the value returned by command when it exits.  The _filter_ parameter is optional and can be nil to pass the output through to the console unchanged.

*hostname* ()

Get the hostname of the computer that the build tool is running on.  Returns the hostname.

*operating_system* ()

Return a string that identifies the operating system that the build tool is being run on - either "windows" or "macosx".

*print* ( _text_ )

Print _text_ to stdout.

*sleep* ( _duration_ )

Do nothing for _duration_ milliseconds.

*whoami* ()

Get the name of the user account that the build tool is running under.  Returns the user name.

## Path and String Functions

*absolute* ( _path_, [_working_directory_] )

Convert path into an absolute path by prepending _working_directory_ or the 
current working directory if _working_directory_ is not specified.  If the 
path is already absolute then it is returned unchanged.

*basename* ( _path_ )

Return the basename of _path_ (everything except for the extension of which 
the dot "." is considered part, i.e. the dot "." is not returned as part of 
the basename).

*branch* ( _path_ )

Return all but the last element of _path_.

*extension* ( _path_ )

Return the extension of _path_ (including the dot ".").

*home* ( [_path_] )

Convert path into a directory relative to the current user's home directory.
If path is omitted then the current user's home directory is returned.

*initial* ( [_path_] )

Convert path into a directory relative to the directory that the build tool
was invoked from.  If path is omitted then the initial directory is returned.

*is_absolute* ( _path_ )

Check whether or path is absolute.  Returns true if _path_ is absolute 
otherwise false.

*is_relative* ( _path_ )

Check whether or path is relative.  Returns true if _path_ is relative 
otherwise false.

*leaf* ( _path_ )

Return the last element of _path_.

*lower* ( _value_ )

Convert value to lower case.  Returns _value_ converted to lower case letters.

*native* ( path )

Convert path into its native equivalent.  Returns _path_ converted into its 
native equivalent.

*relative* ( _path_, [_working_directory_] )

Convert path into a path relative to _working_directory_ or relative to the 
current working directory if _working_directory_ is not specified.  If the 
path is already relative then it is returned unchanged.

*root* ( [_path_] )

Convert _path_ into a path relative to the directory that the `build.lua` file
was found in when searching up from the initial directory.  If _path_ is 
omitted then the root directory is returned.

*upper* ( _value_ )

Convert value to upper case.  Returns _value_ converted to upper case letters.

## Working Directory Functions

*cd* ( _path_ )

Change the current working directory to _path_.  If _path_ is relative it is treated as being relative to the current working directory.

*popd* ()

Pop the current working directory and restore the working directory to the working directory saved by the most recent call to `pushd()`.  If the current working directory is the only directory on the directory stack then this function silently does nothing.

*pushd* ( _path_ )

Push the current working directory (so that it can be returned to later by calling `popd()`) and set the new current working directory to _path_.  If _path_ is relative it is treated as being relative to the current working directory.

*pwd* ()

Get the current (present) working directory.  Returns the current working directory.

## Graph Functions

*bind* ( [_target_] )

Bind the targets to files and set their outdated status and timestamps accordingly.  Returns the number of targets that failed to bind because their files were expected to exist but didn't (see `Target:set_required_to_exist()`).

*buildfile* ( _filename_ )

Load a buildfile from _filename_.

*find_target* ( _id_ )

Find a target in this Graph whose identifier matches _id_.  If _id_ is a relative path then it is treated as being relative to the current working directory.

*load_binary* ( _filename_ )

Load a previously saved dependency graph from _filename_.

*save_binary* ( _filename_ )

Save the current dependency graph to _filename_.

*load_xml* ( _filename_ )

Load a previously saved dependency graph from _filename_.

*save_xml* ( _filename_ )

Save the current dependency graph to _filename_.

*postorder* ( _visitor_, _target_ )

Perform a postorder traversal of targets calling the _visitor_ function for each target visited.  Returns the number of visited targets that failed because they generated an error during their visitation.

*print_dependencies* ( [_target_] )

Print the dependency tree of Targets in this Graph.  If target is nil then dependencies from the entire Graph are printed otherwise dependencies of target are recursively printed.

*print_namespace* ()

Print the namespace of Targets in this Graph.  If target is nil then the namespace of the entire Graph is printed otherwise only Targets that are descended from target are printed.

*target* ( [_id_], [_target_prototype_], [_table_] )

Create or return an existing target.

The _id_ parameter specifies a target path that uniquely identifies the target to create or return.  If _id_ specifies a relative path then it is considered relative to the current working directory.  If _id_ is the empty string or nil then an anonymous target is created using a unique identifier to create a target in the current working directory.

The _target_prototype_ parameter specifies the target prototype to use when creating the target.  It is an error for the same target to be created with more than one target prototype.  If the _target_prototype_ parameter is omitted or nil then a target without any target prototype is created.

The _table_ parameter specifies the table to use to represent the target in the Lua virtual machine.  If the _table_ parameter is omitted or nil then an empty table is created and used.
