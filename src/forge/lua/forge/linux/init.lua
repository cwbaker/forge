
local linux = {};

function linux.initialize( settings )
    if forge:operating_system() == 'linux' then
        local gcc = require 'forge.cc.gcc';

        for _, architecture in ipairs(settings.linux.architectures) do 
            local gcc_forge = forge:configure {
                obj = ("%s/cc_linux_%s"):format( settings.obj, architecture );
                platform = "linux";
                architecture = architecture;
                default_architecture = architecture;
                obj_directory = linux.obj_directory;
            };
            gcc.register( gcc_forge );
            forge:add_default_build( ("cc_linux_%s"):format(architecture), gcc_forge );
        end

        local settings = forge.settings;
        local default_architecture = settings.default_architecture;
        local default_forge = forge:default_build( ('cc_linux_%s'):format(default_architecture) );
        assert( default_forge, 'default Forge instance not found for Linux initialization' );
        local default_settings = default_forge.settings;
        settings.obj = default_settings.obj;
        settings.platform = default_settings.platform;
        settings.architecture = default_settings.architecture;
        settings.default_architecture = default_settings.default_architecture;
        settings.obj_directory = default_settings.obj_directory;
        gcc.register( forge );
    end
end

function linux.obj_directory( target )
    local relative_path = forge:relative( target:working_directory():path(), forge:root() );
    return forge:absolute( relative_path, target.settings.obj );
end

forge:register_module( linux );
return linux;
