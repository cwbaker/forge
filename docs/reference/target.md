
# Target

*Target:id* ()

Get the identifier of this target.  Returns the identifier of this target.

*Target:path* ()

Get the full path of this target.  Returns the full path of this target.

*Target:directory* ()

Get the directory part of the path of this target.  Returns the directory part of the path of this target.

*Target:parent* ()

Get the parent of this target.  Returns the parent of this target.  This is the target's parent in the hierarchical namespace of targets not the target's
parent in the dependency graph (of which there may be many).

*Target:prototype* ()

Get the target prototype for this target.  Returns the target prototype for this target or nil if this target was implicitly created as a working directory for other targets or doesn't have a target prototype.

*Target:set_bind_type* ( _bind_type_ )

Set the bind type for this target.  This overrides the bind type specified by a target's TargetPrototype.  See the TargetPrototype documentation for more details on the different bind types.  The bind type passed to a target can also be `BIND_NULL` to set the target to use the bind type specified by its TargetPrototype.

*Target:get_bind_type* ()

Get the bind type for this target.

*Target:set_required_to_exist* ( _required_to_exist_ )

Set whether or not this target is required to be bound to an existing file or not.  The check for existence is done when targets are bound and an error is reported for any targets that have required to exist set to true but are bound to files that don't exist.

*Target:is_required_to_exist* ()

Is this target required to be bound to an existing file?  Returns true if this target is required to be bound to an existing file otherwise false.

*Target:set_timestamp* ( _timestamp_ )

Set the timestamp of this target to timestamp.

The timestamp is the time that is used to determine whether generated targets are outdated with respect to their dependencies. targets that are bound as generated files that have any dependencies with a timestamp later than theirs are considered to be outdated and in need of update.

*Target:get_timestamp* ()

Get the timestamp of this target. 

If this target isn't bound to a file then the last write time is always the beginning of the epoch - January 1st, 1970, 00:00 GMT.  Because this is the oldest possible timestamp this will leave unbound targets always needing to be updated.

Returns the timestamp of this target.

*Target:set_outdated* ( _outdated_ )

Set whether or not this target is outdated.

*Target:is_outdated* ()

Is this target outdated?  Returns true if this target is outdated otherwise false.

*Target:set_filename* ( _filename_ )

Set the filename of this target to filename.  This is the name of the file that this target will attempt to bind to during the bind traversal.

*Target:get_filename* ()

Get the filename of this target.  Returns the filename of this target or an empty string if this target hasn't been bound to a file.

*Target:set_working_directory* ( _target_ )

Set the working directory of this target to target or to the root directory of its containing graph if target is nil.

The working directory is the target that specifies the directory that files specified in scripts for the target are relative to by default and that any commands executed by the target's script functions are started in.  If a target has no working directory then the root target is used instead.

*Target:get_working_directory* ()

Get the working directory of this target.  Returns the working directory of this target or nil if this target doesn't have a working directory.

*Target:add_dependency* ( _dependency_ )

Add a dependency to this target.  Cyclic dependencies are not valid but are not reported by this function.  If a cyclic dependency is detected during a traversal then it is silently ignored.  If dependency is nil then this function will silently do nothing.

*Target:get_dependencies* ()

Iterate over the dependencies of this target.  Returns an iterator over all of the dependencies of this target.
