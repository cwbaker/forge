
local Executable = TargetPrototype( 'Executable' );

function Executable.create( toolset, identifier )
    local identifier, filename = toolset:executable_filename( identifier );
    local target = toolset:Target( identifier, Executable );
    target:set_filename( filename or target:path() );
    target:set_cleanable( true );
    target:add_ordering_dependency( toolset:Directory(branch(target)) );
    return target;
end

function Executable.build( toolset, target )
    toolset:link( target );
end

-- Collect transitive dependencies on static and dynamic libraries.
--
-- Walks immediate dependencies adding static and dynamic libraries to a list
-- of libraries.  Recursively walks the ordering dependencies of any static
-- libraries to collect transitive static library dependencies.  Removes
-- duplicate libraries preserving the most recently added duplicates at the
-- end of the list.
--
-- Returns the list of static libraries to link with this executable.
function Executable.find_transitive_libraries( target )
    local toolset = target.toolset;
    local function is_static_library( target ) 
        return target:prototype() == toolset.StaticLibrary; 
    end

    local all_libraries = {};
    local index_by_library = {};
    for _, dependency in walk_dependencies(target) do
        local prototype = dependency:prototype();
        if prototype == toolset.StaticLibrary or prototype == toolset.DynamicLibrary then
            table.insert( all_libraries, dependency );
            index_by_library[dependency] = #all_libraries;
            for _, dependency in walk_ordering_dependencies(dependency, is_static_library, is_static_library) do
                table.insert( all_libraries, dependency );
                index_by_library[dependency] = #all_libraries;
            end
        end
    end

    local libraries = {};
    for index, library in ipairs(all_libraries) do
        if index == index_by_library[library] then
            table.insert( libraries, library );
        end
    end
    return libraries;
end

return Executable;
