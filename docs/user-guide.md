
# User Guide

## Running

A build is invoked by running `build` passing the parameters and commands that control which build steps are carried out on the command line.

Once invoked, a build proceeds through four general steps:

1. A search is made up from the current directory to find the file `build.lua`.  This file marks the root directory of the build and, when executed, initializes the modules and settings used in the build.

2. Global variables are set for each assignment on the command line.  These variables allow commonly varied parameters such as variant, platform, and version to be set.  The assignments are made in order and later assignments will override earlier ones in the case of duplicate variables.

3. The file `build.lua` is executed to initialize the build system.  Typically this involves loading and initializing the modules used in the build, setting default values for global variables that can be set on the command line, and initializing default values for the settings used in the build.

4. Each command passed on the command line is executed.  Commands are executed in the order in which they appear.  For example a complete rebuild of a project can be invoked by passing the **clean** command followed by the **default** command (the default command being to build the project).

Any expressions on the command line of the form **variable**=_value_ are assignments.  These assignments specify values that are assigned to global variables before `build.lua` is executed and provide a way to pass arguments to the build system.  For example the variant to build can be specified by setting the value of the _variant_ variable:

```
> build variant=release
```

Any other identifiers on the command line are commands.  Commands specify global functions that are called after `build.lua` has been executed and provide a way to specify which actions the build should carry out.  For example a clean action can be carried out by passing the **clean** command:

```
> build clean
```

Assignments and commands are made and executed in the order that they appear on the command line.  This means that later assignments override earlier ones in the case of duplicate variables and that commands are executed in order.  All assignments are made *before* any commands are executed so interleaving assignments and commands is not generally useful.

A build then proceeds through the following four steps:

- Search up from the current working directory to find the first directory containing a file named `build.lua`.

- Assign values to global variables in Lua for all assignments (`variable=value`) passed on the command line to parameterize the build (e.g. `variant=release`, `version=2.0.x`, etc).

- Execute the previously found `build.lua` to configure the build and load the initial dependency graph.  The dependency graph is typically loaded by loading several modular Lua scripts (referred to as buildfiles).

- Call global functions for each command (`command`) passed on the command line to carry out the desired build actions (e.g. `clean`, `default`, etc).

## Initialization

The following steps are generally carried out by `build.lua` to initialize the build system:

1. Provide default values for variables optionally set on the command line.

2. Load the Lua modules used to build the project.

3. Initialize the build system with project specific settings.

5. Specify the buildfiles that are loaded to define the build.

6. Specify the default targets built when a build is invoked from the root directory.

For example the following listing is of the `build.lua` used to initialize the build of the Sweet Build project:

``` lua
-- Provide default values for variables optionally set on the command line.
platform = platform or operating_system();
variant = variant or "debug";
version = version or os.date( "%Y.%m.%d %H:%M:%S" );
goal = goal or "";
jobs = jobs or 4;

-- Load the Lua modules used to build the project.
package.path = root("build/lua/?.lua")..";"..root("build/lua/?/init.lua");
require "build";
require "build.cc";
require "build.parser";
require "build.visual_studio";
require "build.xcode";
require "build.macosx";
require "build.windows";

-- Initialize the build system with project specific settings.
local settings = build.initialize {
    bin = root( ("../%s_%s/bin"):format(platform, variant) );
    lib = root( ("../%s_%s/lib"):format(platform, variant) );
    obj = root( ("../%s_%s/obj"):format(platform, variant) );
    include_directories = {
        root(),
        root( "boost" )
    };
    library_directories = {
        root( ("../%s_%s/lib"):format(platform, variant) ),
    };
    sln = root( "../sweet_build_tool.sln" );
    xcodeproj = root( "../sweet_build_tool.xcodeproj" );
};

-- Buildfiles loaded to define the build.
build.default_buildfiles {
    "boost/boost.build",
    "lua/lua.build",
    "sweet/assert/assert.build",
    "sweet/atomic/atomic.build",
    "sweet/build_tool/build_tool.build",
    "sweet/cmdline/cmdline.build",
    "sweet/error/error.build",
    "sweet/lua/lua.build",
    "sweet/path/path.build",
    "sweet/persist/persist.build",
    "sweet/pointer/pointer.build",
    "sweet/process/process.build",
    "sweet/rtti/rtti.build",
    "sweet/thread/thread.build",
    "sweet/traits/traits.build",
    "unit/unit.build"
};

-- Targets built when building from the root directory.
build.default_targets {
    "sweet/build_tool/build"
};
```

