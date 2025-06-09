require 'forge';

local cc = {};

-- Collect transitive static library dependencies.
--
-- Walks dependencies from executables and dynamic libraries to find
-- transitive dependencies specified as passive dependencies of static
-- libraries.  Those transitive libraries are made dependencies of the
-- executable or dynamic library at the root of the walk.
--
-- Duplicate libraries are removed and the libraries are ordered such that
-- dependent libraries are listed earlier on the linker command line.
function cc.collect_transitive_dependencies(toolset, target)
    local function yield_recurse_on_library(target)
        local rule = target:rule();
        local yield = rule == toolset.StaticLibrary;
        local recurse = yield or rule == nil;
        return yield, recurse;
    end

    local libraries = {};
    local index_by_library = {};
    for _, dependency in walk_all_dependencies(target, yield_recurse_on_library) do
        table.insert(libraries, dependency);
        index_by_library[dependency] = #libraries;
    end

    for index, library in ipairs(libraries) do
        if index == index_by_library[library] then
            target:add_dependency(library);
        end
    end

    prune();
end

-- Implement depend() for transitive dependencies on static libraries.
--
-- Dependencies with StaticLibrary rules or no rules are added as passive
-- dependencies to be propagated to binaries in the prepare pass before the
-- build runs.  Other dependencies (e.g. object files) are added as normal
-- dependencies.
function cc.static_library_depend(toolset, target, dependencies)
    assert(type(dependencies) == 'table', 'Target.depend() parameter not a table as expected');
    merge(target, dependencies);
    for _, value in walk_tables(dependencies) do
        local dependency = toolset:SourceFile(value);
        local rule = dependency:rule();
        if rule == nil or rule == toolset.StaticLibrary then
            target:add_passive_dependency(dependency);
        else
            target:add_dependency(dependency);
        end
    end
end

_G.cc = cc;

local operating_system = _G.operating_system();
if operating_system == 'windows' then
    return require 'forge.cc.msvc';
elseif operating_system == 'linux' then
    return require 'forge.cc.gcc';
elseif operating_system == 'macos' then
    return require 'forge.cc.clang';
end
