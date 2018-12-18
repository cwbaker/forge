
local xcode_clang = require 'forge.xcode_clang';

local macos = {};

function macos.initialize( forge, settings, local_settings )
end

forge:register_module( macos );
forge.macos = macos;
return macos;