Default values are provided for variables that are optionally set on the command line using a Lua idiom of assigning a value to itself or a default value.  In Lua unassigned variables evaluate to `nil` which is false when used in a boolean expression such as `or` giving the effect of keeping variables that already have values the same and providing default values for those variables that were not already assigned.

The `or` idiom used takes advantage of short circuiting to preserve any value that may already have been set from the command line otherwise it uses the default value on the right.

The first block of code sets Lua's package path and loads the modules used to build the project.  The package path is set to reference build scripts stored locally to the project using the `root()` function to generate paths relative to the root directory of the project (the directory that contains `build.lua`).  The modules are then loaded using the `require()` function which searches for and loads the specified module from the package path.

The build system is initialized via a call to `build.initialize()` passing a table of project specific settings to override the build system's default settings.  This call returns a settings table containing a combination of all of the build system's settings -- those provided for the project, the local settings automatically generated for the local system, and the default settings provided by the build system itself.

Some useful aspects of this initialization are:

- The `bin`, `lib`, and `obj` directories are outside of the project root and source tree.  This reduces the number of exclusions that might need to be added to lists of patterns and files to ignore when dealing with version control.

- The `bin`, `lib`, and `obj` directories are specified per platform and variant so that variants and cross compiles are remain completely independent.  There's no need to clean a previous build just to build a different variant.

- In both cases the generated projects are external build system projects that invoke Sweet Build to carry out the build.  The generated projects are specifically not native Visual Studio or XCode projects.  This means that you get a canonical build representation/description not matter which development environment you prefer.

The default buildfiles are the buildfiles that are loaded to define the dependency graph of targets for the project.  The buildfiles themselves are modular and can be shared between multiple projects.  In general the buildfiles will inherit the settings that they use from those defined in the `build.lua` script of the project that they're used in.

- I should also make some mention of Lua's short form call syntax that allows parentheses to be omitted for function calls that take either a single literal table or a single literal string as an argument.

## Buildfiles 

Buildfiles specify the targets, dependencies, and actions needed to build a project.  They have the `.build` extension and appear throughout the directory hierarchy of a project usually with a one-to-one mapping between buildfile and component, module, package, library, or executable being built.

Buildfiles are plain Lua scripts from which the full syntax of Lua is available.  Lua's syntactic sugar and some conventions are used to provide a convenient declarative interface to specify the files and actions involved in building a project.

- The use of **prototype** [*identifier* | **(** *identifier*, *...* **)**] to specify a target to be built.

- The use of *target* **{** *dependencies* **}** to add dependencies to an already specified target. 

These two idioms and the fact that dependent targets may be specified directly as part of *dependencies* allow convenient chaining of target creation and dependencies into **prototype** (*identifier* | **(** *identifier* **,** *...* **)**) **{** *dependencies* **}**.

#### Hello World!

The following buildfile builds the classic "Hello World!" program.  It creates targets to compile its single source file and to link the resulting object file into the final executable:

``` lua
build.Executable "hello_world" {
    build.Cxx {
        "hello_world.cpp"
    };
};
```

Breaking this buildfile down into its component parts gives:

- A call to `build.Cxx()` taking a table as array containing a list of C++ source files to compile into object files.  In this case compiling `hello_world.cpp` into `hello_world.o` (or `hello_world.obj` depending on compiler).  This call returns a target that depends on all of the object files which in turn depend on the specified source files.  If any of the source files are out of date (newer) with respect to their object files then the compiler will be run to generate a new, up to date, object file.

- A call to `build.Executable()` taking a single string, the identifier of the target to create as its sole argument.  This call returns a target that represents the executable to be built to which dependencies can be added.

- A call to `Executable.call()` taking a table as array containing a list of object files to link into the executable.  In this case linking `hello_world.o` (or `hello_world.obj`) into `hello_world` (or `hello_world.exe` on Windows).  This is not specifically true in that the executable actually depends on one or more `Cxx` targets that in turn depend on the object file targets but that is a detail hidden by the build system at this point.

#### More Hello World!

To make an example using an even more contrived example the following buildfile builds the classic "Hello World!" example using a separate library.  It creates targets to compile the source files in the library and the executable, archive the library, and link the executable.  The executable also specifies a dependency on the library which results in the library being linked in with the executable's object files:

