# Agent notes for Forge

Forge is a Lua-scriptable build tool. The C++ core builds a dependency graph of targets/rules/toolsets, binds targets to files, caches implicit dependencies and settings hashes, and rebuilds outdated targets based on timestamps/settings. Lua bindings and Lua rules provide the user-facing build DSL.

## Where to look

- `forge.lua` is the root build script for this repository.
- `src/forge/*.cpp` and `src/forge/*.hpp` are the core engine: `Forge`, `Graph`, `Target`, `Rule`, `Toolset`, scheduling, execution, graph I/O, and path functions.
- `src/forge/forge` is the CLI executable.
- `src/forge/forge_lua` exposes the C++ engine to Lua.
- `src/forge/forge_hooks` contains platform-specific filesystem access hooks used for automatic dependency detection.
- `src/forge/lua/forge` contains the standard Lua API/rules; `src/forge/lua/forge/cc` contains C/C++ toolchain rules.
- `src/forge/forge_test` contains Forge’s UnitTest++ test suite and Lua test fixtures.
- `src/assert`, `src/cmdline`, `src/error`, `src/luaxx`, and `src/process` are small local support libraries.
- `src/boost`, `src/lua`, and `src/unittest-cpp` are vendored/submodule code; avoid changing them unless explicitly needed.

## Docs map

- `docs/getting-started/running-forge.md` explains CLI commands and variables.
- `docs/getting-started/configuring-forge.md` explains root `forge.lua` build scripts and toolsets.
- `docs/getting-started/writing-buildfiles.md` explains `*.forge` buildfiles.
- `docs/more-details/rules.md`, `dependencies.md`, and `outdated-calculation.md` explain the core build model.
- `docs/reference/` documents Lua API functions, `Target`, and `Toolset`.
- `docs/cc-module/cc.md` documents the C/C++ Lua rules.

## Build and test

- Fresh checkout: run `git submodule update --init`, then `bash ./bootstrap-linux.bash`, `bash ./bootstrap-macos.bash`, or `bootstrap-windows.bat`.
- Normal build from repo root after bootstrapping: `bootstrap/bin/forge` or `bootstrap/bin/forge.exe`.
- Common variables/commands: `variant=debug|release|shipping`, `goal=path/to/target`, `clean`, `reconfigure`, `dependencies`, `namespace`.
- CI-style validation builds with `bootstrap/bin/forge`, installs release with `debug/bin/forge architecture=x86-64 variant=release prefix=forge install`, then runs `release/bin/forge_test`.

## Conventions and guardrails

- C++ is generally C++17. Warnings are treated as errors.
- Follow surrounding C++ style and existing `sweet::forge` namespaces.
- Lua/buildfiles use 4-space indentation, semicolon-separated table entries, and portable `/` paths. Convert to native paths only when passing paths to external tools.
- Do not edit generated or ignored outputs such as `bootstrap/`, `debug/`, `release/`, `shipping/`, `.forge`, `*.o`, or `*.obj`.
- When changing behavior, check whether corresponding updates are needed in the C++ core, Lua bindings/rules, docs/reference, and `src/forge/forge_test`.
