
local StaticLibrary = TargetPrototype( 'StaticLibrary' );

function StaticLibrary.create( toolset, identifier )
    local filename, identifier = toolset:static_library_filename( identifier );
    local target = toolset:Target( identifier, StaticLibrary );
    target:set_filename( filename or target:path() );
    target:set_cleanable( true );
    target:add_ordering_dependency( toolset:Directory(branch(target)) );
    return target;
end

function StaticLibrary.build( toolset, target )
    toolset:archive( target );
end

return StaticLibrary;