~~~
build.Executable "hello_world" {
    libraries = {
        "libhello_world"
    };
    build.Cxx {
        "executable.cpp"
    };
};

build.Library "libhello_world" {
    build.Cxx {
        "library.cpp"
    };
};
~~~

This example buildfile specifies two targets to build.  An executable and a library.  The executable depends on the library through the identifier of the library being listed in the `libraries` table of the executable.  The values listed in the `libraries` table are paths to libraries relative to the root of the project.  Specifying this dependency relationship causes the library to be built before the executable, links the library when linking the executable, and makes sure that the executable is linked if the library is changed.

The `buildfiles()` function loads the dependency graph from several buildfiles spread through the project directory hierarchy.  The `buildfile()` function  takes a relative path to the buildfile to load as its only parameter.  The function sets the working directory to the directory that contains the buildfile and then executes the buildfile as a Lua script.  Changing the working directory has the effect of making any relative paths expressed in the buildfile relative to the directory that the buildfile is in.

The use of separate buildfiles in combination with the inheritance of settings described in the following section allows buildfiles to be reused in a modular way.  For example the `lua/lua.build` buildfile above is used again in the Sweet Build project but with different settings inherited from the `build.lua` script in the Sweet Build project without any changes needing to be made to the `lua/lua.build` buildfile.

For more real world examples have a look at the Sweet Build source or any of the other libraries at http://www.sweetsoftware.co.nz/.

## Settings

The build system generates settings from the following sources:

- Default settings from `default_settings.lua` in the build script's directory.

- Project settings from `build.lua` in the project's root directory.

- User settings from `user_settings.lua` in the user's home directory.

- Local settings from `local_settings.lua` in the project's root directory.

- Platform settings from `settings.settings_by_platform`.

- Variant settings from `settings.settings_by_variant`.

The settings tables from these sources are merged into and inherited from one another to provide three levels of settings made accessible through `build.default_settings`, `build.local_settings`, and `build.settings` for build scripts and, to a lesser degree, buildfiles.

The default settings are generated by merging the project settings from `build.lua` into the default settings from `default_settings.lua`.  Merging settings combines tables by appending array values, overwriting simple named values, and recursively merging table named values.

The local settings are then generated by inheriting an empty table from the default settings and then merging in the user settings from `user_settings.lua` and the local settings from `local_settings.lua`.  Either or both of the user and local settings files might be missing.

The final settings are then generated by inheriting an empty table from the local settings and merging in the platform and variant specific settings that are available from `settings.settings_by_platform` and `settings.settings_by_variant` through inheritance of the local and default settings.

The `local_settings.lua` file is automatically generated by the build system and stores per-machine settings that don't change very often.  For example the location of compilers and tools used during the build.  The local settings are available for the user to change when they need to or can be regenerated using the reconfigure command.

The `user_settings.lua` file provides a way to specify per-user configuration that should apply to all projects.  For example the install directory for common external libraries such as Boost or Qt might be specified as user settings.  User settings are optional. 

Settings should be accessed through the settings stack via the functions `build.push_settings()`, `build.pop_settings()`, and `build.current_settings()`.  Pushing settings onto the stack automatically inherits from the settings that were previously at the top of the stack.

Reaching the end of a buildfile restores the settings stack to the level that it was at at the start of the buildfile.  There is no need to ensure that calls to `build.push_settings()` and `build.pop_settings()` are balanced -- although it is good practice to do so!

The following buildfile overrides the settings for subsystem, stack size, architectures, and defines but inherits the values for all other settings from the project-level settings:

``` lua
local settings = build.push_settings {
    subsystem = "CONSOLE"; 
    stack_size = 32768; 
    architectures = { "x86_64" };
    -- Disable automatic linking to Boost libraries.
    defines = { "BOOST_ALL_NO_LIB" };
};

for _, architecture in ipairs(settings.architectures) do
    build.Executable ("build", architecture) {
        libraries = {
            "boost/libs/filesystem/src/boost_filesystem",
            "boost/libs/system/src/boost_system",
            "sweet/build_tool/build_tool",
            "sweet/cmdline/cmdline",
            "sweet/lua/lua",
            "lua/liblua",
            "sweet/process/process",
            "sweet/rtti/rtti",
            "sweet/thread/thread",
            "sweet/persist/persist",
            "sweet/path/path",
            "sweet/pointer/pointer",
            "sweet/error/error",
            "sweet/assert/assert"
        };
        
        build.Cxx (architecture) {
            "Application.cpp", 
            "main.cpp"
        };    
    }
end

build.pop_settings();
```

