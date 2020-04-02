
local Toolset = require 'forge.Toolset';
local ToolsetPrototype = {};

setmetatable( ToolsetPrototype, {
    __call = function( ToolsetPrototype, _forge, id )
        local toolset_prototype = {
            id = id;
        };
        toolset_prototype.__index = toolset_prototype;
        local toolset_prototype_metatable = {
            __index = Toolset;
            __call = function( _, values )
                return Toolset.new( toolset_prototype, values );
            end;
            __tostring = function( toolset_prototype )
                return toolset_prototype.id;
            end;
        };
        setmetatable( toolset_prototype, toolset_prototype_metatable );
        return toolset_prototype;
    end;
} );

return ToolsetPrototype;
