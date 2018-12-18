
if forge:operating_system() == 'windows' then
    return require 'forge.cc.msvc';
elseif forge:operating_system() == 'linux' then 
    return require 'forge.cc.gcc';
elseif forge:operating_system() == 'macos' then
    return require 'forge.xcode_clang';
end