## Targets

- Dependency graph of nodes (targets to be built) and edges (dependencies between targets).

- Targets are created by passing an identifier to one of the target creation functions `build.SourceFile()`, `build.File()`, or `build.Target()` to create targets representing source files, intermediate files, or targets that determine their filename after creation or don't bind to files at all.

- Targets exist in a hierarchical namespace with the same semantics as operating system paths.  Targets have an identifier, a parent, and zero or more children.  Targets are referred to via `/` delimited paths by using `.` and `..` to refer to the current target and parent target respectively.

- The identifier used to create a target is considered a relative path to the target to create.  Relative paths are considered relative to the build system's notion of the current working directory.

- The hierarchical namespace is *not* the same as the hierarchical of directories and files on disk.  Targets are often bound to files on disk in quite separate locations from their location the target namespace.

- The target namespace is persistent in that creating a target with the same identifier more than once simply returns the originally created target.  This allows for late binding in cases where depending targets must refer to their dependencies before those dependencies are created.

- Anonymous targets are created by passing an empty string as their identifier.  This creates a target using a generated unique identifier.

- Once created targets can be added as dependencies of other targets to implicitly provide and order in which the targets must be built (e.g. source files to object files to executable).

- Perhaps some note about target paths and operating system paths always using `/` as a delimiter regardless of operating system.  Paths can be converted to native paths at the last minute using the `native()` function.

- The use of **prototype** [*identifier* | **(** *identifier*, *...* **)**] to specify a target to be built.

- The use of *target* **{** *dependencies* **}** to add dependencies to an already specified target.

## Target Prototypes

- Target prototypes define the actions undertaken to build a target.  A target prototype defines behavior of its targets in much the same way as a class defines the behavior of its objects in a language like C++ or Java.

- Target prototypes are created by calling `build.TargetPrototype()` and passing the name of the target prototype.  Functions are then defined on the returned target prototype to specify the actions to take when targets using that prototype are visited in a traversal.

- Two functions, `create()` and `call()`, have special meaning.  These two functions are used outside of a traversal to dispatch calls made to the target prototype to create a target and to the target to add dependencies.

- The `create()` function is called whenever a target for that target prototype is created by calling the target prototype table returned from `build.TargetPrototype()`.  The first parameter to this function is the settings at the top of the settings stack and all subsequent arguments are forwarded from the call. 

- The `call()` function is called whenever a call is made on a target for that target prototype.  This is typically used to capture the use of calls of the form *target* **{** *dependencies* **}** used to add dependencies or further define a target.  The first parameter to this function is the target and all subsequent arguments are forwarded from the call.

- The other functions defined on a target prototype are used when making traversals over the dependency graph.  The `build.visit()` function takes the name of a function to call when visiting a target in the graph.  Any targets that provide that function through their target prototypes are visited by having that function called. 

- Not all targets will have a target prototype that defines their behavior.  These targets are typically source files that aren't updated by the build system but do need to be tracked in order to determine when intermediate files that depend on them are outdated.

- The relationship between a target and its target prototype is built using using Lua's prototype based inheritance.  In general prototype based inheritance is more dynamic than class based inheritance although in this case only the ability to change a target from having no target prototype to having one is taken advantage of.

- Using target prototypes is the only way to associate functions with targets and have them saved and loaded to and from the dependency graph correctly.  The  serialization code is unable to save or load functions or closures.  The relationship between a target and its target prototype relationship is serialized correctly and is maintained across a save and load of the dependency graph.

- The relationship between a target prototype and a target is implemented using Lua's prototype based inheritance so the analogy with classes and objects in C++ and Java isn't strictly true.  The prototype based inheritance is thus more like prototypes in Javascript or Self languages.

- For example the `Copy` prototype provides the behavior for a target that copies a file whenever the copied file doesn't exist or is older than its source file.  If the source file is actually a directory then the same operations are carried out except that the entire directory is copied and removed instead.

~~~ lua
local Copy = build.TargetPrototype( "Copy" );

