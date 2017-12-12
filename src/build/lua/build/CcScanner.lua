
local function local_include( target, match )
    local header = file( "%s%s" % {target:directory(), match} );
    target:add_dependency( header );
end

local function global_include( target, match )
    local filename = root( "%s" % match );
    if exists(filename) then
        local header = file( filename );
        target:add_dependency( header );
    end
end

CcScanner = Scanner {
    [ [[^#include "([^"\n\r]*)"]] ] = local_include;
    [ [[^#include <([^>\n\r]*)>]] ] = global_include;
}
