
local Toolset = require 'forge.Toolset';
local ToolsetPrototype = {};

setmetatable( ToolsetPrototype, {
    __call = function( ToolsetPrototype, _forge, id )
        local toolset_prototype = {
            id = id;
        };
        local toolset_metatable = {
            __index = toolset_prototype;
        };
        local toolset_prototype_metatable = {
            __call = function( module, settings )
                local toolset = forge.Toolset():apply( settings );
                toolset.prototype = toolset_prototype;
                setmetatable( toolset, toolset_metatable );
                toolset:install();
                return toolset;
            end;
            __index = Toolset;
        };
        setmetatable( toolset_prototype, toolset_prototype_metatable );
        return toolset_prototype;
    end;
} );

return ToolsetPrototype;
