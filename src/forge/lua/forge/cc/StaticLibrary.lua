
local StaticLibrary = TargetPrototype( 'StaticLibrary' );

function StaticLibrary.create( toolset, identifier )
    local identifier, filename = toolset:static_library_filename( identifier );
    local target = toolset:Target( identifier, StaticLibrary );
    target:set_filename( filename or target:path() );
    target:set_cleanable( true );
    target:add_ordering_dependency( toolset:Directory(branch(target)) );
    return target;
end

-- Add dependencies including transitive static library dependencies.
--
-- Assumes that strings or static libraries added as dependencies are intended
-- to be transitive dependencies linked into the final executable or dynamic
-- library.  Transitive dependencies are added as ordering dependencies so
-- that they are built before they are linked but without causing the static
-- library they're a direct dependency of to be rebuilt.
function StaticLibrary.depend( toolset, target, dependencies )
    assert( type(dependencies) == 'table', 'StaticLibrary.depend() parameter not a table as expected' );
    forge:merge( target, dependencies );
    for _, value in walk_tables(dependencies) do
        if type(value) ~= 'table' or value:prototype() == toolset.StaticLibrary then
            local library = toolset:SourceFile( value );
            target:add_ordering_dependency( library );
        else
            local source_file = toolset:SourceFile( value );
            target:add_dependency( source_file );
        end
    end
end

function StaticLibrary.build( toolset, target )
    toolset:archive( target );
end

return StaticLibrary;
