
HeaderFilePrototype = TargetPrototype {
	"HeaderFile", BIND_SOURCE_FILE
};

function HeaderFile( header_file )
	assert( type(header_file) == "string" );
	return target( header_file, HeaderFilePrototype );
end

local scanner = Scanner {
    [ [[^#include "([^"\n\r]*)"]] ] = function( target, match, headers )
        table.insert( headers, match );
    end;

    [ [[^#include <([^>\n\r]*)>]] ] = function( target, match, headers )
        table.insert( headers, match );
    end;
};

function default()
    local headers = {};
    scan( HeaderFile("scan_example.hpp"), scanner, headers );
    wait();
    for _, header in ipairs(headers) do
        print( header );
    end
end
