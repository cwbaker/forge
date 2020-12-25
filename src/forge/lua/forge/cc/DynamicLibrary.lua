
local DynamicLibrary = TargetPrototype( 'DynamicLibrary' );

function DynamicLibrary.create( toolset, identifier )
    local filename, identifier = toolset:dynamic_library_filename( identifier );
    local target = toolset:Target( identifier, DynamicLibrary );
    target:set_filename( filename or target:path() );
    target:set_cleanable( true );
    target:add_ordering_dependency( toolset:Directory(branch(target)) );
    return target;
end

function DynamicLibrary.build( toolset, target )
    toolset:link( target );
end

-- See Executable.find_transitive_libraries().
DynamicLibrary.find_transitive_libraries = require('forge.cc.Executable').find_transitive_libraries;

return DynamicLibrary;
