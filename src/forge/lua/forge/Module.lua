
local Module = {};

function Module.configure( toolset, module_settings )
    return {};
end

function Module.validate( toolset, module_settings )
    return true;
end

function Module.initialize( toolset )
    return true;
end

function Module.install( module, toolset )
    local module_settings;
    local configure = module.configure;
    if configure then
        local local_settings = forge.local_settings;
        module_settings = local_settings[module.id];
        if not module_settings then
            local settings = toolset.settings;
            module_settings = configure( toolset, settings[module.id] or {} );
            settings[module.id] = module_settings;
            local_settings[module.id] = module_settings;
            local_settings.updated = true;
        end
    end

    local validate = module.validate;
    if validate and not validate( toolset, module_settings ) then
        return;
    end

    local initialize = module.initialize;
    if initialize and not initialize( toolset ) then
        return;
    end

    local identifier = toolset.settings.identifier;
    if identifier then
        add_toolset( toolset:interpolate(identifier), toolset );
    end
end

local module_metatable = {
    __call = function( module, settings )
        local toolset = require( 'forge' ):clone( settings );
        module:install( toolset );
        return toolset;
    end;
    __index = Module;
};

setmetatable( Module, {
    __call = function( module, id )
        local module = {
            id = id;
        };
        setmetatable( module, module_metatable );
        return module;
    end
} );

return Module;
