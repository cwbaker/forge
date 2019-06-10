
if operating_system() == 'windows' then
    return require 'forge.msvc';
elseif operating_system() == 'linux' then 
    return require 'forge.gcc';
elseif operating_system() == 'macos' then
    return require 'forge.xcode';
end
