
local cxx_header = [[
#ifndef _%s_
#define _%s_
    
class %s {
public:
    %s();
    ~%s();
};

#endif
]];

local cxx_source = [[
#include "%s.hpp"
%s::%s() {}
%s::~%s() {}
]];

function generate_source_files( base, libraries, classes, internal_includes, external_includes )
    local function includes( libraries, classes, internal_includes, external_includes )
        local includes = "";        
        for i = 1, external_includes do
            includes = includes..string.format("#include \"../lib_%d/class_%d.hpp\"\n", math.random(1, libraries), math.random(1, classes));
        end        
        for i = 1, internal_includes do
            includes = includes..string.format("#include \"class_%d.hpp\"\n", math.random(1, classes));
        end        
        return includes;
    end

    math.randomseed( 12345 );
    for library = 1, libraries do
        local directory = root( base.."/lib_"..tostring(library) );
        mkdir( directory );
    
        for class = 1, classes do
            local name = "class_"..tostring(class);
            local header = io.open( directory.."/"..name..".hpp", "wb" );
            assert( header, "Opening '"..directory.."/"..name..".hpp' failed" );
            header:write( string.format(cxx_header, name, name, name, name, name) );
            header:close();
            header = nil;
            
            local source = io.open( directory.."/"..name..".cpp", "wb" );
            assert( source, "Opening '"..directory.."/"..name..".cpp' failed" );
            source:write( includes(libraries, classes, internal_includes, external_includes) );
            source:write( string.format(cxx_source, name, name, name, name, name) );
            source:close();
            source = nil;
        end
    end    
end
