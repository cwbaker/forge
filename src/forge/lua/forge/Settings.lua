
local Settings = {};

function Settings.create( self, values )
    local values = values or forge.local_settings;
    apply( self, values );
    return self;
end

function Settings.clone( self, values )
    local settings = Settings();
    apply( settings, self );
    apply( settings, values );
    return settings;
end

function Settings.inherit( self, values )
    local settings = values or {};
    setmetatable( settings, {__index = self} );
    return settings;
end

function Settings.apply( self, values )
    apply( self, values );
    return self;
end

function Settings.defaults( self, values )
    for key, value in pairs(values) do 
        if type(key) == 'string' and self[key] == nil then
            self[key] = value;
        end
    end
    return self;
end

local settings_metatable = {
    __index = Settings;
};

setmetatable( Settings, {
    __call = function ( _, values )
        local settings = {};
        setmetatable( settings, settings_metatable );
        return settings:create( values );
    end
} );

return Settings;
