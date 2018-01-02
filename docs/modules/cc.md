
# C/C++

Sweet Build supports building using various third party tools (compilers, preprocessors, etc) through loading Lua modules that define different target prototypes.

## Settings

## Preprocessor Macros

The following preprocessor macros are defined when compiling C and C++ source.
They allow conditional compilation based on the platform, variant, and module
being built and pass automatically generated version information to the
build.

 - *BUILD_PLATFORM_x* is defined to indicate the platform being built where
 _x_ is replaced by the uppercase platform name.  For example 
 BUILD_PLATFORM_MSVC is defined when building for the "msvc" platform.

 - *BUILD_VARIANT_x* is defined to indicate the variant that is 
being built where _x_ is replaced by the uppercase variant name. For example
BUILD_PLATFORM_RELEASE is defined when building the "release" variant.

 - *BUILD_MODULE_x* is defined to indicate the module being built where _x_
 is replaced by the uppercase executable or library identifier.  For example
BUILD_MODULE_LUA is defined when building the "lua" library. 

 - *BUILD_LIBRARY_TYPE_x* is defined to indicate whether dynamic or 
static libraries are being built where _x_ is replaced by STATIC or 
DYNAMIC for "static" or "dynamic" library types respectively.

 - *BUILD_LIBRARY_SUFFIX* is defined to be the suffix appended to library names 
to distinguish between libraries for different platforms and variants.  For
example BUILD_LIBRARY_SUFFIX is defined as "_msvc_debug.lib" when building
for the "msvc" platform and "debug" variant.

 - *BUILD_VERSION* is defined to be the date, variant, and platform of the 
build.

The preprocessor macro `BUILD_LIBRARY_TYPE_DYNAMIC` is defined when libraries
should be compiled and linked into dynamic libraries instead of static ones.
This is used in conjunction with the `BUILD_MODULE_x` macro to create macros 
that evaluate to `__declspec(dllexport)`, `__declspec(dllimport)`, or nothing 
based on whether a dynamic library is being compiled, a dynamic library is 
being linked with, or a static library is being compiled or linked with.

The `BUILD_VERSION` macro is not defined when precompiled headers are created.  This
is because its value changes based on the time of the build and this causes the macro
to be defined to different values for the precompiled header when the precompiled
header is compiled at a different time from the source files that include it.

## Preprocessor And Linker Debugging

It is possible to generate preprocessed files from source files rather
than passing them to the the compiler.  This is done by setting the settings
field `preprocess` to true.

It is also possible to set the linker to generate verbose output about the 
libraries it is searching for symbols and why it is searching them.  This is 
done by setting the settings field `verbose_linking` to true.

## clang

## gcc

## msvc

## mingw

## Executable

The *Executable* target prototype defines an executable to be linked.  Its 
identifier is the name of the executable to link less any platform and variant
dependent suffix that the build system will add, e.g. "_msvc_debug.exe".
It may contain a `libraries` key to specify the libraries that the 
executable depends on.  It may contain a `settings` key to specify any 
target specific settings that override the global settings.  Its contents 
are the `Source` and `Parser` targets that specify the source files to 
generate, compile, and link to create the executable.
    
## StaticLibrary

The *StaticLibrary* target prototype defines a static library to be archived.
Its identifier is the name of the library to archive less any platform and 
variant dependent suffix that the build system will add, e.g. 
"_msvc_debug.lib".  It may contain a `settings` key to specify any target 
specific settings that override the global settings.  Its contents are the 
`Source` and `Parser` targets that specify the source files to generate, 
compile, and archive to create the library.  It may contain a `libraries` key
that is ignored to allow for the case where a library specified by the Library
target prototype alias is building a static library.

## DynamicLibrary

The *DynamicLibrary* target prototype defines a dynamic library to be linked.
Its identifier is the name of the library to link less any platform and 
variant dependent suffix that the build system will add, e.g. 
"_msvc_debug.dll".  It may contain a `libraries` key to specify the libraries
that the library depends on.  It may contain a `settings` key to specify any
target specific settings that override the global settings.  Its contents are
the `Source` and `Parser` targets that specify the source files to generate, 
compile, and link to create the dynamic library.

The *Library* target prototype is an alias for either the `StaticLibrary` or
`DynamicLibrary` target prototype depending on the value of the `library_type`
setting.

## Cc, Cxx, ObjC, and ObjCxx

The *Source* target prototype defines a group of C or C++ source files.  Its 
identifier is ignored.  It may contain a `pch` attribute to specify the 
pre-compiled header to use.  It may contain a `defines` attribute to specify
extra preprocessor macros to be defined when compiling.  It may contain a 
`settings` key to specify any target specific settings that override the 
global settings or settings inherited from its containing target.  It must 
appear within an `Executable`, `StaticLibrary`, `DynamicLibrary`, or `Library`
target.
    
## Parser

The *Parser* target prototype defines a group of grammar files to be processed
into header files with the Sweet Parser tool.  Its identifier is ignored.  It 
must appear within an `Executable`, `StaticLibrary`, `DynamicLibrary`, or 
`Library` target.
