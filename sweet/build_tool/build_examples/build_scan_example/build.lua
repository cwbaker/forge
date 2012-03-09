
HeaderFile = TargetPrototype( "HeaderFile", BIND_SOURCE_FILE );

local scanner = Scanner {
    [ [[^#include "([^"\n\r]*)"]] ] = function( target, match, headers )
        table.insert( headers, match );
    end;

    [ [[^#include <([^>\n\r]*)>]] ] = function( target, match, headers )
        table.insert( headers, match );
    end;
};

local headers = {};
scan( HeaderFile("scan_example.hpp"), scanner, headers );
wait();
for _, header in ipairs(headers) do
    print( header );
end
