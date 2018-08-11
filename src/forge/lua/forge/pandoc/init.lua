
pandoc = {};

function pandoc.configure( settings )
    local local_settings = forge.local_settings;
    if not local_settings.pandoc then
        local_settings.updated = true;
        local_settings.pandoc = {};
    end
end

function pandoc.initialize( settings )
end

require "forge.pandoc.Html";
require "forge.pandoc.Pdf";

forge:register_module( pandoc );
