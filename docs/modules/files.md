
# Files

# SourceFile

The *SourceFile* target prototype defines a source file that must exist.  It 
doesn't generally appear in buildfiles but is used by the build system to 
expand other targets during the "load", "static_depend", and "depend" passes.

# HeaderFile

The *HeaderFile* target prototype defines a header file (or source file that 
doesn't need to exist).  It doesn't generally appear in buildfiles but is used
by the build system to expand other targets during the "load", 
"static_depend", and "depend" passes.

# File

The *File* target prototype defines a generated file.  It doesn't generally 
appear in buildfiles but is used by the build system to expand other targets
during the "load", "static_depend", and "depend" passes.
