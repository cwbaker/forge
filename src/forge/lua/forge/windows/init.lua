
local windows = {};

function windows.initialize( settings )
    if forge:operating_system() == 'windows' then
        local msvc = require 'forge.cc.msvc';
        
        for _, architecture in ipairs(settings.architectures) do 
            local msvc_forge = forge:configure {
                obj = ('%s/cc_windows_%s'):format( settings.obj, architecture );
                platform = 'windows';
                architecture = architecture;
                default_architecture = architecture;
                obj_directory = windows.obj_directory;
            } 
            msvc.register( msvc_forge );
            forge:add_default_build( ('cc_windows_%s'):format(architecture), msvc_forge );
        end

        local settings = forge.settings;
        local default_architecture = settings.default_architecture;
        local default_forge = forge:default_build( ('cc_windows_%s'):format(default_architecture) )
        assert( default_forge, 'default Forge instance not found for Windows initialization' );
        local default_settings = default_forge.settings;
        settings.obj = default_settings.obj;
        settings.platform = default_settings.platform;
        settings.architecture = default_settings.architecture;
        settings.default_architecture = default_settings.default_architecture;
        settings.obj_directory = default_settings.obj_directory;
        msvc.register( forge );
    end
end

function windows.obj_directory( target )
    local relative_path = forge:relative( target:working_directory():path(), forge:root() );
    return forge:absolute( relative_path, target.settings.obj );
end

forge:register_module( windows );
return windows;
