
local global_include;

local function local_include( target, match )
    local filename = ("%s%s"):format( target:directory(), match );
    if exists(filename) then
        local header = file( filename );
        target:add_dependency( header );
        return true;
    end
end

local function global_include( target, match )
    local settings = target.settings or build.settings;
    if settings then
        for _, include_directory in ipairs(settings.include_directories) do 
            local filename = ("%s/%s"):format( include_directory, match );
            if exists( filename ) then
                local header = file( filename );
                target:add_dependency( header );
                return true;
            end
        end
    end
end

local CcScanner = Scanner {
    [ [[^#include "([^"\n\r]*)"]] ] = function( target, match ) 
        return local_include( target, match ) or global_include( target, match );
    end;
    [ [[^#include <([^>\n\r]*)>]] ] = function( target, match ) 
        return global_include( target, match ) or local_include( target, match );
    end
};

_G.CcScanner = CcScanner;
