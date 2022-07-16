---
layout: page
title: cc
parent: C/C++ Module
nav_order: 1
---

- TOC
{:toc}

The *cc* module supports building C, C++, Objective-C, and Objective-C++ with Clang, GCC, Microsoft Visual C++, and MinGW on Linux, macOS, and Windows.

## Settings

Configure the *cc* module using the following settings specified in the toolset that initializes the module or overridden in individual buildfiles throughout the project:

- `architecture` sets the architecture (*x86_64*, *armv7*, *arm64*, etc);
- `assertions` is true to enable assets
- `debug` is true to enable debugging
- `exceptions` is true to enable C++ exceptions
- `fast_floating_point` is true to enable fast floating point optimizations
- `generate_map_file` is true to generate a map file;
- `incremental_linking` is true to enable incremental linking
- `link_time_code_generation` is true to enable link time code generation
- `minimal_rebuild` is true to enable minimal rebuilds, false to disable
- `optimization` is 0 for no optimization, 3 for full optimization
- `pre_compiled_headers` is true to enable pre-compiled headers
- `preprocess` is true to preprocess source instead of compiling
- `profiling` is to compile with profiling hooks
- `run_time_checks` is true to enable run-time checks false to disable
- `runtime_library` sets the standard library used at runtime
- `run_time_type_info` is true to enable run-time type information
- `stack_size` sets the size of the initial stack
- `standard` sets the C/C++ standard
- `string_pooling` is true to enable string pooling
- `strip` is true to enable stripping
- `verbose_linking` is true to enable verbose messages when linking
- `warning_level` is 0 for no warnings, 3 for full warnings
- `warnings_as_errors` is true to treat warnings as errors

## Prototypes

### Executable

- Links one or more libraries and object files into a single executable.

- Expected dependencies: `StaticLibrary`, `Cc`, `Cxx`, `ObjC`, `ObjCxx`

- The executable's identifier should be without any platform specific extension like *.exe*.  The underlying rule decorates the filename as necessary.

- The executable target's identifier remains platform independent and portable so that it can be referenced by other buildfiles and/or build scripts without the need to add additional code to decorate the filename all over the place.

- The `libraries` attribute can be set to a list (table) of libraries to link with on the command line.  The directories containing these libraries must be added separately to the toolset's settings through the `library_directories` attribute or made available to the linker through some other means (e.g. by being in the default library search path for that toolchain).

~~~lua
forge:Executable '${bin}/forge' {
    '${lib}/forge_${architecture}';
    '${lib}/forge_lua_${architecture}';
    '${lib}/process_${architecture}';
    '${lib}/luaxx_${architecture}';
    '${lib}/cmdline_${architecture}';
    '${lib}/error_${architecture}';
    '${lib}/assert_${architecture}';
    '${lib}/liblua_${architecture}';
    '${lib}/boost_filesystem_${architecture}';
    '${lib}/boost_system_${architecture}';

    libraries = libraries;
    
    forge:Cxx '${obj}/%1' {
        defines = {    
            'BOOST_ALL_NO_LIB';
            ('BUILD_VERSION="\\"%s\\""'):format( version );
        };
        'Application.cpp', 
        'main.cpp'
    };    
};
~~~

### DynamicLibrary

- As per `Executable` above but links a dynamic library instead of an executable.

### StaticLibrary

- Archives one or more object files into a single static library.
- Expected dependencies: `Cc`, `Cxx`, `ObjC`, `ObjCxx`

### Cc, Cxx, ObjC, ObjCxx

- Compile C, C++, Objective-C, or Objective-C++ respectively

- Expected dependencies: Source files of the respective language

- The `defines` attribute can be set to a list of preprocessor macros to pass on the command line.  These can be of the form `IDENTIFIER` or `IDENTIFIER=...` but take care with quoting of strings as they must pass through Lua before being formatted onto the command line (e.g. backslash characters will be interpreted as escape sequences and should themselves be escaped).

## Compilers

### clang

- `framework_directories` lists the directories to search for frameworks
- `generate_dsym_bundle` is true to generate dSYM bundle for debug symbols
- `objc_arc` is true to enable Objective-C ARC
- `objc_modules` is true to enable Objective-C modules

### gcc

### msvc

- `subsystem` sets the Windows subsystem (*BOOT_APPLICATION*, *CONSOLE*, *EFI_APPLICATION*, *EFI_BOOT_SERVICE_DRIVER*, *EFI_ROM*, *EFI_RUNTIME_DRIVER*, *NATIVE*, *POSIX*, *WINDOWS[,major[.minor]*).  See [/SUBSYSTEM](https://docs.microsoft.com/en-us/cpp/build/reference/subsystem-specify-subsystem?view=vs-2019) for more details.

### mingw

## Preprocessor And Linker Debugging

It is possible to generate preprocessed files from source files rather
than passing them to the the compiler.  This is done by setting the settings
field `preprocess` to true.

It is also possible to set the linker to generate verbose output about the 
libraries it is searching for symbols and why it is searching them.  This is 
done by setting the settings field `verbose_linking` to true.