function Copy.create( settings, destination, source )
    local copy = build.File( destination, Copy, settings );
    copy:add_dependency( build.SourceFile(source, settings) );
    copy:add_dependency( build.Directory(copy:branch(), settings) );
    return copy;
end

function Copy.build( copy )
    if copy:outdated() then
        local source = copy:dependency():filename();
        if is_file(source) then 
            rm( copy:filename() );
            cp( copy.source, copy:filename() );
        else
            rmdir( copy:filename() );
            cpdir( copy.source, copy:filename() );
        end
    end
end

function Copy.clean( copy )
    local source = copy:dependency():filename();
    if is_file(source) then 
        rm( copy:filename() );
    else
        rmdir( copy:filename() );
    end
end
~~~

## Traversal

Targets and dependencies provide the dependency graph with structure, target prototypes associate actions and behavior with targets, and traversals provide the correct order for those actions to be carried out.

- Postorder traversal visits each target's dependencies before it visits that target.  This ordering ensures that dependencies are built before the targets that depend on them.

- Targets are guaranteed to only be visited once per traversal even if they are depended upon by more than one depending target.

- Cyclic references are quietly ignored.

---

## Binding

- The bind pass is a special case postorder traversal in which targets are bound to files and their dependencies.  Binding determines whether or not targets are outdated with respect to their dependencies.

- Targets can be bound to zero or more files.  It is usual for targets to only be bound to one file.

- Binding provides targets with timestamps, last write times, and determines whether the target has changed since the build system last checked by considering the file(s) that they are bound to and the targets that they depend on.

- Targets that aren't bound to files don't track last write time or changed status.  These targets exist solely to group other dependencies together in a convenient way.

- Targets that are bound to files that don't exist yet have their timestamp set to the latest possible time so that the target will always be newer than any targets that depend on it.

- Targets that are bound to existing directories and have no dependencies have their timestamp set to the earliest possible time so that targets that depend on them won't be considered outdated.  This behavior is to allow targets to depend on the directory that they will be generated into without being outdated based on the last write time of the directory.

- Targets that are bound existing files or directories with dependencies have their timestamp and last write time set to the last write time of the file.  Any targets that depend on this target will be outdated if they are older than this target.  Additionally if the last write time of the file or directory has changed from the last time the build system checked the target is marked as having changed.

- Targets that are bound to multiple files have their timestamp set to the latest last write time of any of the files and their last write time set to the earliest last write time of any of the files.  This combination covers the range of time represented by this target and interacts correctly with the outdated calculation and timestamp propagation that follows.

- Targets are bound to their dependencies after they have been bound to files.

- Binding a target to its dependencies sets the timestamp of the target to be the latest timestamp of any of its dependencies and its last write time.

- Targets that are bound to files are marked as being outdated if any of their dependencies have timestamps newer than the last write time of the target.  Targets that aren't bound to files are considered to be outdated if any of their dependencies are outdated.

## Execution

Be aware that the `execute()` function suspends execution of the Lua coroutine that it is called on and that this can lead to race conditions in the Lua scripts in much the same way that it does in other environments.

Take particular care when calling `execute()` to populate a shared structure during a preorder pass. The preorder traversal will generate multiple coroutines and those coroutines will run be scheduled to run according to the sequence and timings of the `execute()` calls.

If `execute()` is called as part of a postorder traversal to execute commands to build external files and the dependencies are expressed correctly then there should be no problems with race conditions.  The ordering based on dependencies ensures that race conditions don't occur.

## Modules

- Modules add support for new platforms, compiler toolchains, and tools to the build system.

- Modules are ostensibly (?) using Lua's module system.  They are Lua scripts loaded using the `require()` function that produce a table containing functions that are then accessible to the rest of the build system.

- Two of the functions, `configure()` and `initialize()`, have special meaning.  The two functions are used by the build system to configure and initialize modules that have been registered.

- The `configure()` function automatically generates settings for parameters that don't change very often like the paths to tools and SDKs.  These settings and are stored in `local_settings.lua` in the project root directory.

- The `initialize()` function initializes the module for use on each run.  This typically sets the values of Lua variables that are needed for the module to operate, sets the value of environment variables that the module's tools need to operate.

- Other module functions may be defined as required for the module's intended use.  For example the modules that add support for various C++ compilers define functions that are then called by the generic C++ target prototypes to compile, archive, and link C++ programs.

