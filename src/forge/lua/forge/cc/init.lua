
require 'forge';

local operating_system = _G.operating_system();
if operating_system == 'windows' then
    return require 'forge.cc.msvc';
elseif operating_system == 'linux' then 
    return require 'forge.cc.gcc';
elseif operating_system == 'macos' then
    return require 'forge.cc.clang';
end
