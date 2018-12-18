
local msvc = require 'forge.cc.msvc';

local windows = {};

function windows.initialize( forge, settings, local_settings )
    -- if forge:operating_system() == 'windows' then        
    --     local msvc = require 'forge.cc.msvc';
    --     for _, architecture in ipairs(settings.architectures) do 
    --         local msvc_forge = forge:configure {
    --             obj = ('%s/cc_windows_%s'):format( settings.obj, architecture );
    --             platform = 'windows';
    --             architecture = architecture;
    --         } 
    --         msvc.register( msvc_forge );
    --         forge:add_default_build( ('cc_windows_%s'):format(architecture), msvc_forge );
    --     end
    -- end
end

forge:register_module( windows );
return windows;