- Additional Lua scripts are often loaded from modules to add support for additional target prototypes for the files that the module adds support for building.

- Register modules by calling `build.register_module()` and passing the module table.

- The configuration is auto-generated by the loaded and initialized modules but is intended to be a place for those settings to be manually overridden by the user.  So it's kind of a means of discoverability for the settings used by modules so even if there's no autodetection of a value it's worth writing it to the local settings so that users can discover which values are able to be changed.

## Loading and Saving

The dependency graph can be saved and loaded to and from binary or XML files.  This is useful for caching implied dependencies to speed up incremental builds.  For example it is quite time consuming to scan C\++ source files and headers for extra dependencies implied by include directives but once it has been done that graph can be saved to a file and reloaded again to recreate the same dependencies.  Only the C++ source files and headers that have changed since the graph was last saved need to be scanned again.

The boolean, numeric, string, and table values stored in Lua tables are saved and reloaded as part of the cache.  This includes correctly persisting cyclic relationships between tables.

Function and closure values are *not* saved.  This is generally not a problem because functions and closures are defined in target prototypes and the target prototype relationship of each target is preserved across a save and a load.
        
## Errors

Errors are reported by calling the `error()` function and passing a string that describes the error that has occured.  This displays the error message on the console and causes the current processing to fail.

Errors that occur while visiting a target mark the target and any dependent targets as failing.  The dependent targets are not visited (although an error message is displayed for each dependent target that is not visited).  Processing of other targets that aren't part of the same dependency chain continues.
        
## Debugging

Debugging the build system is unfortunately only possible by adding prints at appropriate places.  There are two functions, `print_dependencies()` and `print_namespace()`, that can be used to display the dependency graph and namespace respectively - this can be useful for tracking down why something is being built when it shouldn't and vice versa.

## Syntax

The language used in both build scripts and buildfiles is Lua.  The language
is described in detail in the online manual at 
http://www.lua.org/manual/5.1/manual.html/.

For readers in a rush Lua is similar to other programming languages.  It is
procedural and contains expressions (`+`, `-`, `/`, etc) and statements 
(`if`, `for`, `while`, etc) with semantics that should be familiar to those 
with knowledge of another programming language.

Some differences between Lua and other programming languages that show up in
build scripts and buildfiles are:

- The "`..`" operator is used to concatenate strings.

- The "`%`" operator is used to interpolate strings with a printf-style format
string and arguments as described by that string.  This is defined in the 
build system (therefore it isn't available until _after_ the build scripts 
have been loaded).

- Strings can be delimited with double square brackets (`[[` and `]]`) as 
well as the usual single and double quotes.  Strings delimited with double
square brackets don't do normal escaping while single and double quoted 
strings do.

- The main data type in Lua is the table - an associative array constructed 
using curly braces and an optional list of fields.  Tables can be treated as
arrays, a maps, or objects interchangeably.  Be aware that array indices are 
numbered from 1 and not 0 like you might expect.  Don't worry!  It works well
in practice.

- Functions are first class values and so show up as fields within tables, 
local variables, parameters, and return values.

- Lexical scoping is used to determine the visibility of variables - local
variables are available from within functions defined within the same scope
even after that scope has been released from the stack.

- An identifier followed by a string literal or an identifier followed by a
table constructor is syntactic sugar for a function call taking a string or 
table as its only argument.

- An identifier followed by colon "`:`" followed by another identifier is 
syntactic sugar for a function call on a table.  The first identifier refers 
to the table and second identifier to a function stored in that table.  The 
table is passed as the implicit first parameter to the function.  The "`:`" 
syntax can also be used when the function is defined in which case it implies
a hidden first parameter named "`self`". 

- Lua is a prototype based language.  A table can set another table to act as
its prototype and have lookups for fields that aren't defined automatically 
redirected to be looked up in the prototype table.  The table inherits the 
fields that it doesn't define itself from its prototype.

- Settings inheritance in the build system is implemented using prototypes.
Per target settings are inherited from their parent target's settings or the
global settings.  The global settings are inherited the default settings 
specified by the build system and the project being built.

- The behaviour of targets in the build system is also defined using 
prototypes.  In this case each target has its own target prototype and the
target prototype defines the functions that specify which actions are taken
for that type of target in a traversal of the dependency graph.
