
if forge:operating_system() == 'windows' then
    return require 'forge.msvc';
elseif forge:operating_system() == 'linux' then 
    return require 'forge.gcc';
elseif forge:operating_system() == 'macos' then
    return require 'forge.xcode';
end
