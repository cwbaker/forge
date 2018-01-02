
# TargetPrototype

*TargetPrototype* { _name_, _bind_type_ [, _functions_*]  }

Create a new target prototype that can be used to create new targets during a traversal.

The name parameter specifies the name that the target prototype is referred to in 
buildfiles.  By convention this should be the same as the name of the
Lua value that it is assigned to.

The bind_type parameter specifies how the targets for this target prototype bind to files.  It can be any of `BIND_PHONY` to not bind to any file, `BIND_DIRECTORY` to bind to a directory, `BIND_SOURCE_FILE` to bind to a source file that must exist, `BIND_INTERMEDIATE_FILE` to bind to a source file that need not exist or a generated source file, or `BIND_GENERATED_FILE` to bind to a generated file.

A bind type of `BIND_PHONY` binds targets as phonies.  They are targets that aren't bound to any file or directory; are outdated if any of their dependencies are outdated; and have the timestamp of their newest dependency.

A bind type of `BIND_DIRECTORY` binds targets to directories. The target is bound to a directory; is outdated if the directory doesn't exist; and has its timestamp set to the earliest possible time so that it won't cause any of the targets that depend on the directory target to be outdated (as they simply require that the directory exists they don't care if the directory is newer or not).

A bind type of `BIND_SOURCE_FILE` or `BIND_INTERMEDIATE_FILE` binds targets to intermediate or source files.  The targets are bound to files; their timestamp is set to the latest timestamp of the file that they are bound to and all their dependencies; they are outdated if any of their dependencies are outdated or if any of their dependencies are newer than they are.  Targets that are bound as source files must also exist and an error will be 
thrown if they aren't found.

A bind type of `BIND_GENERATED_FILE` binds targets to generated files.  The targets are bound to files; their timestamp is set to the last write time of the file that they are bound to; they are outdated if any of their dependencies are newer than they are.

The bind type can be overridden on a per target basis by calling `Target:set_bind_type()` for the targets that need to have bind types different to that specified by their rules.
