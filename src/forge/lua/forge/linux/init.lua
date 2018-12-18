
local gcc = require 'forge.cc.gcc';

local linux = {};

function linux.initialize( forge, settings, local_settings )
    -- if forge:operating_system() == 'linux' then
    --     local gcc = require 'forge.cc.gcc';
    --     for _, architecture in ipairs(settings.linux.architectures) do 
    --         local gcc_forge = forge:configure {
    --             obj = ("%s/cc_linux_%s"):format( settings.obj, architecture );
    --             platform = "linux";
    --             architecture = architecture;
    --         };
    --         gcc.register( gcc_forge );
    --         forge:add_default_build( ("cc_linux_%s"):format(architecture), gcc_forge );
    --     end
    -- end
end

forge:register_module( linux );
return linux;
